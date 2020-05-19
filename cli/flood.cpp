/*!
* \file
* \brief flood.cpp foo
*/

#include "flood.h"
#include "stringtools.h"
#include <algorithm>
#include <stdlib.h>
#include <unistd.h>

// Previously calculated CAN frame size standard
#define FRAME_SIZE 33

const char *CANSimulatorFloodException::what() const throw()
{
    return "CANSimulatorFloodException";
}

/*!
 * \brief nanoSleep
 * Sleep using nanosleep the given usec time
 * \param time: sleep time in microseconds
 */
void nanoSleep(uint64_t time)
{
    struct timespec req;
    // Change usec to sec
    req.tv_sec = (time / 1000000);
    // Change usec to nsec
    req.tv_nsec = ((time % 1000000) * 1000);
    nanosleep(&req, (struct timespec *)NULL);
}

/*!
 * \brief CANSimulatorFloodMode::CANSimulatorFloodMode
 * Constructor
 * \param canSimulator: pointer to a previously initialized CANSimulatorCore
 * \param input: flood parameter input from command line
 */
CANSimulatorFloodMode::CANSimulatorFloodMode(CANSimulatorCore *canSimulator, std::vector<std::string> *input) :
    m_canSimulator(canSimulator),
    m_delay(FLOODER_DEFAULT_INTERVAL),
    m_rate(0),
    m_rateFactor(0),
    m_useInterval(FLOODER_DEFAULT_INTERVAL),
    m_useRate(false),
    m_waitTime(0),
    m_burstDelay(0),
    m_burstEnabled(false),
    m_burstLen(0),
    m_burstWaitTime(0),
    m_metrics(NULL)
{
    if (!canSimulator) {
        throw CANSimulatorFloodException();
    }
    m_bitrate = m_canSimulator->getCANBitrate();
    initTimer();

    // Read commandline inputs
    if (input) {
        processFloodParams(input);
    } else {
        filterSignals(m_canSimulator->getVariables());
    }
    // Check that there is something to actually send
    if (!m_variables.size()) {
        LOG(LOG_ERR, "error=1, No valid messages found!\n");
        throw CANSimulatorFloodException();
    }
}

/*!
 * brief waitUntil
 * Waits until waitTime microseconds have elapsed since start
 * \param start: Starting time
 * \param waitTime: How many microseconds after start this function will return
 */
void waitUntil(const hires_tp &start, uint64_t waitTime)
{
    hires_tp now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - start;
    uint64_t difference = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    if (difference < waitTime) {
        nanoSleep(waitTime - difference);
    }
}

/*!
 * \brief CANSimulatorFloodMode::checkBurstSleep
 * Check whether it is time for burst send or to sleep
 */
void CANSimulatorFloodMode::checkBurstSleep()
{
    // Initialize burst timer
    if (!m_burstWaitTime) {
        m_burstWaitTime += m_burstLen;
        return;
    }
    hires_tp now = std::chrono::high_resolution_clock::now();
    auto elapsed = now - m_start;
    uint64_t difference = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();

    // If bursting time is up, sleep for the burst delay and set new burst timer
    if (difference >= m_burstWaitTime) {
        if (m_metrics) {
            m_metrics->updateBurstData(true);
        }
        m_burstWaitTime += m_burstDelay;        // Set time to sleep
        m_waitTime += m_burstDelay;             // Set flood timer up to date
        nanoSleep(m_burstWaitTime - difference);
        m_burstWaitTime += m_burstLen;          // Set new bursting time
    }
}

/*!
 * brief randomFromSet
 * Chooses a random item from a std::set
 * \param list: The set where to choose from
 * \return A random item from list
 */
const std::string & randomFromSet(const std::set<std::string> *list)
{
    int which = rand() % list->size();
    std::set<std::string>::iterator it = list->begin();
    for (; which != 0; which--) {
        ++it;
    }
    return *it;
}

/*!
 * brief CANSimulatorFloodMode::randomValue
 * Pick a random value between a signal's min and max values
 * \param key: Key to the signal for which to pick a value
 * \return A random value for the signal
 */
