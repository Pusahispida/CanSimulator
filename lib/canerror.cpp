/*!
* \file
* \brief canerror.cpp foo
*/

#include "canerror.h"
#include "logger.h"
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

// Error classes
static const std::vector<std::string> errorClasses = {
    "TX timeout",
    "Lost arbitration",
    "Controller error:",
    "Protocol violation:",
    "Transceiver error:",
    "Received no ACK on transmission",
    "Bus off",
    "Bus error",
    "Controller restarted",
};

// Controller error strings
static const std::vector<std::string> controllerErrors = {
    " RX buffer overflow",
    " TX buffer overflow",
    " Reached warning level for RX errors",
    " Reached warning level for TX errors",
    " Reached error passive status RX",
    " Reached error passive status TX",
    " Recovered to error active state",
};

// Protocol error type strings
static const std::vector<std::string> protocolErrorTypes = {
    " Single bit error",
    " Frame format error",
    " Bit stuffing error",
    " Unable to send dominant bit",
    " Unable to send recessive bit",
    " Bus overload",
    " Active error announcement",
    " Error occured on transmission",
};

// Protocol error position strings
static const std::vector<std::string> protocolErrorPositions = {
    "unspecified",
    "unspecified",
    "ID bits 28-21 (SFF: 10-3)",
    "start of frame",
    "substitute RTR (SFF: RTR)",
    "identifier extension",
    "ID bits 20-18 (SFF: 2-0)",
    "ID bits 17-13",
    "CRC sequence",
    "reserved bit 0",
    "data section",
    "data length code",
    "RTR",
    "reserved bit 1",
    "ID bits 4-0",
    "ID bits 12-5",
    "unspecified",
    "unspecified",
    "intermission",
    "unspecified",
    "unspecified",
    "unspecified",
    "unspecified",
    "unspecified",
    "CRC delimiter",
    "ACK slot",
    "end of frame",
    "ACK delimiter",
    "unspecified",
    "unspecified",
    "unspecified",
    "unspecified",
};

/*!
 * \brief handleError
 * Add error messages to error stringstream based in error flags.
 * \param error: Reference to stringstream to add the error messages.
 * \param errorFlags: Error flags from CAN frame data.
 * \param errors: Pointer to vector of error message strings.
 */
void handleError(std::stringstream &error, uint32_t errorFlags, const std::vector<std::string> &errors)
{
    unsigned int i;

    if (!errorFlags || errors.size() <= 0) {
        return;
    }

    for (i = 0; i < errors.size(); i++) {
        if (errorFlags & (1 << i)) {
            error << errors[i];
        }
    }
}

/*!
 * \brief handleTransceiverError
 * Add transceiver error message to error stringstream based in error flags.
 * \param error: Reference to stringstream to add the error message.
 * \param errorFlags: Error flags from CAN frame data.
 */
void handleTransceiverError(std::stringstream &error, unsigned char errorFlags)
{
    switch (errorFlags & 0x7) {
        case CAN_ERR_TRX_CANH_NO_WIRE:
            error << " CAN_H No wire";
            break;
        case CAN_ERR_TRX_CANH_SHORT_TO_BAT:
            error << " CAN_H short to BAT";
            break;
        case CAN_ERR_TRX_CANH_SHORT_TO_VCC:
            error << " CAN_H short to VCC";
            break;
        case CAN_ERR_TRX_CANH_SHORT_TO_GND:
            error << " CAN_H short to GND";
            break;
        default:
            break;
    }
    switch (errorFlags & 0x70) {
        case CAN_ERR_TRX_CANL_NO_WIRE:
            error << " CAN_L no wire";
            break;
        case CAN_ERR_TRX_CANL_SHORT_TO_BAT:
            error << " CAN_L short to BAT";
            break;
        case CAN_ERR_TRX_CANL_SHORT_TO_VCC:
            error << " CAN_L short to VCC";
            break;
        case CAN_ERR_TRX_CANL_SHORT_TO_GND:
            error << " CAN_L short to GND";
            break;
        default:
            break;
    }
    if ((errorFlags & 0x80) == CAN_ERR_TRX_CANL_SHORT_TO_CANH) {
        error << " CAN_L short to CAN_H";
    }
}

/*!
 * \brief analyzeErrorFrame
 * Analyze error frame and add error messages to output string
 * \param frame: Pointer to the CAN error frame
 * \return Error message string
 */
std::string analyzeErrorFrame(canfd_frame *frame)
{
    unsigned int i;
    canid_t errorClass = frame->can_id & CAN_ERR_MASK;
    std::stringstream error;
    error << "errorframe=0x" << std::hex << errorClass << "\n";
    for (i = 0; i < errorClasses.size(); i++) {
        canid_t mask = 1 << i;
        if (errorClass & mask) {
            error << errorClasses[i];
            if (mask == CAN_ERR_LOSTARB) {
                // Arbitration loss position
                if (frame->data[0] != CAN_ERR_LOSTARB_UNSPEC) {
                    error << " at bit " << std::dec << frame->data[0];
                }
            } else if (mask == CAN_ERR_CRTL) {
                // Controller error details
                handleError(error, frame->data[1], controllerErrors);
            } else if (mask == CAN_ERR_PROT) {
                // Protocol error details
                handleError(error, frame->data[2], protocolErrorTypes);
                // Protocol error position
                if (frame->data[2] && frame->data[3] && frame->data[3] < protocolErrorPositions.size()) {
                    error << " at " << protocolErrorPositions[frame->data[3]];
                }
            } else if (mask == CAN_ERR_TRX) {
                // Transceiver error details
                handleTransceiverError(error, frame->data[4]);
            }
            error << "\n";
        }
    }
    return error.str();
}
