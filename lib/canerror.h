/*!
* \file
* \brief canerror.h foo
*/

#ifndef CANERROR_H
#define CANERROR_H

#include <string>

extern "C" {
#include <linux/can.h>
#include <linux/can/error.h>
}

std::string analyzeErrorFrame(canfd_frame *frame);

#endif // CANERROR_H
