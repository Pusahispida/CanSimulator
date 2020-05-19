/*!
* \file
* \brief test_LIB_errorframe.cpp foo
*/

#include "dummy_logger.h"

#include "../lib/ascreader.cpp"
#include "../lib/canerror.cpp"
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
#include <linux/can/error.h>
#include <gtest/gtest.h>

void empty_canframe(canfd_frame &frame) {
    memset(&frame, 0, sizeof(struct canfd_frame));
}

TEST(LIB_errorframe, test_errorframe) {
    canfd_frame frame;
    std::string error;

    empty_canframe(frame);
    frame.can_id = 0x20000001U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x1\nTX timeout\n");

    empty_canframe(frame);
    frame.can_id = 0x20000002U;
    frame.data[0] = 0x01;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x2\nLost arbitration at bit \x1\n");

    empty_canframe(frame);
    frame.can_id = 0x20000004U;
    frame.data[1] = 0x40;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x4\nController error: Recovered to error active state\n");

    empty_canframe(frame);
    frame.can_id = 0x20000008U;
    frame.data[2] = 0x80;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x8\nProtocol violation: Error occured on transmission\n");

    empty_canframe(frame);
    frame.can_id = 0x20000008U;
    frame.data[2] = 0x01;
    frame.data[3] = 0x1A;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x8\nProtocol violation: Single bit error at end of frame\n");

    empty_canframe(frame);
    frame.can_id = 0x20000010U;
    frame.data[4] = 0x07;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x10\nTransceiver error: CAN_H short to GND\n");

    empty_canframe(frame);
    frame.can_id = 0x20000020U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x20\nReceived no ACK on transmission\n");

    empty_canframe(frame);
    frame.can_id = 0x20000040U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x40\nBus off\n");

    empty_canframe(frame);
    frame.can_id = 0x20000080U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x80\nBus error\n");

    empty_canframe(frame);
    frame.can_id = 0x20000100U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x100\nController restarted\n");

    empty_canframe(frame);
    frame.can_id = 0x20000021U;
    error = analyzeErrorFrame(&frame);
    ASSERT_EQ(error, "errorframe=0x21\nTX timeout\nReceived no ACK on transmission\n");
}
