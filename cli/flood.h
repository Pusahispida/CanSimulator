/*!
* \file
* \brief flood.h foo
*/

#ifndef FLOOD_H_
#define FLOOD_H_

#include "cansimulatorcore.h"
#include "metrics.h"
#include "value.h"
#include <chrono>
#include <exception>
#include <set>
#include <string>
#include <vector>

typedef std::chrono::high_resolution_clock::time_point hires_tp;

// Microseconds
const int FLOODER_DEFAULT_INTERVAL = 100;

class CANSimulatorFloodException : public std::exception
{
public:
    virtual const char *what() const throw();
};

class CANSimulatorFloodMode
{
public:
    CANSimulatorFloodMode(CANSimulatorCore *canSimulator, std::vector<std::string> *input = NULL);
    // congestion setup
    void setRate(int rate);
    void setRate(std::string rate);
    int getRate() const;
    bool getUseRate() const;
    // delay setup
    void setDelay(int delay);
    void setDelay(std::string delay);
    int getDelay() const;
    int getUseInterval() const;
    // flood setup
    bool floodSignal();
    bool processFloodParams(std::vector<std::string> *input);
    float getRateFactor() const;
    float getWaitTime() const;
    int calculateDelay(const std::string &key);
    // burst setup
    bool getBurstEnabled() const;
    void setBurstLen(int len);
    void setBurstLen(std::string len);
    int getBurstLen() const;
    void setBurstDelay(int delay);
    void setBurstDelay(std::string delay);
    int getBurstDelay() const;
    // Metrics setup
    void initMetrics(MetricsCollector *metrics);
    // For testing only
    void forceBitrate(int bitrate);
    bool getMessageExists(std::string name) const;

private:
    int m_bitrate;
    CANSimulatorCore *m_canSimulator;
    int m_delay;
    int m_rate;
    float m_rateFactor;
    int m_useInterval;
    bool m_useRate;
    uint64_t m_waitTime;

    // Timing and success rates
    hires_tp m_start;

    // Burst
    int m_burstDelay;
    bool m_burstEnabled;
    int m_burstLen;
    uint64_t m_burstWaitTime;

    // Message metrics
    MetricsCollector *m_metrics;

    // Message limiters
    std::set<std::string> m_variables;
    bool checkIncludedMessages(std::vector<std::string> includeList, std::set<std::string> source);
    bool removeExcludedMessages(std::vector<std::string> excludeList, std::set<std::string> source);
    static bool verifySignalsExist(std::vector<std::string> &signalList, std::set<std::string> &source);

    void calculateRateFactor();
    void checkBurstSleep();
    void initTimer();
    Value randomValue(const std::string &key);
    bool filterSignals(std::set<std::string> source);
    bool isSignalFiltered(const std::string &key);
};

#endif // FLOOD_H_
