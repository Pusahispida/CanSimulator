/*!
* \file
* \brief test_LIB_cansimulatorcore.cpp foo
*/

#include "dummy_logger.h"

#include "../lib/ascreader.cpp"
#include "../lib/canmessage.cpp"
#include "../lib/cansignal.cpp"
#include "../lib/configuration.cpp"
#include "../lib/cansimulatorcore.cpp"
#include "../lib/cantransceiver.cpp"
#include "../lib/can-dbcparser/attribute.cpp"
#include "../lib/can-dbcparser/dbciterator.cpp"
#include "../lib/can-dbcparser/message.cpp"
#include "../lib/can-dbcparser/signal.cpp"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <gtest/gtest.h>

TEST(LIB_cansimulatorcore, missing_files) {
  ASSERT_THROW(CANSimulatorCore *core = new CANSimulatorCore("", "", "", ""), CANSimulatorCoreException);
  ASSERT_THROW(CANSimulatorCore *core = new CANSimulatorCore("tests.cfg", "", "", ""), CANSimulatorCoreException);
  ASSERT_THROW(CANSimulatorCore *core = new CANSimulatorCore("", "tests.dbc", "", ""), CANSimulatorCoreException);
}

TEST(LIB_cansimulatorcore, test_asc) {
  ASSERT_NO_THROW(CANSimulatorCore *core = new CANSimulatorCore("", "", "tests.asc", ""));
}

TEST(LIB_cansimulatorcore, test_core) {
  CANSimulatorCore *core = NULL;
  ASSERT_NO_THROW(core = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
  ASSERT_EQ("2018-03-26 rev. 1", core->getDBCVersion());
  ASSERT_EQ("2018-03-26 rev. 1", core->getCfgVersion());
  ASSERT_EQ(1, core->getMessage("test1sig1")->getId());
  ASSERT_EQ(0, core->getMessage("wrong"));
  ASSERT_EQ(false, core->getUseNativeUnits());
  core->setUseNativeUnits(true);
  ASSERT_EQ(true, core->getUseNativeUnits());
  ASSERT_EQ(true, core->getSendTime());
  core->setSendTime(false);
  ASSERT_EQ(false, core->getSendTime());
  ASSERT_EQ(false, core->getUseUTCTime());
  core->setUseUTCTime(true);
  ASSERT_EQ(true, core->getUseUTCTime());
  ASSERT_EQ(false, core->isDataSimulatorRunning());
  ASSERT_EQ(-1, core->getRunTime());
  ASSERT_EQ(-1, core->getRunTimeRemaining());
  core->setRunTime(100);
  ASSERT_EQ(100, core->getRunTime());
  ASSERT_EQ(100, core->getRunTimeRemaining());
  ASSERT_EQ(38, core->getVariables().size());
  ASSERT_EQ(0, core->getCANBitrate());

  delete core;
  core = NULL;
  ASSERT_NO_THROW(core = new CANSimulatorCore("tests.cfg", "tests.dbc", "", "", true, true, 100, 1000));
  ASSERT_EQ(38, core->getVariables().size());
  ASSERT_EQ(1000, core->getRunTime());
  ASSERT_EQ(1000, core->getRunTimeRemaining());
}

TEST(LIB_cansimulatorcore, test_set_values) {
  CANSimulatorCore *core = NULL;
  ASSERT_NO_THROW(core = new CANSimulatorCore("tests.cfg", "tests.dbc", "", ""));
  ASSERT_EQ(0, core->getSignal("wrong"));
  ASSERT_EQ(0, core->getSignal("test1sig1")->getValue().toInt());
  ASSERT_EQ(0, core->getSignal("test1sig1")->getMinimum());
  ASSERT_EQ(3, core->getSignal("test1sig1")->getMaximum());
  ASSERT_EQ(0, core->getSignal("test1sig2")->getValue().toInt());
  ASSERT_TRUE(core->setValue("test1sig2", Value(128)));
  ASSERT_TRUE(core->getSignal("test1sig2")->isValueSet());
  ASSERT_EQ(128, core->getSignal("test1sig2")->getValue().toInt());
  ASSERT_EQ(100, core->getSignal("test1sig3")->getValue().toInt());
  ASSERT_TRUE(core->setValue("test1sig3", "1"));
  ASSERT_TRUE(core->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(1, core->getSignal("test1sig3")->getValue().toInt());
  core->setDefaultValues();
  ASSERT_FALSE(core->getSignal("test1sig2")->isValueSet());
  ASSERT_FALSE(core->getSignal("test1sig3")->isValueSet());
  std::vector<std::string> input;
  input.push_back("test1sig1=2");
  input.push_back("test1sig2=5");
  input.push_back("test1sig3=10");
  ASSERT_TRUE(core->setValues(input));
  ASSERT_EQ(2, core->getSignal("test1sig1")->getValue().toInt());
  ASSERT_EQ(5, core->getSignal("test1sig2")->getValue().toInt());
  ASSERT_EQ(10, core->getSignal("test1sig3")->getValue().toInt());
}
