/*!
 * \file
 * \brief test_CLI_floodmode.cpp foo
 */

#include "../cli/flood.cpp"
#include "../cli/commandlineparser.cpp"
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
#include "../lib/configuration.cpp"
#include "../lib/logger.cpp"
#include "../lib/metrics.cpp"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <gtest/gtest.h>

TEST(LIB_floodmode, missing_handler) {
    ASSERT_THROW(CANSimulatorFloodMode *floodmode = new CANSimulatorFloodMode(NULL), CANSimulatorFloodException);
}

TEST(LIB_floodmode, test_no_settings) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator));

    ASSERT_EQ(floodmode->getDelay(), 100);
    ASSERT_EQ(floodmode->getRate(), 0);
    ASSERT_EQ(floodmode->getRateFactor(), 0);
    ASSERT_EQ(floodmode->getWaitTime(), 0);
    ASSERT_EQ(floodmode->getUseInterval(), 100);
    ASSERT_FALSE(floodmode->getUseRate());
}

TEST(LIB_floodmode, test_delay_settings) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator));

    floodmode->forceBitrate(500000);
    floodmode->setDelay(500);
    ASSERT_EQ(floodmode->getDelay(), 500);
    floodmode->setDelay("300");
    ASSERT_EQ(floodmode->getDelay(), 300);
    ASSERT_EQ(floodmode->getRate(), 0);
    ASSERT_EQ(floodmode->getRateFactor(), 0);
    ASSERT_EQ(floodmode->getWaitTime(), 0);
    floodmode->setDelay("250");
    ASSERT_EQ(floodmode->getUseInterval(), 250);
    ASSERT_FALSE(floodmode->getUseRate());
}

TEST(LIB_floodmode, test_rate_settings) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator));

    floodmode->forceBitrate(500000);
    ASSERT_EQ(floodmode->getDelay(), 100);
    ASSERT_EQ(floodmode->getWaitTime(), 0);
    floodmode->setRate(40);
    ASSERT_EQ(floodmode->getRate(), 40);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 5);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate("25");
    ASSERT_EQ(floodmode->getRate(), 25);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 8);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate(100);
    ASSERT_EQ(floodmode->getRate(), 100);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 2);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate(0);
    ASSERT_EQ(floodmode->getRate(), 1);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 200);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->forceBitrate(250000);
    floodmode->setRate(40);
    ASSERT_EQ(floodmode->getRate(), 40);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 10);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate("75");
    ASSERT_EQ(floodmode->getRate(), 75);
    ASSERT_NEAR(floodmode->getRateFactor(), 5.3, 0.1);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate(100);
    ASSERT_EQ(floodmode->getRate(), 100);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 4);
    ASSERT_TRUE(floodmode->getUseRate());

    floodmode->setRate(0);
    ASSERT_EQ(floodmode->getRate(), 1);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 400);
    ASSERT_TRUE(floodmode->getUseRate());
}

TEST(LIB_floodmode, test_burstmode) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator));

    ASSERT_FALSE(floodmode->getBurstEnabled());
    floodmode->setBurstLen("300");
    ASSERT_TRUE(floodmode->getBurstEnabled());
    ASSERT_EQ(floodmode->getBurstLen(), 300);
    ASSERT_EQ(floodmode->getBurstDelay(), 300);

    floodmode->setBurstDelay("900");
    ASSERT_EQ(floodmode->getBurstLen(), 300);
    ASSERT_EQ(floodmode->getBurstDelay(), 900);
}

TEST(LIB_floodmode, test_include_and_delay) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    std::vector<std::string> input;
    input.push_back("delay=500");
    input.push_back("include=test2sig1,test4sig2,test10sig1");
    input.push_back("burst-len=350");

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator, &input));

    ASSERT_EQ(floodmode->getDelay(), 500);
    ASSERT_EQ(floodmode->getBurstLen(), 350);
    ASSERT_EQ(floodmode->getBurstDelay(), 350);

    ASSERT_TRUE(floodmode->getMessageExists("test2sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test4sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test10sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig2"));
}

TEST(LIB_floodmode, test_exclude_and_rate) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;

    std::vector<std::string> input;
    input.push_back("rate=40");
    input.push_back("exclude=test2sig1,test4sig2,test10sig1");
    input.push_back("burst-len=550");
    input.push_back("burst-delay=2350");

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator, &input));

    ASSERT_EQ(floodmode->getRate(), 40);
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 0);
    floodmode->forceBitrate(500000);
    floodmode->setRate("40");
    ASSERT_DOUBLE_EQ(floodmode->getRateFactor(), 5);
    ASSERT_EQ(floodmode->calculateDelay("test1sig1"), 540);
    ASSERT_EQ(floodmode->calculateDelay("test2sig1"), 460);
    ASSERT_EQ(floodmode->calculateDelay("test8sig1"), 540);
    ASSERT_EQ(floodmode->calculateDelay("testing"), 0);
    ASSERT_EQ(floodmode->getBurstLen(), 550);
    ASSERT_EQ(floodmode->getBurstDelay(), 2350);

    ASSERT_FALSE(floodmode->getMessageExists("test2sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test2sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test4sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test10sig2"));
}
