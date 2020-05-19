/*!
* \file
* \brief cansimulatorcore.cpp foo
*/

#include "cansimulatorcore.h"
#include "canerror.h"
#include "logger.h"
#include "stringtools.h"
#include <chrono>
#include <cstring>
#include <time.h>
#include <unistd.h>

extern "C" {
#include <linux/can.h>
}

#define FRAME_SIZE 33

const char *CANSimulatorCoreException::what() const throw()
{
    return "CANSimulatorCoreException";
}

// As the m_useNativeUnits is a static, it has to be initialized here
bool CANSimulatorCore::m_useNativeUnits = false;

/*!
 * \brief CANSimulatorCore::CANSimulatorCore
 * Constructor
 * \param cfg: path to configuration file
 * \param dbc: path to CAN specification dbc file
 * \param asc: path to ASC CAN message log file
 * \param socketName: CAN socket name
 * \param suppressDefaults: suppress reporting incoming initial default values
 * \param ignoreDirections: ignore message directions defined in configuration
 * \param interval: interval for data simulator (default 10 ms)
 * \param runTime: duration of data simulation run (default -1, infinite)
 */
CANSimulatorCore::CANSimulatorCore(const std::string &cfg, const std::string &dbc, const std::string &asc,
                                   const std::string &socketName, bool suppressDefaults, bool ignoreDirections,
                                   unsigned int interval, int runTime) :
    m_interval(interval),
    m_runTime(runTime),
    m_sendTime(true),
    m_useUTCTime(false),
    m_threadsRunning(true),
    m_config(NULL),
    m_ascReader(NULL),
    m_canTransceiver(NULL),
    m_simulationRunning(false),
    m_simulationTime(0)
{
    if (!asc.empty()) {
        try {
            m_ascReader = new ASCReader(asc);
        }
        catch (ASCReaderException&) {
            throw CANSimulatorCoreException();
        }
    } else {
        if (!loadConfiguration(cfg, dbc, suppressDefaults, ignoreDirections)) {
            m_threadsRunning = false;
            throw CANSimulatorCoreException();
        }
        setSendTime(m_sendTime);
    }

    // Do not use CAN interface if socket name is empty
    if (socketName.empty()) {
        return;
    }

    try {
        int bitrateConfigured = 0;
        // If .dbc-file has baudrate info use it in CAN interface initialization
        if (m_config && m_config->getAttribute("Baudrate")) {
            bitrateConfigured = std::stoi(m_config->getAttribute("Baudrate")->getValue());
        }
        m_canTransceiver = new CANTransceiver(socketName, bitrateConfigured);
    }
    catch (const std::invalid_argument &) {
        LOG(LOG_WARN, "warning=2 Invalid dbc Baudrate: %s\n", m_config->getAttribute("Baudrate")->getValue().c_str());
    }
    catch (const std::out_of_range &) {
        LOG(LOG_ERR, "error=2 dbc Baudrate out of range: %s\n", m_config->getAttribute("Baudrate")->getValue().c_str());
    }
    catch (CANTransceiverException&) {
        m_threadsRunning = false;
        delete m_config;
        throw CANSimulatorCoreException();
    }
    memset(&m_errorMetrics, 0, sizeof(struct errorMetrics));
}

/*!
 * \brief CANSimulatorCore::~CANSimulatorCore
 * Destructor
 */
CANSimulatorCore::~CANSimulatorCore()
{
    m_threadsRunning = false;
    // Wait for threads to end.
    if (m_readerThread.joinable()) m_readerThread.join();
    if (m_senderThread.joinable()) m_senderThread.join();
    delete m_ascReader;
    delete m_canTransceiver;
    delete m_config;
}

/*!
* \brief CANSimulatorCore::loadConfiguration
* Load configuration using configuration and dbc filenames
 * \param cfg: path to configuration file
 * \param dbc: path to CAN specification dbc file
 * \param suppressDefaults: suppress reporting incoming initial default values
 * \param ignoreDirections: ignore message directions defined in configuration
 * \return True if successful, false otherwise.
*/
bool CANSimulatorCore::loadConfiguration(const std::string &cfg, const std::string &dbc, bool suppressDefaults, bool ignoreDirections)
{
    delete m_config;
    try {
        m_config = new Configuration(cfg, dbc, suppressDefaults, ignoreDirections);
    }
    catch (ConfigurationException&) {
        return false;
    }
    return true;
}

