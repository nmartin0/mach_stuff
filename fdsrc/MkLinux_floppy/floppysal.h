/*
 * MkLinux Public Source License
 * v1.1
 *
 * Portions Copyright (c) 1999-2002 The MkLinux Project.
 *              All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both the copyright notice and this permission notice and disclaimer
 * appear in all copies of the software, derivative works or modified
 * versions, and any portions thereof, and that these notices also
 * appear in supporting documentation, and provided that all advertising
 * materials mentioning features or use of this software displays the
 * following acknowledgement:
 *
 *      This product includes software developed by The MkLinux Project
 *      and its contributors.
 *
 * Neither the name of The MkLinux Project nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND THE MKLINUX PROJECT HEREBY DISCLAIMS ALL
 * SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT."
 * 
 * IN NO EVENT SHALL THE MKLINUX PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN ACTION OF CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF OR IN CONNECTION WITH THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Software whose source code is freely available, including those licensed
 * under the GNU General Public License (GPL) are exempt from all advertising
 * requirements.  Software collections are not exempt unless they include
 * non MPSL versions of this code as allowed below.
 *
 * License is hereby granted to freely redistribute this code as part of
 * another software product under the terms of the GPL or any other OSI-
 * certified open source license agreement provided that this copyright notice
 * remain intact, and provided that all copies of the source code contain, in
 * some reasonable, human-readable form, information telling where any party
 * may obtain a copy of this source code that is not burdened by the GPL or
 * other license's potentially more restrictive licensing terms.
 */
/*
 * Copyright 1991-1998 by Open Software Foundation, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * OSF DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL OSF BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * Copyright 1991-1998 by Apple Computer, Inc. 
 *              All Rights Reserved 
 *  
 * Permission to use, copy, modify, and distribute this software and 
 * its documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appears in all copies and 
 * that both the copyright notice and this permission notice appear in 
 * supporting documentation. 
 *  
 * APPLE COMPUTER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE. 
 *  
 * IN NO EVENT SHALL APPLE COMPUTER BE LIABLE FOR ANY SPECIAL, INDIRECT, OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN ACTION OF CONTRACT, 
 * NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */
/*
 * MkLinux
 */


#ifndef __FLOPPYSAL_H__
#define __FLOPPYSAL_H__


//#include <Types.h>
//#include <Disks.h>
//#include <NameRegistry.h>
#include "portdef.h"

/************************************************************************
*
* Function prototypes for routines in FloppySAL.c
*
************************************************************************/

OSStatus
RecordError(OSStatus errorCode);

OSStatus
GetMemory(long RequestSize,
	  Ptr * memoryPtr);

OSStatus
MapCacheForDMAIO(LogicalAddress logicalBaseAddr,
		 uint_t length,
		 LogicalAddress * logBase,
		 PhysicalAddress * phyBase);

OSStatus
PrepareCPUCacheForDMARead(void);

OSStatus
PrepareCPUCacheForDMAWrite(void);

OSStatus
FlushDMAedDataFromCPUCache(void);

OSStatus
FloppyTimedSleep(long Milliseconds);

#ifdef OLD_CODE
OSStatus CreateOSHardwareLockResources(spl_t * OSLockID);

OSStatus
DeleteOSHardwareLockResources(void *OSLockID);

spl_t
EnterHardwareLockSection(void);

void
 ExitHardwareLockSection(spl_t);
#else
OSStatus CreateOSHardwareLockResources(mutex_t **OSLockID);

OSStatus DeleteOSHardwareLockResources(mutex_t *OSLockID);

void EnterHardwareLockSection(mutex_t *mutex);
void ExitHardwareLockSection(mutex_t *mutex);
#endif

OSStatus
CreateOSEventResources(UInt32 * OSEventID);

OSStatus
DeleteOSEventResources(void *OSEventID);

OSStatus
SetOSEvent(UInt32 * OSEventID,
	   uint_t eventMask);

OSStatus
CancelOSEvent(UInt32 * OSEventID,
	      uint_t eventMask);

boolean_t
WaitForOSEvent(UInt32 * OSEventID,
	       uint_t eventMask,
	       uint_t timeOut,
	       uint_t * eventMaskResult);

OSStatus
PostDisketteEvent(int unit, unsigned char disketteEvent,
		  short driveIndex);

OSStatus
LaunchMediaScanTask(void);

OSStatus
KillMediaScanTask(void);

OSStatus
RegisterFloppyISR(RegEntryID * deviceEntryID,
		  void *ISRHandlerPtr);

#endif				// if already included.
