/*!
* \file
* \brief cantransceiver.h foo
*/

#ifndef CANTRANSCEIVER_H
#define CANTRANSCEIVER_H

#include "canmessage.h"
#include <exception>
#include <string>

extern "C" {
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
}

class CANTransceiverException : public std::exception
{
  public:

  virtual const char *what() const throw();
};

class CANTransceiver
{
public:
    CANTransceiver(const std::string &socketName, int bitrate);
    ~CANTransceiver();
    const int &getCANSocket() const;
    bool readCANFrame(canfd_frame *frame, bool *canfd);
    bool sendCANFrame(const canfd_frame *frame);
    bool sendCANMessage(CANMessage *message);
    int getCANBitrate();

private:
    bool m_canfd;
    bool m_vcan;
    int m_canSocket;
    std::string m_socketName;

    bool initCAN(int bitrate);
    bool isCANInterfaceUp() const;
    static bool userHasInterfacePermissions();
    void closeCAN();
};

#endif // CANTRANSCEIVER_H
