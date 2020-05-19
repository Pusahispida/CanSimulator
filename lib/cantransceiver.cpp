/*!
* \file
* \brief cantransceiver.cpp foo
*/

#include "cantransceiver.h"
#include "logger.h"
#include <cstring>
#include <iostream>
#include <libsocketcan.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <unistd.h>

const char *CANTransceiverException::what() const throw()
{
    return "CANTransceiverException";
}

/*!
 * \brief CANTransceiver::CANTransceiver
 * Constructor.
 * \param socketName: CAN interface name
 * \param bitrate: CAN bus bitrate to set
 */
CANTransceiver::CANTransceiver(const std::string &socketName, int bitrate) :
    m_socketName(socketName)
{
    if (!m_socketName.compare(0, 4, "vcan")) {
        m_vcan = true;
    } else {
        m_vcan = false;
    }
    if (!initCAN(bitrate)) {
        throw CANTransceiverException();
    }
}

/*!
 * \brief CANTransceiver::~CANTransceiver
 * Destructor.
 */
CANTransceiver::~CANTransceiver()
{
    closeCAN();
}

/*!
 * \brief CANTransceiver::isCANInterfaceUp
 * Check if CAN interface is up
 * \return True if interface is up, otherwise false
 */
bool CANTransceiver::isCANInterfaceUp() const
{
    if (m_canSocket < 0) {
        return false;
    }
    struct ifreq ifr;
    strcpy(ifr.ifr_name, m_socketName.c_str());
    if (ioctl(m_canSocket, SIOCGIFFLAGS, &ifr) < 0) {
        LOG(LOG_ERR, "error=2 Failed getting CAN interface flags\n");
        return false;
    }
    return (ifr.ifr_flags & IFF_UP) == IFF_UP;
}

/*!
 * \brief CANTransceiver::userHasInterfacePermissions
 * Check if user has permissions to control CAN interface
 * \return True if user had needed permissions, otherwise false
 */
bool CANTransceiver::userHasInterfacePermissions()
{
    cap_t caps;
    caps = cap_get_proc();
    bool ret = true;

    if (caps == NULL) {
        LOG(LOG_WARN, "warning=2 Unable read capabilities\n");
        ret = false;
    } else {
        int i;
        cap_flag_value_t val;
        cap_value_t capability[2] = { CAP_NET_ADMIN, CAP_NET_RAW };
        for (i=0; i < 2; i++) {
            cap_get_flag(caps, capability[i ], CAP_PERMITTED, &val);
            if (val != CAP_SET) {
                ret = false;
            }
        }
    }
    return ret;
}

/*!
 * \brief CANTransceiver::initCAN
 * Initialize CAN socket.
 * \param bitrate: CAN bus bitrate to set
 * \return True on success, otherwise false
 */
