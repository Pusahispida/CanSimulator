/*!
* \file
* \brief test_LIB_canframe.cpp foo
*/

#include "dummy_logger.h"

#include "../lib/ascreader.cpp"
#include "../lib/canmessage.cpp"
#include "../lib/cansignal.cpp"
#include "../lib/cansimulatorcore.cpp"
#include "../lib/cantransceiver.cpp"
#include "../lib/configuration.cpp"
#include "../lib/can-dbcparser/attribute.cpp"
#include "../lib/can-dbcparser/dbciterator.cpp"
#include "../lib/can-dbcparser/message.cpp"
#include "../lib/can-dbcparser/signal.cpp"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <linux/can.h>
#include <gtest/gtest.h>

void empty_canframe(canfd_frame &frame) {
    memset(&frame, 0, sizeof(struct canfd_frame));
}

TEST(LIB_canframe, test_assemble) {
    Configuration *config = NULL;
    ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));

    canfd_frame frame;
    empty_canframe(frame);
    config->getMessage("test1sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(25, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test1sig1", "1");
    config->getMessage("test1sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(1, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(25, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test1sig2", "2");
    config->getMessage("test1sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(9, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(25, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test1sig3", "1");
    config->getMessage("test1sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(9, frame.data[0]);
    ASSERT_EQ(64, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test1sig3", "1024");
    config->getMessage("test1sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(9, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(1, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->getMessage("test2sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(127, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test2sig1", "1");
    config->getMessage("test2sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(128, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test2sig2", "1");
    config->getMessage("test2sig2")->assembleCANFrame(&frame);
    ASSERT_EQ(128, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(1, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test2sig3", "8");
    config->getMessage("test2sig3")->assembleCANFrame(&frame);
    ASSERT_EQ(128, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(1, frame.data[2]);
    ASSERT_EQ(128, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    empty_canframe(frame);
    config->setValue("test2sig4", "2");
    config->getMessage("test2sig4")->assembleCANFrame(&frame);
    ASSERT_EQ(128, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(1, frame.data[2]);
    ASSERT_EQ(128, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(32, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test3sig1", "0");
    config->setValue("test3sig2", "0");
    config->setValue("test3sig3", "0");
    config->setValue("test3sig4", "0");
    config->setValue("test3sig5", "0");
    config->getMessage("test3sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test3sig1", "127");
    config->setValue("test3sig2", "32767");
    config->setValue("test3sig3", "65535");
    config->setValue("test3sig4", "12.7");
    config->setValue("test3sig5", "3276.7");
    config->getMessage("test3sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0x7f, frame.data[0]);
    ASSERT_EQ(0xff, frame.data[1]);
    ASSERT_EQ(0x7f, frame.data[2]);
    ASSERT_EQ(0xff, frame.data[3]);
    ASSERT_EQ(0xff, frame.data[4]);
    ASSERT_EQ(0x7f, frame.data[5]);
    ASSERT_EQ(0xff, frame.data[6]);
    ASSERT_EQ(0x7f, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test3sig1", "-128");
    config->setValue("test3sig2", "-32768");
    config->setValue("test3sig3", "0");
    config->setValue("test3sig4", "-12.8");
    config->setValue("test3sig5", "-3276.8");
    config->getMessage("test3sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0x80, frame.data[0]);
    ASSERT_EQ(0x00, frame.data[1]);
    ASSERT_EQ(0x80, frame.data[2]);
    ASSERT_EQ(0x00, frame.data[3]);
    ASSERT_EQ(0x00, frame.data[4]);
    ASSERT_EQ(0x80, frame.data[5]);
    ASSERT_EQ(0x00, frame.data[6]);
    ASSERT_EQ(0x80, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test12sig1", "0");
    config->setValue("test12sig2", "0");
    config->getMessage("test12sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0, frame.data[0]);
    ASSERT_EQ(0, frame.data[1]);
    ASSERT_EQ(0, frame.data[2]);
    ASSERT_EQ(0, frame.data[3]);
    ASSERT_EQ(0, frame.data[4]);
    ASSERT_EQ(0, frame.data[5]);
    ASSERT_EQ(0, frame.data[6]);
    ASSERT_EQ(0, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test12sig1", "2147483647");
    config->setValue("test12sig2", "214748364.7");
    config->getMessage("test12sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0xff, frame.data[0]);
    ASSERT_EQ(0xff, frame.data[1]);
    ASSERT_EQ(0xff, frame.data[2]);
    ASSERT_EQ(0x7f, frame.data[3]);
    ASSERT_EQ(0xff, frame.data[4]);
    ASSERT_EQ(0xff, frame.data[5]);
    ASSERT_EQ(0xff, frame.data[6]);
    ASSERT_EQ(0x7f, frame.data[7]);

    config->setDefaultValues();
    empty_canframe(frame);
    config->setValue("test12sig1", "-2147483648");
    config->setValue("test12sig2", "-214748364.8");
    config->getMessage("test12sig1")->assembleCANFrame(&frame);
    ASSERT_EQ(0x00, frame.data[0]);
    ASSERT_EQ(0x00, frame.data[1]);
    ASSERT_EQ(0x00, frame.data[2]);
    ASSERT_EQ(0x80, frame.data[3]);
    ASSERT_EQ(0x00, frame.data[4]);
    ASSERT_EQ(0x00, frame.data[5]);
    ASSERT_EQ(0x00, frame.data[6]);
    ASSERT_EQ(0x80, frame.data[7]);
}

TEST(LIB_canframe, test_parse) {
    Configuration *config = NULL;
    ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
    canfd_frame frame;

    empty_canframe(frame);
    frame.can_id = 1;
    frame.data[0] = 0xff;
    frame.data[1] = 0xff;
    frame.data[2] = 0xff;
    frame.data[3] = 0xff;
    frame.data[4] = 0xff;
    frame.data[5] = 0xff;
    frame.data[6] = 0xff;
    frame.data[7] = 0xff;
    ASSERT_TRUE(config->getMessage("test1sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(3, config->getSignal("test1sig1")->getValue().toInt());
    ASSERT_EQ(4095, config->getSignal("test1sig2")->getValue().toInt());
    ASSERT_EQ(262143, config->getSignal("test1sig3")->getValue().toInt());
    ASSERT_EQ(4294967295, config->getSignal("test1sig4")->getValue().toUnsigned());

    empty_canframe(frame);
    frame.can_id = 2;
    frame.data[0] = 129;
    ASSERT_TRUE(config->getMessage("test2sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(2, config->getSignal("test2sig1")->getValue().toInt());

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 2;
    frame.data[2] = 2;
    ASSERT_TRUE(config->getMessage("test2sig2")->parseCANFrame(&frame, false));
    ASSERT_EQ(2, config->getSignal("test2sig2")->getValue().toInt());
    ASSERT_FALSE(config->getMessage("test2sig2")->parseCANFrame(&frame, false));

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 3;
    frame.data[0] = 0x80;
    frame.data[1] = 0x00;
    frame.data[2] = 0x80;
    frame.data[3] = 0x00;
    frame.data[4] = 0x00;
    frame.data[5] = 0x80;
    frame.data[6] = 0x00;
    frame.data[7] = 0x80;
    ASSERT_TRUE(config->getMessage("test3sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(-128, config->getSignal("test3sig1")->getValue().toInt());
    ASSERT_EQ(-32768, config->getSignal("test3sig2")->getValue().toInt());
    ASSERT_EQ(0, config->getSignal("test3sig3")->getValue().toInt());
    ASSERT_DOUBLE_EQ(-12.8, config->getSignal("test3sig4")->getValue().toDouble());
    ASSERT_DOUBLE_EQ(-3276.8, config->getSignal("test3sig5")->getValue().toDouble());
    ASSERT_FALSE(config->getMessage("test3sig1")->parseCANFrame(&frame, false));

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 12;
    frame.data[0] = 0x00;
    frame.data[1] = 0x00;
    frame.data[2] = 0x00;
    frame.data[3] = 0x80;
    frame.data[4] = 0x00;
    frame.data[5] = 0x00;
    frame.data[6] = 0x00;
    frame.data[7] = 0x80;
    ASSERT_TRUE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(-2147483648, config->getSignal("test12sig1")->getValue().toInt());
    ASSERT_DOUBLE_EQ(-214748364.8, config->getSignal("test12sig2")->getValue().toDouble());
    ASSERT_FALSE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 12;
    frame.data[0] = 0x01;
    frame.data[1] = 0x00;
    frame.data[2] = 0x00;
    frame.data[3] = 0x80;
    frame.data[4] = 0x01;
    frame.data[5] = 0x00;
    frame.data[6] = 0x00;
    frame.data[7] = 0x80;
    ASSERT_TRUE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(-2147483647, config->getSignal("test12sig1")->getValue().toInt());
    ASSERT_DOUBLE_EQ(-214748364.7, config->getSignal("test12sig2")->getValue().toDouble());
    ASSERT_FALSE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 12;
    frame.data[0] = 0xfe;
    frame.data[1] = 0xff;
    frame.data[2] = 0xff;
    frame.data[3] = 0x7f;
    frame.data[4] = 0xfe;
    frame.data[5] = 0xff;
    frame.data[6] = 0xff;
    frame.data[7] = 0x7f;
    ASSERT_TRUE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(2147483646, config->getSignal("test12sig1")->getValue().toInt());
    ASSERT_DOUBLE_EQ(214748364.6, config->getSignal("test12sig2")->getValue().toDouble());
    ASSERT_FALSE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));

    config->setDefaultValues();
    empty_canframe(frame);
    frame.can_id = 12;
    frame.data[0] = 0xff;
    frame.data[1] = 0xff;
    frame.data[2] = 0xff;
    frame.data[3] = 0x7f;
    frame.data[4] = 0xff;
    frame.data[5] = 0xff;
    frame.data[6] = 0xff;
    frame.data[7] = 0x7f;
    ASSERT_TRUE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));
    ASSERT_EQ(2147483647, config->getSignal("test12sig1")->getValue().toInt());
    ASSERT_DOUBLE_EQ(214748364.7, config->getSignal("test12sig2")->getValue().toDouble());
    ASSERT_FALSE(config->getMessage("test12sig1")->parseCANFrame(&frame, false));
}


TEST(LIB_canframe, test_parse_assemble) {
    srand(testing::UnitTest::GetInstance()->random_seed());
    Configuration *config = NULL;
    ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
    canfd_frame frame_in;

    empty_canframe(frame_in);
    frame_in.can_id = 1;
    frame_in.data[0] = rand() & 0xff;
    frame_in.data[1] = rand() & 0xff;
    frame_in.data[2] = rand() & 0xff;
    frame_in.data[3] = rand() & 0xff;
    ASSERT_TRUE(config->getMessage("test1sig1")->parseCANFrame(&frame_in, false));

    canfd_frame frame_out;
    empty_canframe(frame_out);
    config->getMessage("test1sig1")->assembleCANFrame(&frame_out);
    ASSERT_EQ(frame_out.can_id, frame_in.can_id);
    ASSERT_EQ(frame_out.data[0], frame_in.data[0]);
    ASSERT_EQ(frame_out.data[1], frame_in.data[1]);
    ASSERT_EQ(frame_out.data[2], frame_in.data[2]);
    ASSERT_EQ(frame_out.data[3], frame_in.data[3]);
    ASSERT_EQ(frame_out.data[4], frame_in.data[4]);
    ASSERT_EQ(frame_out.data[5], frame_in.data[5]);
    ASSERT_EQ(frame_out.data[6], frame_in.data[6]);
    ASSERT_EQ(frame_out.data[7], frame_in.data[7]);

    config->setDefaultValues();
    empty_canframe(frame_in);
    frame_in.can_id = 2;
    frame_in.data[0] = rand() & 0xff;
    frame_in.data[1] = rand() & 0xff;
    frame_in.data[2] = rand() & 0xff;
    frame_in.data[3] = rand() & 0xff;
    frame_in.data[4] = rand() & 0xff;
    frame_in.data[5] = rand() & 0xff;
    ASSERT_TRUE(config->getMessage("test2sig1")->parseCANFrame(&frame_in, false));
    empty_canframe(frame_out);
    config->getMessage("test2sig1")->assembleCANFrame(&frame_out);
    ASSERT_EQ(frame_out.can_id, frame_in.can_id);
    ASSERT_EQ(frame_out.data[0], frame_in.data[0]);
    ASSERT_EQ(frame_out.data[1], frame_in.data[1]);
    ASSERT_EQ(frame_out.data[2], frame_in.data[2]);
    ASSERT_EQ(frame_out.data[3], frame_in.data[3]);
    ASSERT_EQ(frame_out.data[4], frame_in.data[4]);
    ASSERT_EQ(frame_out.data[5], frame_in.data[5]);
    ASSERT_EQ(frame_out.data[6], frame_in.data[6]);
    ASSERT_EQ(frame_out.data[7], frame_in.data[7]);

    config->setDefaultValues();
    empty_canframe(frame_in);
    frame_in.can_id = 3;
    frame_in.data[0] = rand() & 0xff;
    frame_in.data[1] = rand() & 0xff;
    frame_in.data[2] = rand() & 0xff;
    frame_in.data[3] = rand() & 0xff;
    frame_in.data[4] = rand() & 0xff;
    frame_in.data[5] = rand() & 0xff;
    frame_in.data[6] = rand() & 0xff;
    frame_in.data[7] = rand() & 0xff;
    ASSERT_TRUE(config->getMessage("test3sig1")->parseCANFrame(&frame_in, false));
    empty_canframe(frame_out);
    config->getMessage("test3sig1")->assembleCANFrame(&frame_out);
    ASSERT_EQ(frame_out.can_id, frame_in.can_id);
    ASSERT_EQ(frame_out.data[0], frame_in.data[0]);
    ASSERT_EQ(frame_out.data[1], frame_in.data[1]);
    ASSERT_EQ(frame_out.data[2], frame_in.data[2]);
    ASSERT_EQ(frame_out.data[3], frame_in.data[3]);
    ASSERT_EQ(frame_out.data[4], frame_in.data[4]);
    ASSERT_EQ(frame_out.data[5], frame_in.data[5]);
    ASSERT_EQ(frame_out.data[6], frame_in.data[6]);
    ASSERT_EQ(frame_out.data[7], frame_in.data[7]);

    config->setDefaultValues();
    empty_canframe(frame_in);
    frame_in.can_id = 12;
    frame_in.data[0] = rand() & 0xff;
    frame_in.data[1] = rand() & 0xff;
    frame_in.data[2] = rand() & 0xff;
    frame_in.data[3] = rand() & 0xff;
    frame_in.data[4] = rand() & 0xff;
    frame_in.data[5] = rand() & 0xff;
    frame_in.data[6] = rand() & 0xff;
    frame_in.data[7] = rand() & 0xff;
    ASSERT_TRUE(config->getMessage("test12sig1")->parseCANFrame(&frame_in, false));
    empty_canframe(frame_out);
    config->getMessage("test12sig1")->assembleCANFrame(&frame_out);
    ASSERT_EQ(frame_out.can_id, frame_in.can_id);
    ASSERT_EQ(frame_out.data[0], frame_in.data[0]);
    ASSERT_EQ(frame_out.data[1], frame_in.data[1]);
    ASSERT_EQ(frame_out.data[2], frame_in.data[2]);
    ASSERT_EQ(frame_out.data[3], frame_in.data[3]);
    ASSERT_EQ(frame_out.data[4], frame_in.data[4]);
    ASSERT_EQ(frame_out.data[5], frame_in.data[5]);
    ASSERT_EQ(frame_out.data[6], frame_in.data[6]);
    ASSERT_EQ(frame_out.data[7], frame_in.data[7]);
}