/*!
 * \brief CANSimulatorCore::getCfgVersion
 * Get cfg file date and revision
 * \return cfg file date and revision string
 */
std::string CANSimulatorCore::getCfgVersion() const
{
    return m_config->getCfgVersion();
}

/*!
 * \brief CANSimulatorCore::getDBCVersion
 * Get DBC file date and revision
 * \return DBC file date and revision string
 */
std::string CANSimulatorCore::getDBCVersion() const
{
    return m_config->getDBCVersion();
}

/*!
 * \brief CANSimulatorCore::getUseNativeUnits
 * Get usage of native units as a static input
 * \return True if native units are enabled, otherwise false
 */
bool CANSimulatorCore::getUseNativeUnits()
{
    return m_useNativeUnits;
}

/*!
 * \brief CANSimulatorCore::setUseNativeUnits
 * Set usage of static native units as input
 * \param enable: boolean state to set the useNativeUnits to
 */
void CANSimulatorCore::setUseNativeUnits(bool enable)
{
    m_useNativeUnits = enable;
}

/*!
 * \brief CANSimulatorCore::getSendTime
 * Get usage of automatic sending of time
 * \return True if automatic sending of time is enabled, otherwise false
 */
bool CANSimulatorCore::getSendTime() const
{
    return m_sendTime;
}

/*!
 * \brief CANSimulatorCore::setSendTime
 * Set usage of automatic sending of time
 */
void CANSimulatorCore::setSendTime(bool enable)
{
    if (enable &&
        (m_config->isVariableSupported("year") &&
         m_config->isVariableSupported("month") &&
         m_config->isVariableSupported("day") &&
         m_config->isVariableSupported("hour") &&
         m_config->isVariableSupported("min"))) {
        m_sendTime = true;
    } else {
        m_sendTime = false;
    }
}

/*!
 * \brief CANSimulatorCore::getUseUTCTime
 * Get usage of UTC time for automatic sending of time
 * \return True if UTC time is enabled, otherwise false
 */
bool CANSimulatorCore::getUseUTCTime() const
{
    return m_useUTCTime;
}

/*!
 * \brief CANSimulatorCore::setUseUTCTime
 * Set usage of UTC time for automatic sending of time
 */
void CANSimulatorCore::setUseUTCTime(bool enable)
{
    m_useUTCTime = enable;
}

/*!
 * \brief CANSimulatorCore::getRunTime
 * Get run time in seconds
 * \return Run time of data simulator in seconds
 */
int CANSimulatorCore::getRunTime() const
{
    return m_runTime;
}

/*!
 * \brief CANSimulatorCore::getRunTimeRemaining in seconds
 * Get remaining run time in seconds
 * \return Remaining run time of data simulator in seconds, -1 if no run time defined
 */
int CANSimulatorCore::getRunTimeRemaining() const
{
    return m_runTime > 0 ? (m_runTime - (m_simulationTime / 1000)) : -1;
}

/*!
 * \brief CANSimulatorCore::setRunTime
 * Set run time in seconds
 * \param runTime: Run time of data simulator in seconds
 */
void CANSimulatorCore::setRunTime(int runTime)
{
    m_runTime = runTime;
}

/*!
 * \brief CANSimulatorCore::setValue
 * Set a value for a key
 * \param key: Variable name
 * \param value: Value for key
 * \return True if successful, false otherwise.
 */
bool CANSimulatorCore::setValue(std::string key, Value value)
{
    std::lock_guard<std::mutex> guard(m_inputMutex);
    return m_config->setValue(key, value);
}

/*!
 * \brief CANSimulatorCore::setValue
 * Set a value for a key from string
 * \param key: Variable name
 * \param value: Value as a string
 * \return True if successful, false otherwise.
 */
bool CANSimulatorCore::setValue(std::string key, std::string value)
{
    std::lock_guard<std::mutex> guard(m_inputMutex);
    return m_config->setValue(key, value);
}

/*!
 * \brief CANSimulatorCore::setValues
 * Set values from vector of strings in format var=val
 * \param input: Vector of strings
 * \return True if successful, false otherwise.
 */
