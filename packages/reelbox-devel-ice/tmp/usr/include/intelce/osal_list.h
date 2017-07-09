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

/*
 *  This file contains OS abstractions for common list functions
 */

#ifndef _OSAL_LIST_H
#define _OSAL_LIST_H

#include <os/osal_list.h>

typedef _os_list_head_t os_list_head_t;
#define OS_LIST_HEAD_INIT(name)  _OS_LIST_HEAD_INIT(name)
#define OS_INIT_LIST_HEAD(name)  _OS_INIT_LIST_HEAD(name)
#define OS_LIST_HEAD(a) _OS_LIST_HEAD(a)
#define OS_LIST_ADD_TAIL(a,b) _OS_LIST_ADD_TAIL(a,b)
#define OS_LIST_DEL(a) _OS_LIST_DEL(a)
#define OS_LIST_FOR_EACH(entry, head) _OS_LIST_FOR_EACH(entry, head)
#define OS_LIST_FOR_EACH_SAFE(entry, n, head) _OS_LIST_FOR_EACH_SAFE(entry, n, head)
#define OS_LIST_EMPTY(a) _OS_LIST_EMPTY(a)
#define OS_LIST_NEXT(a) _OS_LIST_NEXT(a)
#define OS_LIST_PREV(a) _OS_LIST_PREV(a)
#define OS_LIST_ENTRY(ptr,type,member) _OS_LIST_ENTRY(ptr,type,member)

#endif
