/*!
* \file
* \brief test_LIB_configuration.cpp foo
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

TEST(LIB_configuration, missing_files) {
  ASSERT_THROW(Configuration *config = new Configuration("", ""), ConfigurationException);
  ASSERT_THROW(Configuration *config = new Configuration("tests.cfg", ""), ConfigurationException);
  ASSERT_THROW(Configuration *config = new Configuration("", "tests.dbc"), ConfigurationException);
}

TEST(LIB_configuration, test_configs) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
  ASSERT_TRUE(config->isVariableSupported("test1sig3"));
  ASSERT_FALSE(config->isVariableSupported("wrong"));
  ASSERT_EQ(12, config->getSendIDs().size());
  ASSERT_EQ(1, config->getReceiveIDs().size());
  ASSERT_EQ(38, config->getVariables().size());
  ASSERT_EQ("2018-03-26 rev. 1", config->getDBCVersion());
  ASSERT_EQ("2018-03-26 rev. 1", config->getCfgVersion());
}

TEST(LIB_configuration, test_message) {
  Configuration *config = NULL;
  unsigned int msgId = 0;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
  ASSERT_TRUE(config->getMessageId("test1sig3", msgId));
  ASSERT_EQ(1, msgId);
  ASSERT_FALSE(config->getMessageId("wrong", msgId));
  ASSERT_EQ("TEST_1", config->getMessage(1)->getName());
  ASSERT_EQ(1, config->getMessage("test1sig1")->getId());
  ASSERT_EQ(0, config->getMessage("wrong"));
}

TEST(LIB_configuration, test_signal) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
  ASSERT_EQ("TEST_1_SIG_1", config->getSignal("test1sig1")->getName());
  ASSERT_EQ(0, config->getSignal("wrong"));
  ASSERT_EQ(0, config->getValue("test1sig1").toInt());
  ASSERT_EQ(0, config->getValue("test1sig1").toInt());
  ASSERT_EQ(0, config->getSignal("test1sig1")->getMinimum());
  ASSERT_EQ(3, config->getSignal("test1sig1")->getMaximum());
  ASSERT_FALSE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(100, config->getValue("test1sig3").toInt());
  ASSERT_EQ(100, config->getDefaultValue("test1sig3").toInt());
  ASSERT_TRUE(config->setValue("test1sig3", "100"));
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_TRUE(config->setValue("test1sig3", "1"));
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(1, config->getValue("test1sig3").toInt());
  ASSERT_TRUE(config->setValue("test1sig4", "4294967295"));
  ASSERT_EQ(4294967295, config->getValue("test1sig4").toUnsigned());
  config->setDefaultValues();
  ASSERT_FALSE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(100, config->getValue("test1sig3").toInt());
  ASSERT_TRUE(config->setValue("test1sig1", "0"));
  ASSERT_EQ(0, config->getValue("test1sig1").toInt());
  ASSERT_TRUE(config->setValue("test1sig1", "3"));
  ASSERT_EQ(3, config->getValue("test1sig1").toInt());
  ASSERT_FALSE(config->setValue("test1sig1", "-1"));
  ASSERT_FALSE(config->setValue("test1sig1", "4"));
  ASSERT_EQ(-265, config->getValue("test4sig2").toInt());
  ASSERT_EQ(10, config->getValue("test4sig3").toInt());
  ASSERT_FALSE(config->setValue("test4sig1", "1"));
  Value val = Value(0);
  ASSERT_TRUE(config->setValue("test1sig1", val));
  ASSERT_EQ(0, config->getValue("test1sig1").toInt());
  val = Value(3);
  ASSERT_TRUE(config->setValue("test1sig1", val));
  ASSERT_EQ(3, config->getValue("test1sig1").toInt());
  val = Value(-1);
  ASSERT_FALSE(config->setValue("test1sig1", val));
  val = Value(4);
  ASSERT_FALSE(config->setValue("test1sig1", val));
  ASSERT_TRUE(config->setValue("test2sig4", "0.5"));
  ASSERT_EQ(0.5f, config->getValue("test2sig4").toDouble());
}

TEST(LIB_configuration, test_suppress_defaults) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc", true));
  ASSERT_EQ("TEST_1_SIG_1", config->getSignal("test1sig1")->getName());
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(100, config->getValue("test1sig3").toInt());
  ASSERT_EQ(100, config->getDefaultValue("test1sig3").toInt());
  ASSERT_TRUE(config->setValue("test1sig3", "100"));
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_TRUE(config->setValue("test1sig3", "1"));
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(1, config->getValue("test1sig3").toInt());
  config->setDefaultValues();
  ASSERT_TRUE(config->getSignal("test1sig3")->isValueSet());
  ASSERT_EQ(100, config->getValue("test1sig3").toInt());
}

TEST(LIB_configuration, test_ignore_directions) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc", false, true));
  ASSERT_TRUE(config->setValue("test4sig1", "1"));
}

TEST(LIB_configuration, test_attributes) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
  ASSERT_EQ(5, config->getAttributes().size());
  ASSERT_EQ("Test", config->getAttribute("TestAttrGlobal")->getValue());
  ASSERT_EQ("2018", config->getAttribute("VersionYear")->getValue());
  ASSERT_EQ("3", config->getAttribute("VersionMonth")->getValue());
  ASSERT_EQ("26", config->getAttribute("VersionDay")->getValue());
  ASSERT_EQ("1", config->getAttribute("VersionNumber")->getValue());
  ASSERT_EQ(4, config->getMessage(1)->getAttributes().size());
  ASSERT_EQ(2, config->getMessage(1)->getAttribute("TestAttrInt")->toInt());
  ASSERT_EQ("INT", config->getMessage(1)->getAttribute("TestAttrInt")->getValueType());
  ASSERT_EQ("Test", config->getMessage(1)->getAttribute("TestAttrString")->getValue());
  ASSERT_EQ("STRING", config->getMessage(1)->getAttribute("TestAttrString")->getValueType());
  ASSERT_EQ("ENUM", config->getMessage(1)->getAttribute("TestAttrEnum")->getValueType());
  ASSERT_EQ("No", config->getMessage(1)->getAttribute("TestAttrEnum")->getEnumValues()[0]);
  ASSERT_EQ("Yes", config->getMessage(1)->getAttribute("TestAttrEnum")->getEnumValues()[1]);
  ASSERT_EQ("Yes", config->getMessage(1)->getAttribute("TestAttrEnum")->getValue());
  ASSERT_EQ("FLOAT", config->getMessage(1)->getAttribute("TestAttrFloat")->getValueType());
  ASSERT_EQ(2.0f, config->getMessage(1)->getAttribute("TestAttrFloat")->toFloat());
  ASSERT_EQ(4, config->getMessage(2)->getAttributes().size());
  ASSERT_EQ(3, config->getMessage(2)->getAttribute("TestAttrInt")->toInt());
  ASSERT_EQ("", config->getMessage(2)->getAttribute("TestAttrString")->getValue());
  ASSERT_EQ("No", config->getMessage(3)->getAttribute("TestAttrEnum")->getValue());
  ASSERT_EQ(1, config->getSignal("test1sig2")->getAttributes().size());
  ASSERT_EQ(10, config->getSignal("test1sig2")->getAttribute("TestAttrSignal")->toInt());
  ASSERT_EQ(1, config->getSignal("test1sig1")->getAttributes().size());
  ASSERT_EQ(1, config->getSignal("test1sig1")->getAttribute("TestAttrSignal")->toInt());
}

TEST(LIB_configuration, test_conversion_types) {
  ASSERT_EQ(unitToConversionType("none"), ConvertTo::NONE);
  ASSERT_EQ(unitToConversionType("mi"), ConvertTo::MI);
  ASSERT_EQ(unitToConversionType("km"), ConvertTo::KM);
  ASSERT_EQ(unitToConversionType("km/h"), ConvertTo::KMH);
  ASSERT_EQ(unitToConversionType("mi/h"), ConvertTo::MPH);
  ASSERT_EQ(unitToConversionType("m/s"), ConvertTo::MS);
  ASSERT_EQ(unitToConversionType("F"), ConvertTo::F);
  ASSERT_EQ(unitToConversionType("K"), ConvertTo::K);

  long double value = 54200;
  ASSERT_FALSE(unitConversion(value, ConvertTo::NONE));
  ASSERT_EQ(54200, value);
}

#define MAXINT 2147483647

TEST(LIB_configuration, test_conversion_MI) {
  long double value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_NEAR(1334384.0612, value, 0.001);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_NEAR(-1334384.0612, value, 0.001);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_DOUBLE_EQ(0, value);

  value = 80467.2;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_NEAR(50, value, 0.001);

  value = 141622;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_NEAR(88, value, 0.001);

  value = 193121;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MI));
  ASSERT_NEAR(120, value, 0.001);

  value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_NEAR(1334384.0612, value, 0.001);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_NEAR(-1334384.0612, value, 0.001);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_DOUBLE_EQ(0, value);

  value = 80467.2;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_NEAR(50, value, 0.001);

  value = 141622.28;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_NEAR(88, value, 0.001);

  value = 193121.28;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MPH));
  ASSERT_NEAR(120, value, 0.001);
}

TEST(LIB_configuration, test_conversion_KM) {
  long double value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(2147483.647, value);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(-2147483.647, value);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(0, value);

  value = 55500;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(55.5, value);

  value = 105820;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(105.82, value);

  value = 1362190;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KM));
  ASSERT_DOUBLE_EQ(1362.19, value);

  value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(2147483.647, value);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(-2147483.647, value);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(0, value);

  value = 50000;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(50, value);

  value = 100000;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(100, value);

  value = 120000;
  ASSERT_TRUE(unitConversion(value, ConvertTo::KMH));
  ASSERT_DOUBLE_EQ(120, value);
}

TEST(LIB_configuration, test_conversion_MS) {
  long double value = MAXINT;

  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_NEAR(596523.71249, value, 0.001);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_NEAR(-596523.71249, value, 0.001);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_DOUBLE_EQ(0, value);

  value = 54000;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_NEAR(15, value, 0.001);

  value = 119999.98;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_NEAR(33.33, value, 0.01);

  value = 1234799.012;
  ASSERT_TRUE(unitConversion(value, ConvertTo::MS));
  ASSERT_NEAR(343, value, 0.001);
}

TEST(LIB_configuration, test_conversion_F) {
  long double value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_DOUBLE_EQ(3865470596.6, value);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_DOUBLE_EQ(-3865470532.6, value);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_DOUBLE_EQ(32, value);

  value = 100;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_DOUBLE_EQ(212, value);

  value = -30;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_DOUBLE_EQ(-22, value);

  value = -17.78;
  ASSERT_TRUE(unitConversion(value, ConvertTo::F));
  ASSERT_NEAR(-0.004, value, 0.001);
}

TEST(LIB_configuration, test_conversion_K) {
  long double value = MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(2147483920.15, value);

  value = -MAXINT;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(-2147483373.85, value);

  value = 0;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(273.15, value);

  value = -273.15;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(0, value);

  value = 99.97;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(373.12, value);

  value = -45;
  ASSERT_TRUE(unitConversion(value, ConvertTo::K));
  ASSERT_DOUBLE_EQ(228.15, value);
}

TEST(LIB_configuration, test_signal_conversion) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));

  ASSERT_TRUE(config->setValue("test5sig1", "10000"));
  ASSERT_DOUBLE_EQ(10000, config->getValue("test5sig1").toDouble());

  ASSERT_TRUE(config->setValue("test5sig2", "160934"));
  ASSERT_NEAR(100, config->getValue("test5sig2").toDouble(), 0.001);

  ASSERT_TRUE(config->setValue("test6sig1", "100000"));
  ASSERT_DOUBLE_EQ(100, config->getValue("test6sig1").toDouble());

  ASSERT_TRUE(config->setValue("test6sig2", "50000"));
  ASSERT_DOUBLE_EQ(50, config->getValue("test6sig2").toDouble());

  ASSERT_TRUE(config->setValue("test7sig1", "141622"));
  ASSERT_NEAR(88, config->getValue("test7sig1").toDouble(), 0.001);

  ASSERT_TRUE(config->setValue("test7sig2", "90000"));
  ASSERT_NEAR(25, config->getValue("test7sig2").toDouble(), 0.001);

  ASSERT_TRUE(config->setValue("test8sig1", "25"));
  ASSERT_NEAR(298.15, config->getValue("test8sig1").toDouble(), 0.001);

  ASSERT_TRUE(config->setValue("test8sig2", "25"));
  ASSERT_DOUBLE_EQ(77, config->getValue("test8sig2").toDouble());
}

TEST(LIB_configuration, test_signal_native_conversion) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));

  CANSimulatorCore::setUseNativeUnits(true);

  ASSERT_TRUE(config->setValue("test5sig1", "10000"));
  ASSERT_DOUBLE_EQ(10000, config->getValue("test5sig1").toDouble());

  ASSERT_TRUE(config->setValue("test5sig2", "160934"));
  ASSERT_DOUBLE_EQ(160934, config->getValue("test5sig2").toDouble());

  ASSERT_TRUE(config->setValue("test6sig1", "100000"));
  ASSERT_DOUBLE_EQ(100000, config->getValue("test6sig1").toDouble());

  ASSERT_TRUE(config->setValue("test6sig2", "50000"));
  ASSERT_DOUBLE_EQ(50000, config->getValue("test6sig2").toDouble());

  ASSERT_TRUE(config->setValue("test7sig1", "141622"));
  ASSERT_DOUBLE_EQ(141622, config->getValue("test7sig1").toDouble());

  ASSERT_TRUE(config->setValue("test7sig2", "90000"));
  ASSERT_DOUBLE_EQ(90000, config->getValue("test7sig2").toDouble());

  ASSERT_TRUE(config->setValue("test8sig1", "25"));
  ASSERT_DOUBLE_EQ(25, config->getValue("test8sig1").toDouble());

  ASSERT_TRUE(config->setValue("test8sig2", "25"));
  ASSERT_DOUBLE_EQ(25, config->getValue("test8sig2").toDouble());

  CANSimulatorCore::setUseNativeUnits(false);
}

TEST(LIB_configuration, test_signal_int_rounding) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));

  ASSERT_TRUE(config->setValue("test9sig1", "53108"));
  ASSERT_EQ(33, config->getValue("test9sig1").toInt());

  ASSERT_TRUE(config->setValue("test9sig1", "88514"));
  ASSERT_EQ(55, config->getValue("test9sig1").toInt());

  ASSERT_TRUE(config->setValue("test9sig2", "33000"));
  ASSERT_EQ(33, config->getValue("test9sig2").toInt());

  ASSERT_TRUE(config->setValue("test9sig2", "55000"));
  ASSERT_EQ(55, config->getValue("test9sig2").toInt());

  ASSERT_TRUE(config->setValue("test10sig1", "1"));
  ASSERT_EQ(34, config->getValue("test10sig1").toInt());

  ASSERT_TRUE(config->setValue("test10sig1", "34"));
  ASSERT_EQ(93, config->getValue("test10sig1").toInt());

  ASSERT_TRUE(config->setValue("test10sig2", "1"));
  ASSERT_EQ(274, config->getValue("test10sig2").toInt());

  ASSERT_TRUE(config->setValue("test10sig2", "-273"));
  ASSERT_EQ(0, config->getValue("test10sig2").toInt());
}

TEST(LIB_configuration, test_signal_rounding) {
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));


  ASSERT_TRUE(config->setValue("test12sig1", "100.49999999999999"));
  ASSERT_EQ(100, config->getValue("test12sig1").toInt());
  ASSERT_EQ(100, config->getSignal("test12sig1")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig1", "100.5"));
  ASSERT_EQ(101, config->getValue("test12sig1").toInt());
  ASSERT_EQ(101, config->getSignal("test12sig1")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig1", "100.55"));
  ASSERT_EQ(101, config->getValue("test12sig1").toInt());
  ASSERT_EQ(101, config->getSignal("test12sig1")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "100.5"));
  ASSERT_NEAR(100.5, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(1005, config->getSignal("test12sig2")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "1.249999999999999"));
  ASSERT_NEAR(1.249999999999999, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(12, config->getSignal("test12sig2")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "1.25"));
  ASSERT_NEAR(1.25, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(13, config->getSignal("test12sig2")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "1.251"));
  ASSERT_NEAR(1.251, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(13, config->getSignal("test12sig2")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "1.26"));
  ASSERT_NEAR(1.26, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(13, config->getSignal("test12sig2")->getRawValue());

  ASSERT_TRUE(config->setValue("test12sig2", "2.25"));
  ASSERT_NEAR(2.25, config->getValue("test12sig2").toDouble(), 0.001);
  ASSERT_EQ(23, config->getSignal("test12sig2")->getRawValue());
}
