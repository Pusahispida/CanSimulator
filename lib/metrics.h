/*!
* \file
* \brief metrics.h foo
*/

#ifndef METRICS_H
#define METRICS_H

#include "canmessage.h"
#include "cansimulatorcore.h"
#include <chrono>
#include <cstdint>
#include <exception>
#include <map>
#include <string>

typedef std::chrono::high_resolution_clock::time_point hires_timer;

class MetricsCollectorException : public std::exception
{
public:
    virtual const char *what() const throw();
};

struct messageMetrics {
    MessageDirection direction;   // Direction of the message
    bool std;                     // Standard or extended message
    std::uint64_t successful;     // Counter of successful sends
    std::uint64_t failed;         // Counter of failed sends
    std::uint64_t messageSize;    // Current message size in bits
    std::uint64_t messageTime;    // Calculated time it will take message to go through CAN bus
    std::uint64_t timeTotal;      // Total time used sending the message
    std::int64_t idleTime;        // Possibly calculated overhead the message sending might take
    std::uint64_t idleTotal;      // Total time used idling while sending the message
    std::uint64_t falseDirection; // Counter of messages gone in the wrong direction
};

struct totalMetrics {
    std::uint64_t sent;                 // Counter of send messages
    std::uint64_t TxFailed;             // Counter of failed messages
    std::uint64_t received;             // Counter of received messages
    std::uint64_t totalTime;            // Total time used
    std::uint64_t receiveTime;          // Total time used for receiving
    std::uint64_t sendTime;             // Total time used for sending
    std::uint64_t totalRuntime;         // Total time the CAN-simulator has been running
    std::uint64_t extCount;             // Total number of extended messages
    std::uint64_t stdCount;             // Total number of standard messages
    std::uint64_t totalMessages;        // Total count of all messages
    std::uint64_t totalFalseDirection;  // Total count of all false direction messages
};

struct burstMetrics {
    std::uint64_t sendTime;     // Total time of burst sending
    std::uint64_t delayTime;    // Total time burst was sleeping
    std::uint64_t totalCount;   // Total count of messages send by bursting
    std::uint64_t idleTime;     // Total time used not sending during burst
    int len;                    // Set time for burst sending mode
    int delay;                  // Set time for burst sleeping mode
    int min;                    // Minimum count of messages sent at one burst
    int max;                    // Maximum count of messages sent at one burst
    int count;                  // Current burst send counter (if more than 0, not yet added to totalCount)
};

class MetricsCollector
{
public:
    MetricsCollector(CANSimulatorCore *canSimulator, const std::string &filePath = "");

    void initBurstSettings(int len, int delay);
    void updateBurstData(bool sleep);
    void updateBurstIdleTime();

    void initDelaySend(int delay);
    void initRateSend(float factor);

    void setValueSeparator(char separator);
    bool writeToFile(std::string target = "", bool test = false);

    // For easy external access to all the metrics data (GUI etc)
    struct totalMetrics &getTotalMetrics(bool test = false);
    struct burstMetrics &getBurstMetrics(bool test = false);
    struct messageMetrics *getSingleMessageMetrics(std::uint32_t id, bool test = false);
    std::map<std::uint32_t, messageMetrics> &getMessageMetrics(bool test = false);

    void updateMessages();
    void updateMessage(const CANMessage *message);

    // For testing only
    void forceBitrate(int bitrate) { m_bitrate = bitrate; }
private:
    struct totalMetrics m_totalMetrics;
    struct burstMetrics m_burstMetrics;

    // CAN information
    int m_bitrate;
    float m_rateFactor;
    int m_delayTime;
    char m_valueSeparator;
    CANSimulatorCore *m_canSimulator;

    hires_timer m_start;
    std::string m_outputFile;

    std::map<std::uint32_t, messageMetrics> m_messageMetrics;

    static std::string getCurrentTime();
    void updateTotal();

    bool writeMessageData(std::ofstream &file) const;
    bool writeTotalData(std::ofstream &file) const;
    bool writeBurstData(std::ofstream &file) const;
    bool writeErrorData(std::ofstream &file) const;
};

#endif // METRICS_H
