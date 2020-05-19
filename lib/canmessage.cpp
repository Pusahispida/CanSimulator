/*!
* \file
* \brief canmessage.cpp foo
*/

#include "canmessage.h"
#include "logger.h"
#include <cmath>
#include <endian.h>
#include <inttypes.h>
#include <string.h>
#include <sstream>

/*!
 * \brief CANMessage::CANMessage
 * Constructor
 */
CANMessage::CANMessage(const Message &message) :
    m_modified(false),
    m_sendTime(std::chrono::high_resolution_clock::now()),
    m_transferSuccessful(0),
    m_transferFailed(0),
    m_transferFalseDirection(0),
    m_direction(MessageDirection::SEND)
{
    name = message.getName();
    id = message.getId();
    dlc = message.getDlc();
    from = message.getFrom();
    for (auto it = message.begin(); it != message.end(); ++it) {
        m_signals.insert({it->second.getName(), CANSignal(it->second)});
    }
    description = message.getDescription();
    std::map<std::string, Attribute> attributes = message.getAttributes();
    for (const auto &attribute : attributes) {
        attributeList.insert({attribute.first, Attribute(attribute.second)});
    }
}

/*!
 * \brief CANMessage::CANMessage
 * Copy constructor
 * \param message: Source message
 */
CANMessage::CANMessage(const CANMessage &message)
{
    name = message.getName();
    id = message.getId();
    dlc = message.getDlc();
    from = message.getFrom();
    m_direction = message.getDirection();
    m_transferSuccessful = message.getSuccessful();
    m_transferFailed = message.getFailed();
    m_transferFalseDirection = message.getFalseDirection();
    for (auto it = message.m_signals.begin(); it != message.m_signals.end(); ++it) {
        m_signals.insert({it->second.getName(), CANSignal(it->second)});
    }
    description = message.getDescription();
    std::map<std::string, Attribute> attributes = message.getAttributes();
    for (const auto &attribute : attributes) {
        attributeList.insert({attribute.first, Attribute(attribute.second)});
    }
    m_modified = message.m_modified;
}

/*!
 * \brief CANMessage::operator=
 * Assignment operator
 * \param message: Source message
 */
CANMessage& CANMessage::operator=(const CANMessage &message)
{
    name = message.getName();
    id = message.getId();
    dlc = message.getDlc();
    from = message.getFrom();
    m_direction = message.getDirection();
    m_transferSuccessful = message.getSuccessful();
    m_transferFailed = message.getFailed();
    m_transferFalseDirection = message.getFalseDirection();
    for (auto it = message.m_signals.begin(); it != message.m_signals.end(); ++it) {
        m_signals.insert({it->second.getName(), CANSignal(it->second)});
    }
    description = message.getDescription();
    std::map<std::string, Attribute> attributes = message.getAttributes();
    for (const auto &attribute : attributes) {
        attributeList.insert({attribute.first, Attribute(attribute.second)});
    }
    m_modified = message.m_modified;
    return *this;
}

/*!
 * \brief CANMessage::isModified
 * Return whether message content has been modified since last send
 * \return True if message content has been modified, false otherwise.
 */
bool CANMessage::isModified() const
{
    return m_modified;
}

/*!
 * \brief CANMessage::isSendScheduled
 * Return whether message should be sent
 * \param now: Current time
 * \return True if message should be sent, false otherwise.
 */
bool CANMessage::isSendScheduled(std::chrono::time_point<std::chrono::system_clock> &now)
{
    bool ret = false;
    try {
        int cycleTime = 0;
        bool updateSendTime = false;
        if (getAttribute("GenMsgCycleTime")) {
            cycleTime = std::stoi(getAttribute("GenMsgCycleTime")->getValue());
        }
        // Check message first
        std::string msgSendType;
        if (getAttribute("GenMsgSendType")) {
            msgSendType = getAttribute("GenMsgSendType")->getValue();
            if (msgSendType == "Cyclic" && cycleTime > 0 && now >= m_sendTime) {
                updateSendTime = true;
                ret = true;
            } else if (msgSendType == "OnChange" ||
                msgSendType == "OnChangeWithRepetition") {
                if (isModified()) {
                    ret = true;
                }
            }
        }
        // Check also signals if message is not to be sent yet
        if (!ret) {
            for (auto it = m_signals.begin(); it != m_signals.end(); ++it) {
                if (it->second.getAttribute("GenSigSendType")) {
                    std::string sigSendType = it->second.getAttribute("GenSigSendType")->getValue();
                    if ((sigSendType == "Cyclic") && now >= m_sendTime) {
                        updateSendTime = true;
                        ret = true;
                        break;
                    } else if (sigSendType == "OnChange" || sigSendType == "OnChangeWithRepetition") {
                        if (it->second.isModified()) {
                            ret = true;
                            break;
                        }
                    }
                }
            }
        }
        if (updateSendTime) {
            m_sendTime = now + std::chrono::duration<int,std::milli>(cycleTime);
        }
    }
    catch (const std::exception &) {
        return false;
    }
    return ret;
}

