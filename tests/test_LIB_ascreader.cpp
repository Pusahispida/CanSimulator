/*!
* \file
* \brief test_LIB_ascreader.cpp foo
*/

#include "dummy_logger.h"

#include "../lib/ascreader.cpp"
#include "../lib/stringtools.cpp"
#include <linux/can.h>
#include <gtest/gtest.h>

TEST(LIB_ascreader, missing_file) {
    ASSERT_THROW(ASCReader *reader = new ASCReader(""), ASCReaderException);
}

TEST(LIB_ascreader, ascreader) {
    ASCReader *reader = NULL;
    ASSERT_NO_THROW(reader = new ASCReader("tests.asc"));
    std::map<std::uint64_t, canFrameQueueItem> queue = reader->getFrameQueue();
    ASSERT_EQ(3, queue.size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue.begin();
    ASSERT_EQ(2501, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2502, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2503, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x800000a8, it->second.frame.can_id);
    ASSERT_EQ(0xA, it->second.frame.data[0]);
    ASSERT_EQ(9, it->second.frame.data[1]);
    ASSERT_EQ(8, it->second.frame.data[2]);
    ASSERT_EQ(7, it->second.frame.data[3]);
    ASSERT_EQ(6, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(4, it->second.frame.data[6]);
    ASSERT_EQ(3, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(queue.end(), it);

    delete reader;
}

TEST(LIB_ascreader, ascreader_continuous) {
    ASCReader *reader = NULL;
    ASSERT_NO_THROW(reader = new ASCReader("tests_second.asc"));
    std::map<std::uint64_t, canFrameQueueItem> queue = reader->getFrameQueue();
    ASSERT_EQ(6, queue.size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue.begin();
    ASSERT_EQ(2501, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2502, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2503, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x800000a8, it->second.frame.can_id);
    ASSERT_EQ(0xA, it->second.frame.data[0]);
    ASSERT_EQ(9, it->second.frame.data[1]);
    ASSERT_EQ(8, it->second.frame.data[2]);
    ASSERT_EQ(7, it->second.frame.data[3]);
    ASSERT_EQ(6, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(4, it->second.frame.data[6]);
    ASSERT_EQ(3, it->second.frame.data[7]);
    ++it;
    // Second file content
    ASSERT_EQ(2701, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2801, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(2901, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x800000a8, it->second.frame.can_id);
    ASSERT_EQ(0xA, it->second.frame.data[0]);
    ASSERT_EQ(9, it->second.frame.data[1]);
    ASSERT_EQ(8, it->second.frame.data[2]);
    ASSERT_EQ(7, it->second.frame.data[3]);
    ASSERT_EQ(6, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(4, it->second.frame.data[6]);
    ASSERT_EQ(3, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(queue.end(), it);

    delete reader;
}


TEST(LIB_ascreader, ascreader_relative) {
    ASCReader *reader = NULL;
    ASSERT_NO_THROW(reader = new ASCReader("tests_relative.asc"));
    std::map<std::uint64_t, canFrameQueueItem> queue = reader->getFrameQueue();
    ASSERT_EQ(2, queue.size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue.begin();
    ASSERT_EQ(2501, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(5003, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(queue.end(), it);

    delete reader;
}

TEST(LIB_ascreader, ascreader_relative_continuous) {
    ASCReader *reader = NULL;
    ASSERT_NO_THROW(reader = new ASCReader("tests_relative_second.asc"));
    std::map<std::uint64_t, canFrameQueueItem> queue = reader->getFrameQueue();
    ASSERT_EQ(4, queue.size());
    std::map<std::uint64_t, canFrameQueueItem>::iterator it = queue.begin();
    ASSERT_EQ(2501, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(5003, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    // Second file content
    ASSERT_EQ(7704, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x128, it->second.frame.can_id);
    ASSERT_EQ(0, it->second.frame.data[0]);
    ASSERT_EQ(1, it->second.frame.data[1]);
    ASSERT_EQ(2, it->second.frame.data[2]);
    ASSERT_EQ(3, it->second.frame.data[3]);
    ASSERT_EQ(4, it->second.frame.data[4]);
    ASSERT_EQ(5, it->second.frame.data[5]);
    ASSERT_EQ(6, it->second.frame.data[6]);
    ASSERT_EQ(7, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(10406, it->second.timestamp);
    ASSERT_EQ(true, it->second.in);
    ASSERT_EQ(0x129, it->second.frame.can_id);
    ASSERT_EQ(0x10, it->second.frame.data[0]);
    ASSERT_EQ(0x20, it->second.frame.data[1]);
    ASSERT_EQ(0, it->second.frame.data[2]);
    ASSERT_EQ(0, it->second.frame.data[3]);
    ASSERT_EQ(0, it->second.frame.data[4]);
    ASSERT_EQ(0, it->second.frame.data[5]);
    ASSERT_EQ(0, it->second.frame.data[6]);
    ASSERT_EQ(0, it->second.frame.data[7]);
    ++it;
    ASSERT_EQ(queue.end(), it);

    delete reader;
}

TEST(LIB_ascreader, missing_previous_file) {
    // Test missing previous log
    ASSERT_THROW(ASCReader *reader = new ASCReader("tests_missing.asc"), ASCReaderException);
}
