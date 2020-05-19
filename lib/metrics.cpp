/*!
 * \file
 * \brief metrics.cpp foo
 */

#include "metrics.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string.h>
#include <time.h>

// Previously calculated CAN frame size standard
#define FRAME_SIZE 33

const char *MetricsCollectorException::what() const throw()
{
     return "MetricsCollectorException";
}

/*
 * \brief MetricsCollector::MetricsCollector
 * Constructor
 * \param canSimulator: pointer to a previously initialized CANSimulatorCore
 * \param filePath: path to a metrics output file to be created
 */
MetricsCollector::MetricsCollector(CANSimulatorCore *canSimulator, const std::string &filePath) :
    m_rateFactor(0),
    m_delayTime(0),
    m_valueSeparator(';'),
    m_canSimulator(canSimulator)
{
    if (canSimulator == NULL) {
        throw MetricsCollectorException();
    }
    if (filePath.empty()) {
        m_outputFile = "./metricsData_" + getCurrentTime() + ".txt";
    } else {
        m_outputFile = filePath + "_" + getCurrentTime() + ".txt";
    }
    memset(&m_totalMetrics, 0, sizeof(struct totalMetrics));
    memset(&m_burstMetrics, 0, sizeof(struct burstMetrics));

    m_bitrate = canSimulator->getCANBitrate();
    m_burstMetrics.min = -1;
    m_start = std::chrono::high_resolution_clock::now();
}

/*!
 * \brief MetricsCollector::updateMessage
 * Update or add a single message metrics data
 * \param message: pointer to an existing message
 */
void MetricsCollector::updateMessage(const CANMessage *message)
{
    if (!message) {
        LOG(LOG_WARN, "warning=4 Couldn't update message metrics, message not found\n");
        return;
    }
    std::map<std::uint32_t, messageMetrics>::iterator it;
    std::uint32_t id = message->getId();

    if ((it = m_messageMetrics.find(id)) == m_messageMetrics.end()) {
        struct messageMetrics newMessage;
        memset(&newMessage, 0, sizeof(struct messageMetrics));
        // Set message data direction
        newMessage.direction = message->getDirection();
        // Set message extended or stadard frame
        newMessage.std = !(id & CAN_EFF_FLAG);
        // Set message successful count
        newMessage.successful = message->getSuccessful();
        // Set message failed count
        newMessage.failed = message->getFailed();
        // Set message false direction count
        newMessage.falseDirection = message->getFalseDirection();

        // Calculate message bitsize
        newMessage.messageSize = FRAME_SIZE + (message->getDlc() * 8) + ((newMessage.std) ? CAN_SFF_ID_BITS : CAN_EFF_ID_BITS);
        // Calculate time message will take to send (if no bitrate, assume 1usec/bit)
        float uSecFactor = (m_bitrate > 0) ? ((1000000 / (float)m_bitrate)) : 1;
        newMessage.messageTime = newMessage.messageSize * uSecFactor;
        // Calculate possible idle sending time (only with sending messages)
        if ((m_rateFactor > 0 || m_delayTime > 0) && (message->getDirection() == MessageDirection::SEND)) {
            newMessage.idleTime = ((m_rateFactor > 0) ? (newMessage.messageSize * m_rateFactor) : m_delayTime) - newMessage.messageTime;
            // As idle time may be below zero, no idle time needed
            if (newMessage.idleTime < 0) {
                newMessage.idleTime = 0;
            }
            // Calculate total idle time message has used
            newMessage.idleTotal = (message->getSuccessful() + message->getFailed()) * newMessage.idleTime;
        }
        // Calculate total time message has used
        newMessage.timeTotal = (message->getSuccessful() + message->getFailed() + message->getFalseDirection()) * newMessage.messageTime;

        // Add new message data to map
        m_messageMetrics.insert(std::pair<std::uint32_t, messageMetrics>(id, newMessage));
    } else {
        // Update existing message metrics
        it->second.successful = message->getSuccessful();
        it->second.failed = message->getFailed();
        it->second.falseDirection = message->getFalseDirection();
        it->second.timeTotal = (message->getSuccessful() + message->getFailed() + message->getFalseDirection()) * it->second.messageTime;
        if (m_rateFactor > 0 || m_delayTime > 0) {
            it->second.idleTotal = (message->getSuccessful() + message->getFailed()) * it->second.idleTime;
        }
    }
}

/*!
 * \brief MetricsCollector::initBurstSettings
 * Initialize burst collector settings
 * \param len: currently used burst-len
 * \param delay: currently used burst-delay
 */