/*!
 * \brief CANMessage::setModified
 * Set whether message content has been modified since last send
 * \param modified: Boolean whether message content has been modified since last send
 */
void CANMessage::setModified(bool modified)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_modified = modified;
    if (!modified) {
        for (auto it = m_signals.begin(); it != m_signals.end(); ++it) {
            it->second.setModified(modified);
        }
    }
}

/*!
 * \brief CANMessage::resetValues
 * Reset signals in the message to default values.
 * \param setValues: Define values as set after reset
 */
void CANMessage::resetValues(bool setValues)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto it = m_signals.begin(); it != m_signals.end(); ++it) {
        if (it->second.resetValue(setValues)) {
            m_modified = false;
        }
    }
}

/*!
 * \brief CANMessage::setValue
 * Set a value for a variable name
 * \param key: Variable name
 * \param value: Value for variable name
 * \return True if successful, false otherwise.
 */
bool CANMessage::setValue(const std::string &key, Value &value)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (CANSignal *signal = getSignalPrivate(key)) {
        if (signal->setValue(value)) {
            m_modified = true;
            return true;
        }
    }
    return false;
}

/*!
 * \brief CANMessage::setValue
 * Set a value for a variable name from string
 * \param key: Variable name
 * \param value: Value as a string
 * \return True if successful, false otherwise.
 */
bool CANMessage::setValue(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> guard(m_mutex);
    if (CANSignal *signal = getSignalPrivate(key)) {
        if (signal->setValue(value)) {
            m_modified = true;
            return true;
        }
    }
    return false;
}

/*!
 * \brief CANMessage::getSignal
 * Get CANSignal by signal name
 * \param name: Signal name
 * \return Const pointer to CANSignal object
 */
const CANSignal *CANMessage::getSignal(const std::string &name)
{
    return getSignalPrivate(name);
}

/*!
 * \brief CANMessage::getSignalPrivate
 * Get CANSignal by signal name
 * \param name: Signal name
 * \return Pointer to CANSignal object
 */
CANSignal *CANMessage::getSignalPrivate(const std::string &name)
{
    std::map<std::string, CANSignal>::iterator sig = m_signals.find(name);
    if (sig != m_signals.end()) {
        return &sig->second;
    }  else {
        LOG(LOG_WARN, "warning=3 Signal '%s' not found in dbc file\n", name.c_str());
        return NULL;
    }
}

/*!
 * \brief CANMessage::toString
 * Print information about the message
 * \param details: Add also message attributes
 * \return Compact overview of the message as a string
 */
std::string CANMessage::toString(bool details) const
{
    std::stringstream sstream;
    sstream << "CAN message " << id << " (0x" << std::hex << id << "): " << description << "\n";
    if (details) {
        sstream << "Message attributes:" << "\n";
        // Attributes
        for (std::map<std::string, Attribute>::const_iterator value = attributeList.begin(); value != attributeList.end(); ++value) {
            sstream << "\t" << value->first << ": " << value->second.getValue() << "\n";
        }
    }
    return sstream.str();
}

/*!
 * \brief CANMessage::getSignals
 * Get all CANSignal objects in a map
 * \return Const reference to map containing CANSignal objects
 */
std::map<std::string, CANSignal> &CANMessage::getSignals()
{
    return m_signals;
}

/*!
 * \brief CANSimulatorCore::assembleCANFrame
 * Fill CAN frame with current values of message
 * \param frame: Pointer to canfd_frame to fill
 */
void CANMessage::assembleCANFrame(canfd_frame *frame)
{
    memset(frame, 0, sizeof(struct canfd_frame));

    LOG(LOG_DBG, "Assemble message %u (%#x): %s\n", id, id, name.c_str());
    frame->len = dlc;
    frame->can_id = id;

    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto it = m_signals.begin(); it != m_signals.end(); ++it) {
        LOG(LOG_DBG, "Add to CAN message: %s=%f\n", it->second.getName().c_str(), it->second.getValue().toDouble());
        uint64_t value = it->second.getRawValue();
        // Index of the first part of the value in array
        int startIndex;
        // Calculate start bit for the signal value
        int startBit;
        if (it->second.getByteOrder() == ByteOrder::INTEL) {
            startBit = it->second.getStartbit();
            startIndex = floor(startBit / 8);
        } else {
            //Convert bit numbering to inverted format
            //data will be filled to CAN frame in reverse order
            //Start from the end (dlc-1)
            startBit = (dlc - 1) * 8;
            //Subtract length-1 of the value
            startBit -= (it->second.getLength() - 1);
            //Subtract full bytes of the start bit
            startBit -= floor(it->second.getStartbit() / 8) * 8;
            //Add remainder of the start bit
            startBit += it->second.getStartbit() % 8;
            startIndex = floor(startBit / 8) - (dlc - 1);
        }

        // Amount of unread value data of current signal left in array
        unsigned int dataLeft = it->second.getLength();
        // Offset from array index
        unsigned int startOffset = abs(startBit) % 8;
        // Current offset
        unsigned int offset = startOffset;
        // Current index
        int index = startIndex;
        // Add value to array
        while (dataLeft > 0) {
            // Amount of data to be read from current index
            unsigned int currentSize = ((8 - offset) >= dataLeft) ? dataLeft : (8 - offset);
            // Bit mask for the current index
            unsigned int bitmask = (1 << currentSize) - 1;
            // Add part of the value to current index
            if (startOffset != 0 && (startIndex - index) == 0) {
                frame->data[abs(index)] |= (value & bitmask) << startOffset;
            } else {
                frame->data[abs(index)] |= (value >> (8 * abs(startIndex - index) - startOffset)) & bitmask;
            }
            offset=0;
            // Calculate amount of data left
            dataLeft -= currentSize;
            // Shift index
            index++;
        }
    }
}

