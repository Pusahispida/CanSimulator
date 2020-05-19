/*!
* \file
* \brief test_LIB_queue.cpp foo
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
#include "../lib/queue.h"
#include "../lib/stringtools.cpp"
#include "../lib/unitconversion.cpp"
#include "../lib/value.cpp"
#include <linux/can.h>
#include <linux/can/error.h>
#include <gtest/gtest.h>

bool wait_for_input(fd_set &input_set, int nfds) {
  // Wait 10 milliseconds for input
  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  return select(nfds, &input_set, NULL, NULL, &timeout) > 0;
}

TEST(LIB_queue, test_queue) {
  Queue<std::shared_ptr<CANMessage>> messageQueue;
  fd_set input_set;
  int nfds = 0;
  int messageQueueFd = messageQueue.getEventFd();
  FD_ZERO(&input_set);
  if (messageQueueFd >= 0) {
    FD_SET(messageQueueFd, &input_set);
    nfds = ((messageQueueFd+1) > nfds) ? messageQueueFd+1 : nfds;
  }
  ASSERT_FALSE(wait_for_input(input_set, nfds));
  Configuration *config = NULL;
  ASSERT_NO_THROW(config = new Configuration("tests.cfg", "tests.dbc"));
  ASSERT_EQ("TEST_1_SIG_1", config->getSignal("test1sig1")->getName());
  ASSERT_TRUE(config->setValue("test1sig1", "1"));
  ASSERT_TRUE(config->setValue("test2sig1", "2"));
  CANMessage *msg1 = config->getMessage("test1sig1");
  CANMessage *msg2 = config->getMessage("test2sig1");
  // Add messages to queue
  CANMessage *newMsg1 = new CANMessage(*msg1);
  CANMessage *newMsg2 = new CANMessage(*msg2);
  std::shared_ptr<CANMessage> msgCopy1(newMsg1);
  std::shared_ptr<CANMessage> msgCopy2(newMsg2);
  messageQueue.push(msgCopy1);
  messageQueue.push(msgCopy2);
  // Check queue for added messages
  FD_ZERO(&input_set);
  if (messageQueueFd >= 0) {
    FD_SET(messageQueueFd, &input_set);
    nfds = ((messageQueueFd+1) > nfds) ? messageQueueFd+1 : nfds;
  }
  ASSERT_TRUE(wait_for_input(input_set, nfds));
  // Check set signal values
  std::shared_ptr<CANMessage> out_message1 = messageQueue.pop();
  ASSERT_EQ(1, out_message1.get()->getId());
  ASSERT_EQ(1, out_message1.get()->getSignal("TEST_1_SIG_1")->getValue().toInt());
  std::shared_ptr<CANMessage> out_message2 = messageQueue.pop();
  ASSERT_EQ(2, out_message2.get()->getId());
  ASSERT_EQ(2, out_message2.get()->getSignal("TEST_2_SIG_1")->getValue().toInt());
}
