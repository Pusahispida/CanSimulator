/*!
 * \file
 * \brief test_LIB_filters.cpp foo
 */

#include "dummy_logger.h"

#include "../cli/flood.cpp"
#include "../lib/ascreader.cpp"
#include "../lib/canmessage.cpp"
#include "../lib/cansignal.cpp"
#include "../lib/cansimulatorcore.cpp"
#include "../lib/cantransceiver.cpp"
#include "../lib/can-dbcparser/attribute.cpp"
#include "../lib/can-dbcparser/dbciterator.cpp"
#include "../lib/can-dbcparser/message.cpp"
#include "../lib/can-dbcparser/signal.cpp"
#include "../lib/configuration.cpp"
#include "../lib/metrics.cpp"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <gtest/gtest.h>

TEST(LIB_filters, message_filters_exclude) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));

    input.push_back("1");
    input.push_back("3");
    input.push_back("5");
    input.push_back("7");
    input.push_back("9");

    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, true));

    ASSERT_TRUE(canSimulator->isMessageFiltered(1));
    ASSERT_FALSE(canSimulator->isMessageFiltered(2));
    ASSERT_TRUE(canSimulator->isMessageFiltered(3));
    ASSERT_FALSE(canSimulator->isMessageFiltered(4));
    ASSERT_TRUE(canSimulator->isMessageFiltered(5));
    ASSERT_FALSE(canSimulator->isMessageFiltered(6));
    ASSERT_TRUE(canSimulator->isMessageFiltered(7));
    ASSERT_FALSE(canSimulator->isMessageFiltered(8));
    ASSERT_TRUE(canSimulator->isMessageFiltered(9));
    ASSERT_FALSE(canSimulator->isMessageFiltered(10));
}

TEST(LIB_filters, message_filters_include) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));

    input.push_back("1");
    input.push_back("3");
    input.push_back("5");
    input.push_back("7");
    input.push_back("9");

    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, false));

    ASSERT_FALSE(canSimulator->isMessageFiltered(1));
    ASSERT_TRUE(canSimulator->isMessageFiltered(2));
    ASSERT_FALSE(canSimulator->isMessageFiltered(3));
    ASSERT_TRUE(canSimulator->isMessageFiltered(4));
    ASSERT_FALSE(canSimulator->isMessageFiltered(5));
    ASSERT_TRUE(canSimulator->isMessageFiltered(6));
    ASSERT_FALSE(canSimulator->isMessageFiltered(7));
    ASSERT_TRUE(canSimulator->isMessageFiltered(8));
    ASSERT_FALSE(canSimulator->isMessageFiltered(9));
    ASSERT_TRUE(canSimulator->isMessageFiltered(10));
}

TEST(LIB_filters, individual_message_filters) {
    CANSimulatorCore *canSimulator = NULL;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_TRUE(canSimulator->initializeMessageFilterList(NULL, false));

    ASSERT_TRUE(canSimulator->setMessageFilterState(1, false));
    ASSERT_TRUE(canSimulator->setMessageFilterState(2, false));
    ASSERT_TRUE(canSimulator->setMessageFilterState(3, true));
    ASSERT_TRUE(canSimulator->setMessageFilterState(4, false));
    ASSERT_TRUE(canSimulator->setMessageFilterState(5, false));
    ASSERT_TRUE(canSimulator->setMessageFilterState(6, true));
    ASSERT_TRUE(canSimulator->setMessageFilterState(7, false));
    ASSERT_TRUE(canSimulator->setMessageFilterState(8, true));
    ASSERT_TRUE(canSimulator->setMessageFilterState(9, true));
    ASSERT_TRUE(canSimulator->setMessageFilterState(10, false));
    ASSERT_FALSE(canSimulator->setMessageFilterState(500, false));
    ASSERT_FALSE(canSimulator->setMessageFilterState(5600, false));

    ASSERT_FALSE(canSimulator->isMessageFiltered(1));
    ASSERT_FALSE(canSimulator->isMessageFiltered(2));
    ASSERT_TRUE(canSimulator->isMessageFiltered(3));
    ASSERT_FALSE(canSimulator->isMessageFiltered(4));
    ASSERT_FALSE(canSimulator->isMessageFiltered(5));
    ASSERT_TRUE(canSimulator->isMessageFiltered(6));
    ASSERT_FALSE(canSimulator->isMessageFiltered(7));
    ASSERT_TRUE(canSimulator->isMessageFiltered(8));
    ASSERT_TRUE(canSimulator->isMessageFiltered(9));
    ASSERT_FALSE(canSimulator->isMessageFiltered(10));
}

