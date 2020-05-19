/*!
* \file
* \brief ascreader.cpp foo
*/

#include "ascreader.h"
#include "logger.h"
#include "stringtools.h"
#include <cmath>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <utility>

const char *ASCReaderException::what() const throw()
{
    return "ASCReaderException";
}

/*!
 * \brief ASCReader::ASCReader
 * Constructor
 */
ASCReader::ASCReader(const std::string &fileName) :
    m_absoluteTimestamps(false),
    m_fileName(fileName),
    m_hexId(false),
    m_oldTimestamp(0)
{
    if (!parseASC(m_fileName)) {
        throw ASCReaderException();
    }
}

/*!
 * \brief ASCReader::parseASC
 * Parse ASC file and fill CAN frame queue
 * \param fileName: Filename of file to parse
 * \return True if successful, false otherwise.
 */
bool ASCReader::parseASC(const std::string &fileName)
{
    std::string line;
    std::ifstream file(fileName);

    if (!file.is_open()) {
        LOG(LOG_ERR, "error=1 Unable to open ASC file.\n");
        return false;
    }
    // Parse ASC file header
    if (!parseHeader(file)) {
        return false;
    }
    // Check for continuous log and if found parse previous files recursively
    if (!parseContinuousLogHeader(file)) {
        return false;
    }

    // Parse messages
    while (std::getline(file, line)) {
        // Parse input line for message
        parseMessage(line);
    }
    return true;
}

/*!
 * \brief ASCReader::parseContinuousLogHeader
 * Parse ASC file header for continuous log information and parse previous file if found
 * \param file: File stream to parse
 * \return True if successful, false otherwise.
 */
bool ASCReader::parseContinuousLogHeader(std::ifstream &file)
{
    // Check if line contains comment data with continuous log information
    int c = file.peek();
    if (c == '/') {
        std::string line;
        std::getline(file, line);
        if (line.find("previous log file") != std::string::npos) {
            // Find filename of previous log
            std::size_t logfilePosition = line.rfind(" ");
            if (logfilePosition != std::string::npos) {
                std::string previousLog = line.substr(logfilePosition);
                previousLog = trim(previousLog, " ");
                // Find separator between path and filename to get path of the original ASC file
                std::size_t separatorPosition = m_fileName.find_last_of("/");
                if ((separatorPosition == std::string::npos && !parseASC(previousLog)) ||
                    (separatorPosition != std::string::npos && !parseASC(m_fileName.substr(0, separatorPosition) + "/" + previousLog))) {
                    LOG(LOG_ERR, "Previous log file '%s' could not be found.\n", (m_fileName.substr(0, separatorPosition) + "/" + previousLog).c_str());
                    return false;
                }
                // Update old timestamp to last message in queue
                if (m_frameQueue.size() > 0) {
                    m_oldTimestamp = m_frameQueue[m_frameQueue.size() - 1].timestamp / 1000.0f;
                }
            } else {
                LOG(LOG_ERR, "Unable to find filename of previous log.\n");
                return false;
            }
        }
    }
    return true;
}

/*!
 * \brief ASCReader::parseHeader
 * Parse ASC file header
 * \param file: File stream to parse
 * \return True if successful, false otherwise.
 */
bool ASCReader::parseHeader(std::ifstream &file)
{
    std::istringstream input;
    std::string base, line, timestampFormat;

    // Skip first line
    std::getline(file, line);
    // Read line with ID number base and timestamp format
    std::getline(file, line);
    input.str(line);
    input.ignore(line.length(), ' ');
    // Get ID number base (hex or dec)
    input >> base;
    if (base == "hex") {
        m_hexId = true;
    } else if (base != "dec") {
        LOG(LOG_ERR, "Failed to parse CAN ID number base from ASC file header\n");
        return false;
    }
    input.ignore(line.length(), ' ');
    input.ignore(line.length(), ' ');
    input.ignore(line.length(), ' ');
    // Get timestamp format (absolute or relative)
    input >> timestampFormat;
    if (timestampFormat == "absolute") {
        m_absoluteTimestamps = true;
    } else if (timestampFormat != "relative") {
        LOG(LOG_ERR, "Failed to parse timestamp format from ASC file header\n");
        return false;
    }
    // Skip unneeded information
    std::getline(file, line);
    std::getline(file, line);
    return true;
}

/*!
 * \brief ASCReader::parseMessage
 * Parse message in ASC file
 * \param line: Input string
 * \return True if successful, false otherwise.
 */
bool ASCReader::parseMessage(std::string &line)
{
    std::istringstream input;
    input.str(line);
    int bus = 0, dlc = 0;
    int index = m_frameQueue.size();
    float timestamp = 0;
    uint32_t id = 0;
    std::string dir, type;
    unsigned int data[8];

    // Get timestamp and CAN bus number
    input >> timestamp >> bus;

    if (!input.good()) {
        return false;
    }
    // Get CAN ID
    if (m_hexId) {
        input >> std::hex >> id;
    } else {
        input >> std::dec >> id;
    }
    if (!input.good()) {
        return false;
    }
    // Check for extended frame
    int c = input.peek();
    if (c == 'x') {
        input.ignore();
        id |= 0x80000000U;
    }
    // Get direction
    input >> dir;
    // Get message type
    input >> type;
    // Data frame
    if (type == "d") {
        // Get number of bytes of data (dlc)
        input >> std::dec >> dlc;
        // Get frame data
        int d;
        input >> std::hex;
        for (d = 0; d < dlc; ++d) {
            input >> data[d];
        }
    } else {
        // Other frame types not supported
        return false;
    }
    // Set successfully parsed frame to frame queue
    if (!input.fail()) {
        if (!m_absoluteTimestamps) {
            // When using relative timestamps add the old (cumulative) timestamp
            // to timestamp of current message
            timestamp += m_oldTimestamp;
            // Keep track of old (cumulative) timestamp
            m_oldTimestamp = timestamp;
        }
        m_frameQueue[index].timestamp = llround(timestamp * 1000);
        if (dir == "Rx") {
            m_frameQueue[index].in = true;
        } else if (dir == "Tx") {
            m_frameQueue[index].in = false;
        }
        memset(&m_frameQueue[index].frame, 0, sizeof(struct canfd_frame));
        m_frameQueue[index].frame.can_id = id;
        m_frameQueue[index].frame.len = dlc;
        int d;
        for (d = 0; d < dlc; ++d) {
            m_frameQueue[index].frame.data[d] = data[d];
        }
    }
    return true;
}

/*!
 * \brief ASCReader::getFrameQueue
 * Get CAN frame queue
 * \return Reference to CAN frame queue
 */
std::map<std::uint64_t, canFrameQueueItem> &ASCReader::getFrameQueue()
{
    return m_frameQueue;
}

/*!
 * \brief ASCReader::createFilterList
 * Add all messages to filter list
 * \param list: filter list to be updated
 * \return true if successfully updated filtering list, false if not
 */
bool ASCReader::createFilterList(std::map<std::uint32_t, bool> &list)
{
    std::map<std::uint64_t, canFrameQueueItem>::iterator queue;
    for (queue = m_frameQueue.begin(); queue != m_frameQueue.end(); ++queue) {
        list.insert(std::pair<std::uint32_t, bool>(queue->second.frame.can_id, false));
    }
    return (list.size() > 0);
}
