//************************************************************************************************************
//
// © 2016-2019 Regents of the University of California on behalf of the University of California at Berkeley
//       with rights granted for USDOT OSADP distribution with the ECL-2.0 open source license.
//
//*************************************************************************************************************
#ifndef _ASN_J2735_LIB_H
#define _ASN_J2735_LIB_H

#include <cstddef>
#include <cstdint>
#include <fstream>

#include "dsrcFrame.h"

namespace AsnJ2735Lib
{ /// UPER encoding functions
	size_t encode_msgFrame(const Frame_element_t& dsrcFrameIn, uint8_t* buf, size_t size);
	/// UPER decoding functions
	size_t decode_msgFrame(const uint8_t* buf, size_t size, Frame_element_t& dsrcFrameOut);
};

#endif
