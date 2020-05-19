/*!
 * \file
 * \brief test_LIB_metrics.cpp foo
 */

#include "../lib/metrics.cpp"
#include "../lib/ascreader.cpp"
#include "../lib/can-dbcparser/attribute.cpp"
#include "../lib/can-dbcparser/dbciterator.cpp"
#include "../lib/can-dbcparser/message.cpp"
#include "../lib/can-dbcparser/signal.cpp"
#include "../lib/canerror.cpp"
#include "../lib/canmessage.cpp"
#include "../lib/cansignal.cpp"
#include "../lib/cansimulatorcore.cpp"
#include "../lib/cantransceiver.cpp"
#include "../cli/commandlineparser.cpp"
#include "../lib/configuration.cpp"
#include "../lib/logger.cpp"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <gtest/gtest.h>

TEST(LIB_metrics, missing_handler) {
    ASSERT_THROW(MetricsCollector *metrics = new MetricsCollector(NULL), MetricsCollectorException);
}

TEST(LIB_metrics, message_testing) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    struct messageMetrics *msg = NULL;
    bool suc = false;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    CANMessage *message = new CANMessage(*canSimulator->getMessage("test1sig1"));
    for (int i = 0; i < 50; ++i) {
        suc = !suc;
        message->updateTransfer(suc);
    }
    metrics->updateMessage(message);
    msg = metrics->getSingleMessageMetrics(1, true);
    ASSERT_EQ(msg->successful, 25);
    ASSERT_EQ(msg->failed, 25);
    ASSERT_EQ(msg->messageSize, 108);
    ASSERT_EQ(msg->messageTime, msg->messageSize);
    ASSERT_EQ(msg->timeTotal, msg->messageTime * 50);
    ASSERT_EQ(msg->idleTime, 0);
    ASSERT_EQ(msg->idleTotal, 0);

    msg = metrics->getSingleMessageMetrics(15);
    ASSERT_TRUE(msg == NULL);
}

TEST(LIB_metrics, total_testing) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    struct messageMetrics *msg = NULL;
    struct totalMetrics total;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    metrics->forceBitrate(500000);
    CANMessage *message = new CANMessage(*canSimulator->getMessage("test2sig2"));
    for (int i = 0; i < 50; ++i) {
        message->updateTransfer(true);
    }
    metrics->updateMessage(message);
    ASSERT_EQ(metrics->getMessageMetrics(true).size(), 1);

    msg = metrics->getSingleMessageMetrics(2, true);
    ASSERT_TRUE(msg != NULL);
    ASSERT_EQ(msg->successful, 50);
    ASSERT_EQ(msg->failed, 0);
    ASSERT_EQ(msg->messageSize, 92);
    ASSERT_EQ(msg->messageTime, 184);
    ASSERT_EQ(msg->timeTotal, msg->messageTime * 50);
    ASSERT_EQ(msg->idleTime, 0);
    ASSERT_EQ(msg->idleTotal, 0);

    total = metrics->getTotalMetrics(true);
    ASSERT_EQ(total.sent, 50);
    ASSERT_EQ(total.TxFailed, 0);
    ASSERT_EQ(total.received, 0);
    ASSERT_EQ(total.totalTime, 9200);
    ASSERT_EQ(total.receiveTime, 0);
    ASSERT_EQ(total.sendTime, 9200);
    ASSERT_EQ(total.extCount, 0);
    ASSERT_EQ(total.stdCount, 50);
    ASSERT_EQ(total.totalMessages, 50);
}

TEST(LIB_metrics, idle_congestion) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    struct messageMetrics *msg = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    metrics->initRateSend(5);
    metrics->forceBitrate(500000);
    CANMessage *message = new CANMessage(*canSimulator->getMessage("test8sig1"));
    bool suc = false;
    for (int i = 0; i < 50; ++i) {
        suc = !suc;
        message->updateTransfer(suc);
    }
    metrics->updateMessage(message);
    msg = metrics->getSingleMessageMetrics(8, true);
    ASSERT_EQ(msg->successful, 25);
    ASSERT_EQ(msg->failed, 25);
    ASSERT_EQ(msg->messageSize, 108);
    ASSERT_EQ(msg->messageTime, 216);
    ASSERT_EQ(msg->timeTotal, msg->messageTime * 50);
    ASSERT_EQ(msg->idleTime, 324);
    ASSERT_EQ(msg->idleTotal, 16200);
}