void MetricsCollector::initBurstSettings(int len, int delay)
{
    m_burstMetrics.len = len;
    m_burstMetrics.delay = delay;
}

/*!
 * \brief MetricsCollector::updateBurstData
 * Update burst counters
 * \param sleep: if set as true, burst is not sending and counters will be updated
 */
void MetricsCollector::updateBurstData(bool sleep)
{
    if (m_burstMetrics.len <= 0 || m_burstMetrics.delay <= 0)  {
        // Bursting has no time set, so no use in calculating anything
        return;
    }
    if (!sleep) {
        m_burstMetrics.count++;
    } else {
        if (m_burstMetrics.count > 0) {
            m_burstMetrics.min = (m_burstMetrics.min < 0) ? m_burstMetrics.count : m_burstMetrics.min;
            m_burstMetrics.min = (m_burstMetrics.count < m_burstMetrics.min) ? m_burstMetrics.count : m_burstMetrics.min;
            m_burstMetrics.max = (m_burstMetrics.count > m_burstMetrics.max) ? m_burstMetrics.count : m_burstMetrics.max;
            m_burstMetrics.totalCount += m_burstMetrics.count;
            m_burstMetrics.count = 0;
        }
        m_burstMetrics.sendTime += m_burstMetrics.len;
        m_burstMetrics.delayTime += m_burstMetrics.delay;
    }
}

/*!
 * \brief MetricsCollector::updateBurstIdleTime
 * Update burst idletime counter from all the messages
 */
void MetricsCollector::updateBurstIdleTime()
{
    m_burstMetrics.idleTime = 0;
    for (auto it = m_messageMetrics.begin(); it != m_messageMetrics.end(); ++it) {
        m_burstMetrics.idleTime += it->second.idleTotal;
    }
}

/*!
 * \brief MetricsCollector::initDelaySend
 * Set message sending time
 * \param delay: set delay time
 */
void MetricsCollector::initDelaySend(int delay)
{
    m_delayTime = delay;
}

/*!
 * \brief MetricsCollector::initRateSend
 * Set message sending time factor
 * \param factor: calculated congestion rate for a bit
 */
void MetricsCollector::initRateSend(float factor)
{
    m_rateFactor = factor;
}

/*!
 * \brief MetricsCollector::setValueSeparator
 * Set output file value separator character
 * \param separator: char to be used as a value separator in the output file
 */
void MetricsCollector::setValueSeparator(char separator)
{
    m_valueSeparator = separator;
}

/*!
 * \brief MetricsCollector::writeMessageData
 * Write individual message metrics to given file
 * \param file: reference to previously opened target file
 * \return true if wrote data, false if didn't
 */
bool MetricsCollector::writeMessageData(std::ofstream &file) const
{
    std::stringstream writer;
    if (file.is_open()) {
        file << std::string("INDIVIDUAL MESSAGES\n") +
            "ID" + m_valueSeparator +
            "Direction" + m_valueSeparator +
            "Successful" + m_valueSeparator +
            "Failed" + m_valueSeparator +
            "False direction" + m_valueSeparator +
            "Time (usec)" + m_valueSeparator +
            "Idle time (usec)" + m_valueSeparator +
            "Description\n";
        for (auto it = m_messageMetrics.begin(); it != m_messageMetrics.end(); ++it) {
            const CANMessage *msg = m_canSimulator->getMessage(it->first);
            writer << it->first << m_valueSeparator
                << ((it->second.direction == MessageDirection::RECEIVE) ? (std::string("Rx") + m_valueSeparator + "") : (std::string("Tx") + m_valueSeparator + ""))
                << it->second.successful << m_valueSeparator
                << it->second.failed << m_valueSeparator
                << it->second.falseDirection << m_valueSeparator
                << it->second.timeTotal << m_valueSeparator
                << it->second.idleTotal << m_valueSeparator
                << msg->getDescription() << '\n';
            file << writer.str();
            writer.str("");
        }
        return true;
    }
    return false;
}

/*!
 * \brief MetricsCollector::writeTotalData
 * Write total metrics data to a given file
 * \param file: reference to previously opened target file
 * \return true if wrote data, false if didn't
 */
