/*!
* \file
* \brief ascreader.h foo
*/

#ifndef ASCREADER_H
#define ASCREADER_H

#include <exception>
#include <map>
#include <string>

extern "C" {
#include <linux/can.h>
}

class ASCReaderException : public std::exception
{
public:
    virtual const char *what() const throw();
};

struct canFrameQueueItem {
    uint64_t timestamp;
    bool in;
    canfd_frame frame;
};

class ASCReader
{
public:
    explicit ASCReader(const std::string &fileName);
    std::map<std::uint64_t, canFrameQueueItem> &getFrameQueue();
    bool createFilterList(std::map<std::uint32_t, bool> &list);

private:
    bool m_absoluteTimestamps;
    std::string m_fileName;
    std::map<std::uint64_t, canFrameQueueItem> m_frameQueue;
    bool m_hexId;
    float m_oldTimestamp;

    bool parseASC(const std::string &fileName);
    bool parseContinuousLogHeader(std::ifstream &file);
    bool parseHeader(std::ifstream &file);
    bool parseMessage(std::string &line);
};

#endif // CANTRANSCEIVER_H
