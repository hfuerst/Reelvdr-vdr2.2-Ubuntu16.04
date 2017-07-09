/*==========================================================================
  This file is provided under a BSD license.  When using or
  redistributing this file, you may do so under this license.

  BSD LICENSE 

  Copyright(c) 2005-2011 Intel Corporation. All rights reserved.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

    * Redistributions of source code must retain the above copyright 
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright 
      notice, this list of conditions and the following disclaimer in 
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Intel Corporation nor the names of its 
      contributors may be used to endorse or promote products derived 
      from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 =========================================================================*/
#ifndef _OSAL_CLOCK_H
#define _OSAL_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "os/osal_clock.h"

/**
Gets the the current "ticks" of the system clock.  The tick is a monotomic clock
running at a system-specific rate.  The rate is guarenteed to be faster than
1 tick per second

@param[out] ticks : tick count of the system clock

@retval OSAL_SUCCESS : function returned successfully
@retval OSAL_ERROR : internal OSAL error
*/
osal_result os_clock_get_ticks( unsigned long long *ticks );

/**
Returns the freqency the system clock runs at.  the frequency is the number of
ticks per second.

@param[out] freq : ticks per second of the system clock

@retval OSAL_SUCCESS : function returned successfully
@retval OSAL_ERROR : internal OSAL error
*/
osal_result os_clock_get_tick_freq( unsigned long long *freq );

/**
Returns the current time offset. This is an opaque value
used to pass into os_clock_get_time_diff_* functions

@param[out] time : returned time offset

@retval OSAL_SUCCESS : function returned successfully
@retval OSAL_ERROR : internal OSAL error
*/
osal_result os_clock_get_time(os_time_t *time);

/**
Returns the difference in seconds between the current system time and
and the time parameter passed in.

@param[in] time : opaque timestamp from os_clock_get_time()
@param[out] secs : time difference in seconds

@retval OSAL_SUCCESS : function returned successfully
@retval OSAL_ERROR : internal OSAL error
*/
osal_result os_clock_get_time_diff_secs(os_time_t *time, \
                                        unsigned long *secs);

/**
Returns the difference in milliseconds between the current time and
and the time parameter passed in.

@param[in] time : opaque timestamp from os_clock_get_time()
@param[out] msecs : time difference in milliseconds

@retval OSAL_SUCCESS : function returned successfully
@retval OSAL_ERROR : internal OSAL error
*/
osal_result os_clock_get_time_diff_msecs(os_time_t *time, unsigned long *msecs );

#ifdef __cplusplus
}
#endif
#endif