bool CANTransceiver::initCAN(int bitrate)
{
    bool interfacePermissions = userHasInterfacePermissions();
    struct sockaddr_can addr;
    struct ifreq ifr;

    memset(&addr, 0, sizeof(addr));

    if ((m_canSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        LOG(LOG_ERR, "error=2 Unable to open CAN socket\n");
        return false;
    }

    strcpy(ifr.ifr_name, m_socketName.c_str());

    if (ioctl(m_canSocket, SIOCGIFINDEX, &ifr) < 0) {
        LOG(LOG_ERR, "error=2 Failed setting CAN interface name to %s\n", m_socketName.c_str());
        return false;
    }

    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // If user has permission try to initialize CAN interface
    if (interfacePermissions) {
        if (!m_vcan && bitrate > 0 && !(isCANInterfaceUp() && getCANBitrate() == bitrate)) {
            if (isCANInterfaceUp() && can_do_stop(m_socketName.c_str()) < 0) {
                LOG(LOG_WARN, "warning=2 Unable to stop CAN interface\n");
            }
            if (can_set_bitrate(m_socketName.c_str(), bitrate) < 0) {
                LOG(LOG_WARN, "warning=2 Unable to set CAN bitrate\n");
            }
        }
        if (!isCANInterfaceUp()) {
            if (can_do_start(m_socketName.c_str()) < 0) {
                LOG(LOG_WARN, "warning=2 Unable to start CAN interface\n");
            }
        }
    }

    // Fail if CAN interface is down or the CAN bitrate (if set to non-zero value) is wrong
    // Ignore bitrate in vcan
    if ((!isCANInterfaceUp() || (!m_vcan && bitrate > 0 && getCANBitrate() != bitrate)) && !interfacePermissions) {
        LOG(LOG_ERR, "error=2 No permissions to setup CAN interface\n");
        return false;
    }

    int enable_sockopt = 1;
    if (setsockopt(m_canSocket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_sockopt, sizeof(enable_sockopt)) == 0) {
        m_canfd = true;
    } else {
        m_canfd = false;
    }

    can_err_mask_t err_mask = CAN_ERR_MASK;
    if (setsockopt(m_canSocket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask)) != 0) {
        LOG(LOG_ERR, "warning=2 Unable to enable error filter\n");
    }

    if (bind(m_canSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        LOG(LOG_ERR, "error=2 Unable to bind CAN socket\n");
        return false;
    }

    return true;
}

/*!
 * \brief CANTransceiver::closeCAN
 * Close CAN socket.
 */
void CANTransceiver::closeCAN()
{
    if (m_canSocket >= 0) {
        close(m_canSocket);
    }
}

/*!
 * \brief CANTransceiver::getCANSocket
 * Get the CAN socket, only to be used for polling the socket for new data
 * \return Reference to CAN socket descriptor
 */
const int &CANTransceiver::getCANSocket() const
{
    return m_canSocket;
}

/*!
 * \brief CANTransceiver::readCANFrame
 * Read a CAN (FD) frame
 * \param frame: Pointer to the CAN (FD) frame to be filled
 * \param canfd: Pointer to boolean for returning received frame type: true if CAN FD frame, false if CAN frame
 * \return True on success, false otherwise
 */
bool CANTransceiver::readCANFrame(canfd_frame *frame, bool *canfd)
{
    int nbytes;
    if (m_canSocket < 0) {
        LOG(LOG_ERR, "error=2 CAN socket not ready\n");
        return false;
    }
    if ((nbytes = read(m_canSocket, frame, CANFD_MTU)) >= 0) {
        if (nbytes == CANFD_MTU) {
            *canfd = true;
        } else if (nbytes == CAN_MTU) {
            *canfd = false;
        } else {
            LOG(LOG_WARN, "warning=2 Incomplete CAN frame received\n");
            return false;
        }
        return true;
    }
    return false;
}

/*!
 * \brief CANTransceiver::sendCANFrame
 * Send a CAN (FD) frame
 * \param frame: Pointer to the CAN (FD) frame to be sent
 * \return True on success, false otherwise
 */
bool CANTransceiver::sendCANFrame(const canfd_frame *frame)
{
    if (m_canSocket < 0) {
        LOG(LOG_ERR, "error=2 CAN socket not ready\n");
        return false;
    }

    int retval = write(m_canSocket, frame, m_canfd ? CANFD_MTU : CAN_MTU);
    return retval >= 0;
}

/*!
 * \brief CANTransceiver::sendCANMessage
 * Send a CAN message
 * \param frame: Pointer to the CANMessage to be sent
 * \return True on success, false otherwise
 */
bool CANTransceiver::sendCANMessage(CANMessage *message)
{
    canfd_frame frame;
    message->assembleCANFrame(&frame);
    if (!sendCANFrame(&frame)) {
        message->updateTransfer(false);
        return false;
    }
    message->updateTransfer(true);
    message->setModified(false);
    return true;
}

/*!
 * \brief CANTransceiver::getCANBitrate
 * Read CAN bitrate from CAN bus
 * \return CAN bitrate, or 0 in case of error
 */
int CANTransceiver::getCANBitrate()
{
    struct can_bittiming bt;
    // VCAN doesn't have a bitrate
    if (m_vcan) {
        return 0;
    }
    if (can_get_bittiming(m_socketName.c_str(), &bt) < 0) {
        LOG(LOG_ERR, "warning=2 unable to read CAN bitrate\n");
        return 0;
    } else {
        return bt.bitrate;
    }
}
