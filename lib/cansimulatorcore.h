/*!
* \file
* \brief cansimulatorcore.h foo
*/

#ifndef CANSIMULATORCORE_H
#define CANSIMULATORCORE_H

#include "ascreader.h"
#include "cantransceiver.h"
#include "configuration.h"
#include "queue.h"
#include "value.h"
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

struct errorMetrics {
    std::uint64_t errorMessages;        // Total number of error messages
    std::uint64_t unknownMessages;      // Total number of unknown messages
    std::uint64_t errorSize;            // Total size of all error messages
    std::uint64_t unknownSize;          // Total size of all unknown messages
};

class CANSimulatorCoreException : public std::exception
{
  public:

  virtual const char *what() const throw();
};

class CANSimulatorCore
{
public:
    explicit CANSimulatorCore(const std::string &cfg, const std::string &dbc, const std::string &asc,
                              const std::string &socketName, bool suppressDefaults=false, bool ignoreDirections=false,
                              unsigned int interval=10, int runTime=-1);
    ~CANSimulatorCore();
    bool loadConfiguration(const std::string &cfg, const std::string &dbc, bool suppressDefaults, bool ignoreDirections);
    std::string getCfgVersion() const;
    std::string getDBCVersion() const;
    static bool getUseNativeUnits();
    static void setUseNativeUnits(bool enable);
    bool getSendTime() const;
    void setSendTime(bool enable);
    bool getUseUTCTime() const;
    void setUseUTCTime(bool enable);
    // Data simulator
    void startDataSimulator();
    void stopDataSimulator();
    bool isDataSimulatorRunning() const;
    int getRunTime() const;
    int getRunTimeRemaining() const;
    void setRunTime(int runTime);
    // Manual control
    bool setValue(std::string key, Value value);
    bool setValue(std::string key, std::string value);
    bool setValues(std::vector<std::string> &input);
    void setDefaultValues(bool sendMessages = false);
    const CANSignal *getSignal(const std::string &key);
    const CANMessage *getMessage(const std::string &key);
    const CANMessage *getMessage(std::uint32_t id);
    const std::map<std::uint32_t, CANMessage> &getMessages() const;
    const std::set<std::string> &getVariables() const;
    bool sendCANMessage(std::uint32_t id, bool forceSend = false);
    bool sendCANMessage(const std::string &key, bool forceSend = false);
    bool sendCANMessages(bool sendAll = false);
    void startCANReaderThread();
    void startCANSenderThread();
    void stopCANThreads();
    int getCANBitrate();
    Queue<std::shared_ptr<CANMessage>> *getMessageQueue();
    std::map<std::uint64_t, canFrameQueueItem> *getFrameQueue();
    bool setMessageFilterState(std::uint32_t id, bool filterState);
    bool isMessageFiltered(std::uint32_t id);
    bool initializeMessageFilterList(std::vector<std::string> *ids, bool filterState, bool reset = false);
    struct errorMetrics getErrorMetrics() const;

private:
    int m_interval;
    int m_runTime;
    bool m_sendTime;
    bool m_useUTCTime;
    bool m_threadsRunning;
    static bool m_useNativeUnits;
    Configuration *m_config;
    Queue<std::shared_ptr<CANMessage>> m_messageQueue;
    std::thread m_senderThread;
    std::thread m_readerThread;
    std::mutex m_inputMutex;
    std::map<std::uint32_t, bool> m_filterList;
    struct errorMetrics m_errorMetrics;

    ASCReader *m_ascReader;
    CANTransceiver *m_canTransceiver;

    bool m_simulationRunning;
    uint64_t m_simulationTime;

    // Do not copy CANSimulatorCore
    CANSimulatorCore(const CANSimulatorCore&) { }
    void CANReaderThread();
    void CANSenderThread();
    std::uint32_t readCANMessage();
    void updateTime(std::chrono::time_point<std::chrono::system_clock> &timeSendCounter,
                    std::chrono::time_point<std::chrono::system_clock> &now,
                    std::chrono::duration<int,std::milli> &timeSendInterval);
};

#endif // CANSIMULATORCORE_H