TEST(LIB_metrics, idle_delay) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    struct messageMetrics *msg = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    metrics->initDelaySend(500);
    metrics->forceBitrate(250000);
    CANMessage *message = new CANMessage(*canSimulator->getMessage("test8sig1"));
    for (int i = 0; i < 50; ++i) {
        message->updateTransfer(false);
    }
    metrics->updateMessage(message);

    msg = metrics->getSingleMessageMetrics(8, true);
    ASSERT_EQ(msg->successful, 0);
    ASSERT_EQ(msg->failed, 50);
    ASSERT_EQ(msg->messageSize, 108);
    ASSERT_EQ(msg->messageTime, 432);
    ASSERT_EQ(msg->timeTotal, msg->messageTime * 50);
    ASSERT_EQ(msg->idleTime, 68);
    ASSERT_EQ(msg->idleTotal, 3400);
}

TEST(LIB_metrics, burst_testing) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    struct burstMetrics burst;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    metrics->initDelaySend(200);
    metrics->initBurstSettings(1000, 250);
    for (int i = 1; i < 1001; ++i) {
        if (!(i % 200)) {
            metrics->updateBurstData(false);
        }
        metrics->updateBurstData(!(i % 50));
    }
    metrics->updateBurstData(true);
    burst = metrics->getBurstMetrics(true);
    ASSERT_EQ(burst.sendTime, 21000);
    ASSERT_EQ(burst.delayTime, 5250);
    ASSERT_EQ(burst.totalCount, 985);
    ASSERT_EQ(burst.idleTime, 0);
    ASSERT_EQ(burst.len, 1000);
    ASSERT_EQ(burst.delay, 250);
    ASSERT_EQ(burst.min, 49);
    ASSERT_EQ(burst.max, 50);
    ASSERT_EQ(burst.count, 0);
}

TEST(LIB_metrics, fileWriting) {
    CANSimulatorCore *canSimulator = NULL;
    MetricsCollector *metrics = NULL;
    bool suc = false;
    std::stringstream signalName;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(metrics = new MetricsCollector(canSimulator));

    metrics->forceBitrate(500000);
    metrics->initRateSend(5);
    metrics->initBurstSettings(500, 200);
    for (int j = 1; j < 11; j++) {
        signalName << std::string("test") << j << "sig1";
        CANMessage *message = new CANMessage(*canSimulator->getMessage(signalName.str()));
        suc = !suc;
        for (int i = 0; i < 50; ++i) {
            message->updateTransfer(suc);
        }
        message->setDirection((!(j % 3)) ? MessageDirection::RECEIVE : MessageDirection::SEND);
        metrics->updateMessage(message);
        delete message;
        signalName.str("");
    }
    for (int i = 1; i < 1001; ++i) {
        if (!(i % 200)) {
            metrics->updateBurstData(false);
        }
        metrics->updateBurstData(!(i % 50));
    }

    struct messageMetrics *msg = metrics->getSingleMessageMetrics(4, true);
    ASSERT_TRUE(msg != NULL);
    ASSERT_EQ(msg->successful, 0);
    ASSERT_EQ(msg->failed, 0);
    ASSERT_EQ(msg->messageSize, 76);
    ASSERT_EQ(msg->messageTime, 152);
    ASSERT_EQ(msg->timeTotal, msg->messageTime * 50);
    ASSERT_EQ(msg->idleTime, 228);
    ASSERT_EQ(msg->idleTotal, 0);
    ASSERT_EQ(msg->falseDirection, 50);

    metrics->updateBurstData(true);
    struct burstMetrics burst = metrics->getBurstMetrics(true);
    ASSERT_EQ(burst.sendTime, 10500);
    ASSERT_EQ(burst.delayTime, 4200);
    ASSERT_EQ(burst.totalCount, 985);
    ASSERT_EQ(burst.idleTime, 94800);
    ASSERT_EQ(burst.len, 500);
    ASSERT_EQ(burst.delay, 200);
    ASSERT_EQ(burst.min, 49);
    ASSERT_EQ(burst.max, 50);
    ASSERT_EQ(burst.count, 0);
    ASSERT_EQ(burst.totalCount, 985);

    struct totalMetrics total = metrics->getTotalMetrics(true);
    ASSERT_EQ(total.sent, 150);
    ASSERT_EQ(total.TxFailed, 150);
    ASSERT_EQ(total.received, 100);
    ASSERT_EQ(total.totalTime, 198000);
    ASSERT_EQ(total.receiveTime, 32400);
    ASSERT_EQ(total.sendTime, 70800);
    ASSERT_EQ(total.extCount, 0);
    ASSERT_EQ(total.stdCount, 500);
    ASSERT_EQ(total.totalMessages, 500);
    ASSERT_EQ(total.totalFalseDirection, 50);

    ASSERT_TRUE(metrics->writeToFile("./metricsTestRun.txt", true));
}