bool CANSimulatorCore::setValues(std::vector<std::string> &input)
{
    std::lock_guard<std::mutex> guard(m_inputMutex);
    bool ret = true;
    // Loop through input
    for (std::vector<std::string>::iterator it = input.begin(); it != input.end(); ++it) {
        // Skip empty strings
        if ((*it).length() == 0) {
            continue;
        }
        // Parse and set signal values
        std::vector<std::string> values = split(*it, '=');
        if (values.size() == 2) {
            if (!m_config->setValue(values[0], values[1])) {
                ret = false;
            }
        } else {
            LOG(LOG_WARN, "warning=3 Unknown input '%s'.\n", (*it).c_str());
            ret = false;
        }
    }
    return ret;
}

/*!
 * \brief CANSimulatorCore::setDefaultValues
 * Set all value to defaults
 * \param sendMessages: boolean to define whether values are send to CAN bus
 */
void CANSimulatorCore::setDefaultValues(bool sendMessages)
{
    m_config->setDefaultValues();
    if (sendMessages) {
        sendCANMessages(true);
    }
}

/*!
 * \brief CANSimulatorCore::startDataSimulator
 * Start automatic data simulator
 */
void CANSimulatorCore::startDataSimulator()
{
    if (m_ascReader && !m_simulationRunning) {
        m_simulationRunning = true;
        m_threadsRunning = false;
        startCANSenderThread();
    }
}

/*!
 * \brief CANSimulatorCore::stopDataSimulator
 * Stop automatic data simulator
 */
void CANSimulatorCore::stopDataSimulator()
{
    m_simulationRunning = false;
}

/*!
 * \brief CANSimulatorCore::isDataSimulatorRunning
 * Check if automatic data simulator is running
 * \return True if data simulator running, false otherwise
 */
bool CANSimulatorCore::isDataSimulatorRunning() const
{
    return m_simulationRunning;
}

/*!
 * \brief CANSimulatorCore::getSignal
 * Get the signal by variable name
 * \param key: Variable name
 * \return Pointer to signal object, NULL if not found.
 */
const CANSignal *CANSimulatorCore::getSignal(const std::string &key)
{
    return m_config->getSignal(key);
}

/*!
 * \brief CANSimulatorCore::getMessage
 * Get the message by variable name
 * \param key: Variable name
 * \return Pointer to message object, NULL if not found.
 */
const CANMessage *CANSimulatorCore::getMessage(const std::string &key)
{
    return m_config->getMessage(key);
}

/*!
 * \brief CANSimulatorCore::getMessage
 * Get message by CAN message id
 * \param id: CAN message ID
 * \return pointer to CANMessage or NULL, if not found
 */
const CANMessage *CANSimulatorCore::getMessage(std::uint32_t id)
{
    return m_config->getMessage(id);
}

/*!
 * \brief CANSimulatorCore::getMessages
 * Get all configured messages
 * \return complete CANMessage map
 */
const std::map<std::uint32_t, CANMessage> &CANSimulatorCore::getMessages() const
{
    return m_config->getMessages();
}

/*!
 * \brief CANSimulatorCore::getVariables
 * Get all configured variables.
 * \return Reference to the set of configured variables
 */
const std::set<std::string> &CANSimulatorCore::getVariables() const
{
    return m_config->getVariables();
}

/*!
 * \brief CANSimulatorCore::sendCANMessage
 * \param key: name of the signal
 * \param forceSend: Send message even if not modified
 * \return True if successful, false otherwise
 */
bool CANSimulatorCore::sendCANMessage(const std::string &key, bool forceSend)
{
    std::uint32_t msgId;
    if (m_config->getMessageId(key, msgId)) {
       return sendCANMessage(msgId, forceSend);
    }
    return false;
}

/*!
 * \brief CANSimulatorCore::sendCANMessage
 * \param id: CAN ID of the signal
 * \param forceSend: Send message even if not modified
 * \return True if successful, false otherwise
 */
bool CANSimulatorCore::sendCANMessage(std::uint32_t id, bool forceSend)
{
    CANMessage *message = m_config->getMessage(id);
    if (message) {
        if (!isMessageFiltered(message->id)) {
            if (forceSend || message->isModified()) {
                return m_canTransceiver->sendCANMessage(message);
            }
        }
    }
    return false;
}

/*!
 * \brief CANSimulatorCore::sendCANMessages
 * Send all modified CAN messages
 * \param sendAll: Send messages even if not modified
 * \return True if all modifien message were sent successfully, false otherwise
 */