Value CANSimulatorFloodMode::randomValue(const std::string &key)
{
    const CANSignal *signal = m_canSimulator->getSignal(key);
    Value v;

    if (signal->getValue().type() == Value::Type::Integer) {
        int min = (int)signal->getMinimum();
        int max = (int)signal->getMaximum();
        v = Value(rand() % (max - min) + min);
    } else {
        double min = signal->getMinimum();
        double max = signal->getMaximum();
        v = Value(((double)rand() / (double)RAND_MAX) * (max - min) + min);
    }
    return v;
}

/*!
 * \brief CANSimulatorFloodMode::initTimer
 * Initialize flood sleep timer and variable list
 */
void CANSimulatorFloodMode::initTimer()
{
    m_start = std::chrono::high_resolution_clock::now();
    srand((unsigned int)m_start.time_since_epoch().count());
}

/*!
 * brief CANSimulatorFloodMode::floodSignal
 * Sends random signal to CAN bus and waits for specified interval
 * \return true if successful, false if sending failed
 */
bool CANSimulatorFloodMode::floodSignal()
{
    if (getBurstEnabled()) {
        checkBurstSleep();
    }
    const std::string &var = randomFromSet(&m_variables);
    Value v = randomValue(var);
    m_canSimulator->setValue(var, v);

    // Force send messages even if the randomly chosen signal value
    // happens to be the same as it previously was
    bool sent = m_canSimulator->sendCANMessage(var, true);
    if (m_metrics) {
        m_metrics->updateBurstData(false);
    }

    // Use congestion rate to calculate the delay between message sending
    // or use basic delay between messages
    m_waitTime += (getUseRate()) ? calculateDelay(var) : m_useInterval;
    waitUntil(m_start, m_waitTime);
    return sent;
}

/*!
 * \brief CANSimulatorFloodMode::setRate
 * Set congestion rate value and enable rate calculation
 * \param rate: congestion percentage
 */
void CANSimulatorFloodMode::setRate(int rate)
{
    m_rate = rate;
    m_useRate = true;
    calculateRateFactor();
}

/*!
 * \brief CANSimulatorFloodMode::setRate
 * Set congestion rate value from string and enable rate calculation
 * \param rate: congestion percentage as string
 */
void CANSimulatorFloodMode::setRate(std::string rate)
{
    int strRate;
    try {
        strRate = std::stoi(rate);
    }
    catch (const std::invalid_argument &) {
        LOG(LOG_ERR, "error=2 Flood mode rate value is invalid %d.\n", rate);
        throw CANSimulatorFloodException();
    }
    catch (const std::out_of_range &) {
        LOG(LOG_ERR, "error=2 Flood mode rate value is out of range %d.\n", rate);
        throw CANSimulatorFloodException();
    }
    setRate(strRate);
}

/*!
 * \brief CANSimulatorFloodMode::getRate
 * Get congestion percentage
 * \return set congestion rate
 */
int CANSimulatorFloodMode::getRate() const
{
    return m_rate;
}

/*!
 * \brief CANSimulatorFloodMode::calculateRateFactor()
 * Calculate bit-delay rate factor from congestion percentage
 */
void CANSimulatorFloodMode::calculateRateFactor()
{
    // Check if congestion percentage is within 1-100%
    m_rate = (m_rate <= 0) ? 1 : (m_rate > 100) ? 100 : m_rate;
    if (m_rate && m_bitrate) {
        // Time it takes a bit to travel in microseconds
        float uSecFactor = (1000000 / (float)m_bitrate);
        // Time each bit has to use for the CAN congestion flooding
        m_rateFactor = (uSecFactor * (100 / (float)m_rate));
    } else {
        LOG(LOG_ERR, "error=2 No CAN bitrate set %d, using delay (%d usec) instead of congestion\n", m_bitrate, m_delay);
        m_rateFactor = 0;
        m_useRate = false;
    }
}

/*!
 * \brief CANSimulatorFloodMode::getUseInterval
 * Get calculated time interval used by flooding
 * \return currently used delay interval
 */
int CANSimulatorFloodMode::getUseInterval() const
{
    return m_useInterval;
}

/*!
 * \brief CANSimulatorFloodMode::setDelay
 * Set flooding delay
 * \param delay: flooding delay in microseconds
 */
