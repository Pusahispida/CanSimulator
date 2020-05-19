/*!
* \file
* \brief canmessage.h foo
*/

#ifndef CANMESSAGE_H
#define CANMESSAGE_H

#include "cansignal.h"
#include <can-dbcparser/header/message.hpp>
#include <linux/can.h>
#include <chrono>
#include <mutex>
#include <string>
#include <map>

class CANSimulatorCore;
class CANTransceiver;
class Configuration;

enum class MessageDirection {
    SEND = 0,
    RECEIVE = 1
};

class CANMessage : public Message
{
    friend class CANSimulatorCore;
    friend class CANTransceiver;
    friend class Configuration;

public:
    explicit CANMessage(const Message &message);
    CANMessage(const CANMessage &message);
    CANMessage& operator=(const CANMessage & msg);
    const CANSignal *getSignal(const std::string &name);
    std::map<std::string, CANSignal> &getSignals();
    bool isModified() const;
    std::string toString(bool details = false) const;
    void assembleCANFrame(canfd_frame *frame);
    bool parseCANFrame(canfd_frame *frame, bool canfd);
    MessageDirection getDirection() const;
    void setDirection(MessageDirection direction);
    uint64_t getSuccessful() const;
    uint64_t getFailed() const;
    uint64_t getFalseDirection() const;
    void updateTransfer(bool successful, MessageDirection direction = MessageDirection::SEND);

protected:
    bool isSendScheduled(std::chrono::time_point<std::chrono::system_clock> &now);
    void resetValues(bool setValues = false);
    void setModified(bool modified);
    bool setValue(const std::string &key, const std::string &valueString);
    bool setValue(const std::string &key, Value &value);

private:
    bool m_modified;
    std::mutex m_mutex;
    std::chrono::time_point<std::chrono::system_clock> m_sendTime;
    std::map<std::string, CANSignal> m_signals;
    CANSignal *getSignalPrivate(const std::string &name);

    uint64_t m_transferSuccessful;
    uint64_t m_transferFailed;
    uint64_t m_transferFalseDirection;
    MessageDirection m_direction;
};

#endif // CANMESSAGE_H