TEST(LIB_filters, no_filtering) {
    CANSimulatorCore *canSimulator = NULL;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    ASSERT_TRUE(canSimulator->initializeMessageFilterList(NULL, false));
    ASSERT_FALSE(canSimulator->isMessageFiltered(1));
    ASSERT_FALSE(canSimulator->isMessageFiltered(2));
    ASSERT_FALSE(canSimulator->isMessageFiltered(3));
    ASSERT_FALSE(canSimulator->isMessageFiltered(4));
    ASSERT_FALSE(canSimulator->isMessageFiltered(5));
    ASSERT_FALSE(canSimulator->isMessageFiltered(6));
    ASSERT_FALSE(canSimulator->isMessageFiltered(7));
    ASSERT_FALSE(canSimulator->isMessageFiltered(8));
    ASSERT_FALSE(canSimulator->isMessageFiltered(9));
    ASSERT_FALSE(canSimulator->isMessageFiltered(10));
}

TEST(LIB_filters, asc_filtering) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("", "", "tests.asc", ""));

    std::map<std::uint64_t, canFrameQueueItem> *queue = canSimulator->getFrameQueue();
    ASSERT_EQ(3, queue->size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue->begin();

    input.push_back("296");
    input.push_back("2147483816");

    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, true));
    ASSERT_TRUE(canSimulator->isMessageFiltered(it->second.frame.can_id));
    ++it;
    ASSERT_FALSE(canSimulator->isMessageFiltered(it->second.frame.can_id));
    ++it;
    ASSERT_TRUE(canSimulator->isMessageFiltered(it->second.frame.can_id));
}

TEST(LIB_filters, asc_filtering_hexadecimal) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("", "", "tests.asc", ""));

    std::map<std::uint64_t, canFrameQueueItem> *queue = canSimulator->getFrameQueue();
    ASSERT_EQ(3, queue->size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue->begin();

    input.push_back("0x129");
    input.push_back("0x800000a8");

    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, true));
    ASSERT_FALSE(canSimulator->isMessageFiltered(it->second.frame.can_id));
    ++it;
    ASSERT_TRUE(canSimulator->isMessageFiltered(it->second.frame.can_id));
    ++it;
    ASSERT_TRUE(canSimulator->isMessageFiltered(it->second.frame.can_id));
}

TEST(LIB_filters, floodmode_filtering_exclude) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));

    input.push_back("2");
    input.push_back("4");
    input.push_back("6");
    input.push_back("8");
    input.push_back("10");
    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, true));

    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator, NULL));

    ASSERT_TRUE(floodmode->getMessageExists("test1sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig3"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig4"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig3"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig4"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig5"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig3"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig3"));
    ASSERT_TRUE(floodmode->getMessageExists("test5sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test5sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test6sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test6sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test7sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test7sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test8sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test8sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test9sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test9sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig2"));
}

TEST(LIB_filters, floodmode_filtering_include) {
    CANSimulatorFloodMode *floodmode = NULL;
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;

    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));

    input.push_back("1");
    input.push_back("3");
    input.push_back("5");
    input.push_back("7");
    input.push_back("9");
    ASSERT_TRUE(canSimulator->initializeMessageFilterList(&input, false));

    ASSERT_NO_THROW(floodmode = new CANSimulatorFloodMode(canSimulator, NULL));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig3"));
    ASSERT_TRUE(floodmode->getMessageExists("test1sig4"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig3"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig4"));
    ASSERT_FALSE(floodmode->getMessageExists("test2sig5"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test3sig3"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test4sig3"));
    ASSERT_TRUE(floodmode->getMessageExists("test5sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test5sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test6sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test6sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test7sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test7sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test8sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test8sig2"));
    ASSERT_TRUE(floodmode->getMessageExists("test9sig1"));
    ASSERT_TRUE(floodmode->getMessageExists("test9sig2"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig1"));
    ASSERT_FALSE(floodmode->getMessageExists("test10sig2"));
}

TEST(LIB_filters, test_wrong_messages) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));

    input.push_back("110");
    input.push_back("9");
    ASSERT_FALSE(canSimulator->initializeMessageFilterList(&input, false));

    input.clear();
    input.push_back("1");
    input.push_back("5");
    input.push_back("70");
    ASSERT_FALSE(canSimulator->initializeMessageFilterList(&input, true));
}

TEST(LIB_filters, test_filter_all) {
    CANSimulatorCore *canSimulator = NULL;
    std::vector<std::string> input;
    ASSERT_NO_THROW(canSimulator = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
    input.clear();
    input.push_back("1");
    input.push_back("2");
    input.push_back("3");
    input.push_back("4");
    input.push_back("5");
    input.push_back("6");
    input.push_back("7");
    input.push_back("8");
    input.push_back("9");
    input.push_back("10");
    input.push_back("11");
    input.push_back("12");
    input.push_back("13");
    ASSERT_FALSE(canSimulator->initializeMessageFilterList(&input, true));
}