void CANSimulatorFloodMode::setDelay(int delay)
{
    m_delay = delay;
    m_useInterval = delay;
    m_useRate = false;
}

/*!
 * \brief CANSimulatorFloodMode::setDelay
 * Set flooding delay as string
 * \param delay: flooding delay as string
 */
void CANSimulatorFloodMode::setDelay(std::string delay)
{
    int strDelay;
    try {
        strDelay = std::stoi(delay);
    }
    catch (const std::invalid_argument &) {
        LOG(LOG_ERR, "error=2 Flood mode delay value is invalid %d.\n", delay);
        throw CANSimulatorFloodException();
    }
    catch (const std::out_of_range &) {
        LOG(LOG_ERR, "error=2 Flood mode delay value is out of range %d.\n", delay);
        throw CANSimulatorFloodException();
    }
    setDelay(strDelay);
}

/*!
 * \brief CANSimulatorFloodMode::getDelay
 * Get flooding delay time
 * \return flooding delay time in microseconds
 */
int CANSimulatorFloodMode::getDelay() const
{
    return m_delay;
}

/*!
 * \brief CANSimulatorFloodMode::calculateDelay
 * Calculate flooding delay time by congestion rate from the message size
 * \param key: CAN message key to be sent to the CAN
 * \return calculated microsecond delay or 0 in case of an error
 */
int CANSimulatorFloodMode::calculateDelay(const std::string &key)
{
    if (m_bitrate > 0) {
        if (const CANMessage *message = m_canSimulator->getMessage(key)) {
            int messageBits = FRAME_SIZE +                                               // Previously calculated CAN frame size
                (message->getDlc() * 8) +                                                // CAN message size
                ((message->getId() & CAN_EFF_FLAG) ? CAN_EFF_ID_BITS : CAN_SFF_ID_BITS); // Check if standard or extended CAN ID
            m_useInterval = (messageBits * m_rateFactor);
            return m_useInterval;
        }
    }
    return 0;
}

/*!
 * \brief CANSimulatorFloodMode::getUseRate
 * Get current state of using rate or delay as the flooding interval
 * \return true if using congestion rate instead of normal delay
 */
bool CANSimulatorFloodMode::getUseRate() const
{
    return m_useRate;
}

/*!
 * \brief CANSimulatorFloodMode::processFloodParams
 * Go through and parse all the flood-mode parameters and set them accordingly
 * \param input: flood mode parameters settings as a string vector pointer
 * \return true if parsing was successful, otherwise false
 */
bool CANSimulatorFloodMode::processFloodParams(std::vector<std::string> *input)
{
    bool ret = true, variablesInited = false;
    for (std::vector<std::string>::iterator it = input->begin(); it != input->end(); ++it) {
        if ((*it).length() == 0) {
            continue;
        }
        std::vector<std::string> values = split(*it, '=');
        if (values.size() == 2) {
            if (!values[0].compare("delay")) {
                setDelay(values[1]);
            } else if (!values[0].compare("rate")) {
                setRate(values[1]);
            } else if (!values[0].compare("burst-len")) {
                setBurstLen(values[1]);
            } else if (!values[0].compare("burst-delay")) {
                setBurstDelay(values[1]);
            } else if (!values[0].compare("include")) {
                variablesInited = checkIncludedMessages(split(values[1], ','), m_canSimulator->getVariables());
            } else if (!values[0].compare("exclude")) {
                variablesInited = removeExcludedMessages(split(values[1], ','), m_canSimulator->getVariables());
            } else {
                LOG(LOG_WARN, "warning=2 Unknown flood parameter '%s'\n", values[0].c_str());
                ret = false;
            }
        } else {
            setDelay(*it);    // This is to be deprecated, necessary to keep old working style
            LOG(LOG_WARN, "warning=3 Unknown or deprecated flood parameter '%s', using as flood delay\n", (*it).c_str());
            ret = false;
        }
    }
    if (!variablesInited) {
        filterSignals(m_canSimulator->getVariables());
    }
    return ret;
}

/*!
 * \brief CANSimulatorFloodMode::getRateFactor
 * Get calculated microsecond delay factor per CAN bitrate
 * \return currently used delay factor
 */