bool CANSimulatorCore::sendCANMessages(bool sendAll)
{
    bool ret = true;
    std::set<std::uint32_t> sendIDs = m_config->getSendIDs();
    for (std::set<std::uint32_t>::iterator it = sendIDs.begin(); it != sendIDs.end(); ++it) {
        if (!sendCANMessage(*it, sendAll)) {
            ret = false;
        }
    }
    return ret;
}

/*!
 * \brief CANSimulatorCore::readCANMessage
 * Read the data from incoming CAN message and set values to variables
 * \return Message CAN ID if message was read and its content was changed, otherwise 0
 */
std::uint32_t CANSimulatorCore::readCANMessage()
{
    canfd_frame frame;
    bool canfd;
    if (m_canTransceiver->readCANFrame(&frame, &canfd)) {
        if ((frame.can_id & CAN_ERR_FLAG) == CAN_ERR_FLAG) {
            LOG(LOG_ERR, analyzeErrorFrame(&frame));
            m_errorMetrics.errorMessages++;
            m_errorMetrics.errorSize += FRAME_SIZE + frame.len + ((frame.can_id & CAN_EFF_FLAG) ? CAN_EFF_ID_BITS : CAN_SFF_ID_BITS);
            return 0;
        } else if (!isMessageFiltered(frame.can_id)) {
            if (m_config->getReceiveIDs().count(frame.can_id)) {
                if (CANMessage *message = m_config->getMessage(frame.can_id)) {
                    message->updateTransfer(true, MessageDirection::RECEIVE);
                    if (message->parseCANFrame(&frame, canfd)) {
                        return frame.can_id;
                    }
                    return 0;
                }
            } else if (m_config->getSendIDs().count(frame.can_id)) {
                if (CANMessage *message = m_config->getMessage(frame.can_id)) {
                    message->updateTransfer(true, MessageDirection::RECEIVE);
                    return 0;
                }
            }
        }
        m_errorMetrics.unknownMessages++;
        m_errorMetrics.unknownSize += FRAME_SIZE + frame.len + ((frame.can_id & CAN_EFF_FLAG) ? CAN_EFF_ID_BITS : CAN_SFF_ID_BITS);
    }
    return 0;
}

/*!
 * \brief CANSimulatorCore::getCANBitrate
 * Read CAN bus bitrate from the bus
 * \return CAN bitrate, or 0 in case of an error
 */
int CANSimulatorCore::getCANBitrate()
{
    if (m_canTransceiver) {
        return m_canTransceiver->getCANBitrate();
    }
    return 0;
}

/*!
 * \brief CANSimulatorCore::startCANReaderThread
 * Start a thread for reading CAN messages from CAN bus.
 */
void CANSimulatorCore::startCANReaderThread()
{
    m_readerThread = std::thread( [this] { this->CANReaderThread(); } );
}

/*!
 * \brief CANSimulatorCore::startCANSenderThread
 * Start a thread for sending CAN messages to CAN bus.
 */
void CANSimulatorCore::startCANSenderThread()
{
    m_senderThread = std::thread( [this] { this->CANSenderThread(); } );
}

/*!
 * \brief CANSimulatorCore::stopCANThreads
 * Stop CAN threads.
 */
void CANSimulatorCore::stopCANThreads()
{
    m_threadsRunning = false;
}

/*!
 * \brief CANSimulatorCore::CANReaderThread
 * Read incoming messages from CAN bus.
 */
void CANSimulatorCore::CANReaderThread()
{
    fd_set input_set;
    while (m_threadsRunning) {
        // Empty the FD set
        FD_ZERO(&input_set);
        int canSocket = m_canTransceiver->getCANSocket();
        if (canSocket >= 0) {
            FD_SET(canSocket, &input_set);
        }
        // Wait 10 milliseconds for incoming CAN messages
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        if (select(canSocket+1, &input_set, NULL, NULL, &timeout) > 0) {
            // Check for data from CAN bus
            if (FD_ISSET(canSocket, &input_set)) {
                std::uint32_t id;
                if ((id = readCANMessage())) {
                    CANMessage *msg = m_config->getMessage(id);
                    CANMessage *newMsg = new CANMessage(*msg);
                    std::shared_ptr<CANMessage> msgCopy(newMsg);
                    m_messageQueue.push(msgCopy);
                    msg->setModified(false);
                    LOG(LOG_DBG, "Incoming message read successfully\n");
                } else {
                    LOG(LOG_DBG, "Incoming message read failed\n");
                }
            }
        }
    }
}