bool MetricsCollector::writeTotalData(std::ofstream &file) const
{
    std::stringstream writer;
    if (file.is_open()) {
        file << std::string("\nTOTAL DATA (") + getCurrentTime() + ")\n" +
            "Total sent" + m_valueSeparator +
            "Total Tx failed" + m_valueSeparator +
            "Total received" + m_valueSeparator +
            "Total false direction" + m_valueSeparator +
            "Runtime (usec)" + m_valueSeparator +
            "Total time (usec)" + m_valueSeparator +
            "Total Tx time (usec)" + m_valueSeparator +
            "Total Rx time (usec)" + m_valueSeparator +
            "Message Average (usec)" + m_valueSeparator +
            "Ext count" + m_valueSeparator +
            "Std count\n";
        writer << m_totalMetrics.sent << m_valueSeparator
            << m_totalMetrics.TxFailed << m_valueSeparator
            << m_totalMetrics.received << m_valueSeparator
            << m_totalMetrics.totalFalseDirection << m_valueSeparator
            << m_totalMetrics.totalRuntime << m_valueSeparator
            << m_totalMetrics.totalTime << m_valueSeparator
            << m_totalMetrics.sendTime << m_valueSeparator
            << m_totalMetrics.receiveTime << m_valueSeparator
            << ((m_totalMetrics.totalMessages > 0) ? ((double)m_totalMetrics.totalTime / (double)(m_totalMetrics.totalMessages)) : 0) << m_valueSeparator
            << m_totalMetrics.extCount << m_valueSeparator
            << m_totalMetrics.stdCount << '\n';
        file << writer.str();
        return true;
    }
    return false;
}

/*!
 * \brief MetricsCollector::writeBurstData
 * Write burst metrics to a given file
 * \param file: reference to previously opened target file
 * \return true if wrote data, false if didn't
 */
bool MetricsCollector::writeBurstData(std::ofstream &file) const
{
    std::stringstream writer;
    if (file.is_open()) {
        file << std::string("\nBURST DATA\n") +
            "Total burst time (usec)" + m_valueSeparator +
            "Burst idle time (usec)" + m_valueSeparator +
            "Total delay time (usec)" + m_valueSeparator +
            "Burst min messages" + m_valueSeparator +
            "Burst max messages" + m_valueSeparator +
            "Total burst count" + m_valueSeparator +
            "Burst length" + m_valueSeparator +
            "Burst delay\n";
        writer << m_burstMetrics.sendTime << m_valueSeparator
            << m_burstMetrics.idleTime << m_valueSeparator
            << m_burstMetrics.delayTime << m_valueSeparator
            << m_burstMetrics.min << m_valueSeparator
            << m_burstMetrics.max << m_valueSeparator
            << (m_burstMetrics.totalCount + m_burstMetrics.count) << m_valueSeparator
            << m_burstMetrics.len << m_valueSeparator
            << m_burstMetrics.delay << '\n';
        file << writer.str();
        return true;
    }
    return false;
}
/*!
 * \brief MetricsCollector::writeErrorData
 * Write error metrics to a given file
 * \param file: reference to previously opened target file
 * \return true if wrote data, false if didn't
 */
bool MetricsCollector::writeErrorData(std::ofstream &file) const
{
    std::stringstream writer;
    if (file.is_open()) {
        struct errorMetrics const error = m_canSimulator->getErrorMetrics();
        file << std::string("\nERROR DATA\n") +
            "Total errors" + m_valueSeparator +
            "Error time (usec)" + m_valueSeparator +
            "Total unknown" + m_valueSeparator +
            "Unknown time (usec)\n";
        float uSecFactor = (m_bitrate > 0) ? ((1000000 / (float)m_bitrate)) : 1;
        writer << error.errorMessages << m_valueSeparator
            << (error.errorSize * uSecFactor) << m_valueSeparator
            << error.unknownMessages << m_valueSeparator
            << (error.unknownSize * uSecFactor) << '\n';
        file << writer.str();
        return true;
    }
    return false;
}

/*!
 * \brief MetricsCollector::writeToFile
 * Write metrics to an output file with character-separator -style
 * \param target: alternative filepath to output file
 * \param test: if true, will not update data from actual CANMessages, used for testing only
 * \return true if successful, false otherwise
 */
bool MetricsCollector::writeToFile(std::string target, bool test)
{
    std::ofstream file(((target.empty()) ? m_outputFile : target));

    if (file.is_open()) {
        bool success = true;
        if(!test) {
            updateMessages();
        }
        success = success && writeMessageData(file);
        updateTotal();
        success = success && writeTotalData(file);
        success = success && writeErrorData(file);

        if (m_burstMetrics.totalCount > 0) {
            updateBurstIdleTime();
            success = success && writeBurstData(file);
        }
        file.close();
        return success;
    }

    LOG(LOG_ERR, "error=6 Couldn't create metrics output file '%s'\n", ((target.empty()) ? m_outputFile.c_str() : target.c_str()));
    return false;
}