float CANSimulatorFloodMode::getRateFactor() const
{
    return m_rateFactor;
}

/*!
 * \brief CANSimulatorFloodMode::getWaitTime
 * Get current calculated waitTime
 * \return currently used waitTime
 */
float CANSimulatorFloodMode::getWaitTime() const
{
    return m_waitTime;
}

/*!
 * \brief CANSimulatorFloodMode::forceBitrate
 * Force CAN bitrate to a new bitrate, used for testing only
 * \param bitrate: new bitrate
 */
void CANSimulatorFloodMode::forceBitrate(int bitrate)
{
    m_bitrate = bitrate;
}

/*!
 * \brief CANSimulatorFloodMode::getMessageExists
 * Check if signal exists in the current variable list, used for testing only
 * \param name: name of the signal
 * \return true if found, false if not
 */
bool CANSimulatorFloodMode::getMessageExists(std::string name) const
{
    if (m_variables.find(name) != m_variables.end()) {
        return true;
    }
    return false;
}

/*!
 * \brief CANSimulatorFloodMode::getBurstEnabled
 * Get current mode of burst flooding
 * \return current state of burst flooding
 */
bool CANSimulatorFloodMode::getBurstEnabled() const
{
    return m_burstEnabled;
}

/*!
 * \brief CANSimulatorFloodMode::setBurstLen
 * Set bursting time and delay, if delay is not set
 * \param len: burst time in usec
 */
void CANSimulatorFloodMode::setBurstLen(int len)
{
    m_burstLen = len;
    if (!m_burstDelay) {
        m_burstDelay = len;
    }
    m_burstEnabled = true;
}

/*!
 * \brief CANSimulatorFloodMode::setBurstLen
 * Set bursting time and delay, if delay is not set
 * \param len: burst time string in usec
 */
void CANSimulatorFloodMode::setBurstLen(std::string len)
{
    int strLen;
    try {
        strLen = std::stoi(len);
    }
    catch (const std::invalid_argument &) {
        LOG(LOG_ERR, "error=2 Flood mode burst-len is invalid %d.\n", len);
        throw CANSimulatorFloodException();
    }
    catch (const std::out_of_range &) {
        LOG(LOG_ERR, "error=2 Flood mode burst-len is out of range %d.\n", len);
        throw CANSimulatorFloodException();
    }
    setBurstLen(strLen);
}

/*!
 * \brief CANSimulatorFloodMode::getBurstLen
 * Get current bursting time
 * \return Current burst time in usec
 */
int CANSimulatorFloodMode::getBurstLen() const
{
    return m_burstLen;
}

/*!
 * \brief CANSimulatorFloodMode::setBurstDelay
 * Set time between bursting
 * \param delay: time to wait before next burst in usec
 */
void CANSimulatorFloodMode::setBurstDelay(int delay)
{
    m_burstDelay = delay;
    if (!m_burstLen) {
        m_burstLen = delay;
    }
    m_burstEnabled = true;
}

/*!
 * \brief CANSimulatorFloodMode::setBurstDelay
 * Set time between bursting
 * \param delay: time to wait before next burst in usec
 */
void CANSimulatorFloodMode::setBurstDelay(std::string delay)
{
    int strDelay;
    try {
        strDelay = std::stoi(delay);
    }
    catch (const std::invalid_argument &) {
        LOG(LOG_ERR, "error=2 Flood mode burst-delay is invalid %d.\n", delay);
        throw CANSimulatorFloodException();
    }
    catch (const std::out_of_range &) {
        LOG(LOG_ERR, "error=2 Flood mode burst-delay is out of range %d.\n", delay);
        throw CANSimulatorFloodException();
    }
    setBurstDelay(strDelay);
}

/*!
 * \brief CANSimulatorFloodMode::getBurstDelay
 * Get current bursting delay
 * \return current bursting delay in usec
 */
int CANSimulatorFloodMode::getBurstDelay() const
{
    return m_burstDelay;
}

/*!
 * \brief CANSimulatorFloodMode::checkIncludedMessages
 * Check that the given signals for included messages actually exist and remove excessive
 * \param includeList: vector that has all the signals to be included
 * \param source: full list of all loaded signals
 * \return true if new list is created, false if not
 */