/*!
 * \brief CANSimulatorCore::CANSenderThread
 * Send outgoing messages to CAN bus.
 */
void CANSimulatorCore::CANSenderThread()
{
    std::map<std::uint64_t, canFrameQueueItem> queue;
    std::map<std::uint64_t, canFrameQueueItem>::iterator it;
    std::set<std::uint32_t> sendIDs;
    std::chrono::time_point<std::chrono::system_clock> loopCounter = std::chrono::high_resolution_clock::now();
    std::chrono::time_point<std::chrono::system_clock> timeSendCounter = std::chrono::high_resolution_clock::now();
    std::chrono::duration<int,std::milli> loopTime(std::chrono::duration<int,std::milli>(10));
    std::chrono::duration<int,std::milli> timeSendInterval(std::chrono::duration<int,std::milli>(100));
    if (m_simulationRunning) {
        m_simulationTime = 0;
        queue = m_ascReader->getFrameQueue();
        it = queue.begin();
    } else {
        sendIDs = m_config->getSendIDs();
    }
    while (m_threadsRunning || m_simulationRunning) {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::high_resolution_clock::now();
        int canSocket = m_canTransceiver->getCANSocket();
        if (canSocket < 0) {
            LOG(LOG_ERR, "CAN socket not ready\n");
            continue;
        }
        if (m_simulationRunning) {
            while (it != queue.end() && it->second.timestamp <= m_simulationTime) {
                if (it->second.in) {
                    if (!isMessageFiltered(it->second.frame.can_id)) {
                        m_canTransceiver->sendCANFrame(&it->second.frame);
                    }
                }
                ++it;
            }
            m_simulationTime += m_interval;

            if (it == queue.end() || (m_runTime > 0 && m_simulationTime > (unsigned int)m_runTime * 1000)) {
                m_simulationRunning = false;
            }
        } else {
            std::lock_guard<std::mutex> guard(m_inputMutex);
            // Set current time to signal values if needed
            updateTime(timeSendCounter, now, timeSendInterval);
            // Check all messages
            for (std::set<std::uint32_t>::iterator it = sendIDs.begin(); it != sendIDs.end(); ++it) {
                CANMessage *msg = m_config->getMessage(*it);
                if (msg) {
                    if (!isMessageFiltered(msg->id)) {
                        bool force = false;
                        bool send = false;
                        if (msg->isSendScheduled(now)) {
                            send = true;
                            force = true;
                        }
                        if (send && sendCANMessage(*it, force)) {
                            LOG(LOG_DBG, "Sent message %u\n", *it);
                        }
                    }
                }
            }
        }
        // Wait in loop for 10 milliseconds
        loopCounter += loopTime;
        // Prevent fast looping in case of long delays in loop
        if (loopCounter < now) {
            loopCounter = now + loopTime;
        }
        std::this_thread::sleep_until(loopCounter);
    }
}

/*!
 * \brief CANSimulatorCore::updateTime
 * Update time signal values if needed
 */
void CANSimulatorCore::updateTime(std::chrono::time_point<std::chrono::system_clock> &timeSendCounter,
                                  std::chrono::time_point<std::chrono::system_clock> &now,
                                  std::chrono::duration<int,std::milli> &timeSendInterval)
{
    if (m_sendTime) {
        if (timeSendCounter <= now) {
            time_t rawtime;
            struct tm ptm;
            time(&rawtime);
            if (m_useUTCTime) {
                gmtime_r(&rawtime, &ptm);
            } else {
                localtime_r(&rawtime, &ptm);
            }
            m_config->setValue("year", std::to_string(1900+ptm.tm_year));
            m_config->setValue("month", std::to_string(ptm.tm_mon+1));
            m_config->setValue("day", std::to_string(ptm.tm_mday));
            m_config->setValue("hour", std::to_string(ptm.tm_hour));
            m_config->setValue("min", std::to_string(ptm.tm_min));
            if (m_config->isVariableSupported("sec")) {
                m_config->setValue("sec", std::to_string(ptm.tm_sec));
            }
            timeSendCounter += timeSendInterval;
            if (timeSendCounter < now) {
                timeSendCounter = now + timeSendInterval;
            }
        }
    }
}