/*!
 * \brief MetricsCollector::getTotalMetrics
 * Update and get reference to total metrics
 * \param test: if true, will not update data from actual CANMessages, used for testing only
 * \return totalMetrics structure
 */
struct totalMetrics &MetricsCollector::getTotalMetrics(bool test)
{
    if (!test) {
        updateMessages();
    }
    updateTotal();
    return m_totalMetrics;
}

/*!
 * \brief MetricsCollector::getBurstMetrics
 * Get reference to burst metrics
 * \param test: if true, will not update data from actual CANMessages, used for testing only
 * \return burstMetrics structure
 */
struct burstMetrics &MetricsCollector::getBurstMetrics(bool test)
{
    if (!test) {
        updateMessages();
    }
    updateBurstIdleTime();
    return m_burstMetrics;
}

/*!
 * \brief MetricsCollector::getSingleMessageMetrics
 * Get single message from the metrics map
 * \param id: ID of the wanted message
 * \param test: if true, will not update data from actual CANMessages, used for testing only
 * \return reference to a existing message or NULL if not found
 */
struct messageMetrics *MetricsCollector::getSingleMessageMetrics(std::uint32_t id, bool test)
{
    std::map<std::uint32_t, messageMetrics>::iterator it;
    if (!test) {
        updateMessages();
    }

    if ((it = m_messageMetrics.find(id)) != m_messageMetrics.end()) {
        return &it->second;
    } else {
        return NULL;
    }
}

/*!
 * \brief MetricsCollector::getMessageMetrics
 * Get reference to collected message metrics collection
 * \param test: if true, will not update data from actual CANMessages, used for testing only
 * \return messageMetrics map structure
 */
std::map<std::uint32_t, messageMetrics> &MetricsCollector::getMessageMetrics(bool test)
{
    if (!test) {
        updateMessages();
    }
    return m_messageMetrics;
}

/*!
 * \brief MetricsCollector::updateMessages
 * Update message metrics from CANMessages
 */
void MetricsCollector::updateMessages()
{
    std::map<std::uint32_t, CANMessage> const messages = m_canSimulator->getMessages();
    for (auto it = messages.begin(); it != messages.end(); ++it) {
        updateMessage(&it->second);
    }
}

/*!
 * \brief MetricsCollector::getCurrentTime
 * Get current time as a string
 * \return string to current time
 */
std::string MetricsCollector::getCurrentTime()
{
    time_t rawtime;
    struct tm ptm;
    time(&rawtime);
    std::stringstream ret;

    localtime_r(&rawtime, &ptm);
    ret << (ptm.tm_year + 1900) << "." <<
        std::setfill('0') << std::setw(2) << (ptm.tm_mon + 1) << "." <<
        std::setfill('0') << std::setw(2) <<
        std::setfill('0') << std::setw(2) << ptm.tm_mday << "-" <<
        std::setfill('0') << std::setw(2) <<
        std::setfill('0') << std::setw(2) << ptm.tm_hour << "." <<
        std::setfill('0') << std::setw(2) << ptm.tm_min;

    return ret.str();
}

/*!
 * \brief MetricsCollector::updateTotal
 * Update total metrics from all the messages
 */
void MetricsCollector::updateTotal()
{
    memset(&m_totalMetrics, 0, sizeof (struct totalMetrics));
    for (auto it = m_messageMetrics.begin(); it != m_messageMetrics.end(); ++it) {
        if (it->second.direction == MessageDirection::RECEIVE) {
            m_totalMetrics.received += it->second.successful;
            m_totalMetrics.receiveTime += it->second.timeTotal;
        } else {
            m_totalMetrics.sent += it->second.successful;
            m_totalMetrics.TxFailed += it->second.failed;
            m_totalMetrics.sendTime += it->second.timeTotal;
        }
        if (it->second.std) {
            m_totalMetrics.stdCount += it->second.successful + it->second.failed + it->second.falseDirection;
        } else {
            m_totalMetrics.extCount += it->second.successful + it->second.failed + it->second.falseDirection;
        }

        m_totalMetrics.totalTime += it->second.timeTotal + it->second.idleTotal;
        m_totalMetrics.totalMessages += it->second.successful + it->second.failed + it->second.falseDirection;
        m_totalMetrics.totalFalseDirection += it->second.falseDirection;
    }
    hires_timer now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - m_start;
    m_totalMetrics.totalRuntime = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}