bool CANSimulatorFloodMode::checkIncludedMessages(std::vector<std::string> includeList, std::set<std::string> source)
{
    for (std::vector<std::string>::const_iterator it = includeList.begin(); it != includeList.end(); ++it) {
        // Check that the added signal actually is in the used signals list
        if (source.find(*it) != source.end()) {
            if (!isSignalFiltered(*it)) {
                m_variables.insert(*it);
            } else {
                LOG(LOG_WARN, "warning=2 Message of '%s' signal is blocked globally\n", (*it).c_str());
            }
        } else {
            LOG(LOG_WARN, "warning=2 '%s' is not a valid signal!\n", (*it).c_str());
        }
    }
    if (!m_variables.size()) {
        LOG(LOG_WARN, "warning=4 No eligible signals found\n");
        return false;
    }
    return true;
}

/*!
 * \brief CANSimulatorFloodMode::removeExcludedMessages
 * Remove all excluded messages from variable list
 * \param excludeList: vector that has every signal to be excluded from the original signals
 * \param source: full list of all loaded signals
 * \return true if new list is created, false if not
 */
bool CANSimulatorFloodMode::removeExcludedMessages(std::vector<std::string> excludeList, std::set<std::string> source)
{
    verifySignalsExist(excludeList, source);
    for (std::set<std::string>::const_iterator it = source.begin(); it != source.end(); ++it) {
        if (std::find(excludeList.begin(), excludeList.end(), *it) == excludeList.end()) {
            if (!isSignalFiltered(*it)) {
                m_variables.insert(*it);
            } else {
                LOG(LOG_WARN, "warning=2 Message of '%s' signal is blocked globally\n", (*it).c_str());
            }
        }
    }
    if (!m_variables.size()) {
        LOG(LOG_WARN, "warning=4 No signals have been included\n");
        return false;
    }
    return true;
}

/*!
 * \brief CANSimulatorFloodMode::verifySignalsExist
 * Verify that all given signals exist, print warnings for missing signals
 * \param signalList: list of given signals
 * \param source: full list of all signals
 * \return true if all given signals were found, false if not
 */
bool CANSimulatorFloodMode::verifySignalsExist(std::vector<std::string> &signalList, std::set<std::string> &source)
{
    bool ret = true;
    for (std::vector<std::string>::iterator it = signalList.begin(); it != signalList.end(); ++it) {
        if (source.find(*it) == source.end()) {
            LOG(LOG_WARN, "Warning=2 Signal '%s' not found!\n", (*it).c_str());
            ret = false;
        }
    }
    return ret;
}

/*!
 * \brief CANSimulatorFloodMode::filterSignals
 * Check and remove all signals, that are filtered by the simulator
 * \return false if all signals were filtered, true otherwise
 */
bool CANSimulatorFloodMode::filterSignals(std::set<std::string> source)
{
    for (auto it = source.begin(); it != source.end(); ++it) {
        if (!isSignalFiltered(*it)) {
            m_variables.insert(*it);
        }
    }
    if (!m_variables.size()) {
        return false;
    }
    return true;
}

/*!
 * \brief CANSimulatorFloodMode::isSignalFiltered
 * Check if signal is to be filtered
 * \param key: signal name
 * \return true if signal is to be filtered or not found, false otherwise
 */
bool CANSimulatorFloodMode::isSignalFiltered(const std::string &key)
{
    if (CANMessage const *message = m_canSimulator->getMessage(key)) {
        return m_canSimulator->isMessageFiltered(message->getId());
    }
    LOG(LOG_ERR, "error=2 Signal '%s' not found in any messages\n", key.c_str());
    return true;
}

/*!
 * \brief CANSimulatorFloodMode::initMetrics
 * Initialize metrics setup for flood mode
 * \param metrics: previously initialized MetricsCollector class
 */
void CANSimulatorFloodMode::initMetrics(MetricsCollector *metrics)
{
    if (metrics) {
        m_metrics = metrics;
        m_metrics->initBurstSettings(m_burstLen, m_burstDelay);
        if (m_rateFactor > 0) {
            m_metrics->initRateSend(m_rateFactor);
        } else {
            m_metrics->initDelaySend(m_delay);
        }
    }
}