/*!
 * \brief CANSimulatorCore::getMessageQueue
 * Get pointer to incoming message queue
 * \return Pointer to incoming message queue
 */
Queue<std::shared_ptr<CANMessage>> *CANSimulatorCore::getMessageQueue()
{
    return &m_messageQueue;
}

/*!
 * \brief CANSimulatorCore::getFrameQueue
 * Get CAN frame queue (used for testing)
 * \return pointer to CAN frame queue
 */
std::map<std::uint64_t, canFrameQueueItem> *CANSimulatorCore::getFrameQueue()
{
    if (m_ascReader) {
        return &m_ascReader->getFrameQueue();
    }
    return NULL;
}

/*!
 * \brief CANSimulatorCore::setMessageFilterState
 * Set message ID filtering
 * \param id: message ID
 * \param filterState: true to be filtered, false not to be filtered
 * return true if filter was set correctly, false otherwise
 */
bool CANSimulatorCore::setMessageFilterState(std::uint32_t id, bool filterState)
{
    std::map<uint32_t, bool>::iterator message;
    if ((message = m_filterList.find(id)) != m_filterList.end()) {
        message->second = filterState;
        return true;
    }
    LOG(LOG_ERR, "error=2, Message ID: %d not found in filter-database!\n", id);
    return false;
}

/*!
 * \brief CANSimulatorCore::isMessageFiltered
 * Check if message is filtered
 * \param id: message ID
 * \return true if message is to be filtered, false if not
 */
bool CANSimulatorCore::isMessageFiltered(std::uint32_t id)
{
    std::map<uint32_t, bool>::iterator message;
    if ((message = m_filterList.find(id)) != m_filterList.end()) {
        return message->second;
    }
    return false;
}

/*! \brief CANSimulatorCore::initializeMessageFilterList
 * Initialize message filtering list based on message sources
 * \param ids: list of message IDs to be filtered, if NULL or empty apply to all messages
 * \param filterState: true=exclude given messages, false=include only given messages
 * \param reset: set true if all messages are needed to be re-read
 * \return true if list was initialized, false if not
 */
bool CANSimulatorCore::initializeMessageFilterList(std::vector<std::string> *ids, bool filterState, bool reset)
{
    if (reset) {
        m_filterList.clear();
    }
    // Read all messages from the used source database
    if (m_filterList.size() == 0) {
        bool success = false;
        if (m_ascReader) {
            success = m_ascReader->createFilterList(m_filterList);
        } else if (m_config) {
            success = m_config->createFilterList(m_filterList);
        }
        if (!success) {
            LOG(LOG_ERR, "error=2 No messages found to be filtered!\n");
            return false;
        }
    }

    if (ids && ids->size() > 0) {
        // Set all filtering states opposite of the final filtering state
        for (auto it = m_filterList.begin(); it != m_filterList.end(); ++it) {
            it->second = !filterState;
        }
        // Set filtering states from the list
        for (auto it = ids->begin(); it != ids->end(); ++it) {
            uint32_t targetId = 0;
            try {
                if (!(*it).compare(0, 2, "0x")) {
                    targetId = std::stoul(*it, NULL, 16);
                } else {
                    targetId = std::stoul(*it);
                }
            }
            catch (const std::invalid_argument &) {
                LOG(LOG_ERR, "error=2 %s is not a valid message ID\n", (*it).c_str());
                return false;
            }
            if (!setMessageFilterState(targetId, filterState)) {
                return false;
            }
        }
    } else {
        for (auto it = m_filterList.begin(); it != m_filterList.end(); ++it) {
            it->second = filterState;
        }
    }

    int includeCount = 0;
    // Calculate messages that aren't filtered
    for (auto it = m_filterList.begin(); it != m_filterList.end(); ++it) {
        includeCount += (!it->second) ? 1 : 0;
    }
    // Fail on empty include list only if some ids were given
    if (!includeCount && ids && ids->size() > 0) {
        return false;
    }
    return (m_filterList.size() > 0);
}

/*!
 * \brief CANSimulatorCore::getErrorMetrics
 * Get simulator error metrics
 * \return simulator error metrics
 */
struct errorMetrics CANSimulatorCore::getErrorMetrics() const
{
    return m_errorMetrics;
}