/*!
 * \brief CANMessage::parseCANFrame
 * Parse CAN frame and return boolean whether message content had changed
 * \param frame: Pointer to canfd_frame to parse
 * \param canfd: Boolean whether frame is a CAN FD frame
 * \return True if signal values had changed, false otherwise.
 */
bool CANMessage::parseCANFrame(canfd_frame *frame, bool canfd)
{
    // Silence unused variable warning
    (void)canfd;
    bool ret = false;
    LOG(LOG_DBG, "Parse CANFrame %u (%#x), len: %u, CAN FD: %i\n",  frame->can_id, frame->can_id, frame->len, canfd);

    std::lock_guard<std::mutex> guard(m_mutex);
    for (auto it = m_signals.begin(); it != m_signals.end(); ++it) {
        uint64_t value = 0;
        // Index of the first part of the value in array
        int startIndex;
        // Calculate start bit for the signal value
        int startBit;
        if (it->second.getByteOrder() == ByteOrder::INTEL) {
            startBit = it->second.getStartbit();
            startIndex = floor(startBit / 8);
        } else {
            //Convert bit numbering to inverted format
            //data will be filled to CAN frame in reverse order
            //Start from the end (dlc-1)
            startBit = (dlc - 1) * 8;
            //Subtract length-1 of the value
            startBit -= (it->second.getLength() - 1);
            //Subtract full bytes of the start bit
            startBit -= floor(it->second.getStartbit() / 8) * 8;
            //Add remainder of the start bit
            startBit += it->second.getStartbit() % 8;
            startIndex = floor(startBit / 8) - (dlc - 1);
        }

        // Amount of unread value data of current signal left in array
        unsigned int dataLeft = it->second.getLength();
        // Offset from array index
        unsigned int startOffset = abs(startBit) % 8;
        // Current offset
        unsigned int offset = startOffset;
        // Current index
        int index = startIndex;
        // Extract value from array
        while (dataLeft > 0) {
            // Amount of data to be read from current index
            unsigned int currentSize = ((8 - offset) >= dataLeft) ? dataLeft : (8 - offset);
            // Bit mask for the current index
            unsigned int bitmask = (1 << currentSize) - 1;
            // Extract part of the value from current index
            if (startOffset != 0 && (startIndex - index) == 0) {
                value |= (frame->data[abs(index)] >> startOffset) & bitmask;
            } else {
                value |= (frame->data[abs(index)] & bitmask) << (8 * abs(startIndex - index) - startOffset);
            }
            // Calculate amount of data left
            dataLeft -= currentSize;
            // Only start of the value can have offset
            offset = 0;
            // Shift index
            index++;
        }

        // Process extracted value
        if (!it->second.isValueSet() || value != it->second.getRawValue()) {
            if (it->second.setValueFromRaw(value)) {
                ret = true;
            }
        }
    }
    return ret;
}

/*
 * \brief CANMessage::getDirection
 * Get message direction
 * \return MessageDirection enum value
 */
MessageDirection CANMessage::getDirection() const
{
    return m_direction;
}

/*
 * \brief CANMessage::setDirection
 * Set message direction
 * \param direction: direction of the message
 */
void CANMessage::setDirection(MessageDirection direction)
{
    m_direction = direction;
}

/*
 * \brief CANMessage::getSuccessful
 * Get number of successfully send/received messages (depends on direction)
 * \return number of successful messages
 */
uint64_t CANMessage::getSuccessful() const
{
    return m_transferSuccessful;
}

/*
 * \brief CANMessage::getFailed
 * Get number of failed send/received messages (depends on direction)
 * \return number of failed messages
 */
uint64_t CANMessage::getFailed() const
{
    return m_transferFailed;
}

/*!
 * \brief CANMessage::getFalseDirection
 * Get number of messages gone to the wrong direction
 * \return number of wrong direction messages
 */
uint64_t CANMessage::getFalseDirection() const
{
    return m_transferFalseDirection;
}

/*
 * \brief CANMessage::updateTransfer
 * Update count of successful or failed messages transferred
 * \param successful: true if successful, false if failed
 * \param direction: intended direction of the message
 */
void CANMessage::updateTransfer(bool successful, MessageDirection direction)
{
    if (direction != m_direction) {
        m_transferFalseDirection++;
    } else {
        if (successful) {
            m_transferSuccessful++;
        } else {
            m_transferFailed++;
        }
    }
}
