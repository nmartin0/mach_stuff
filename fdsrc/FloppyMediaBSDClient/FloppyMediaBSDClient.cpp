/*
 * MkLinux Public Source License
 * v1.1
 *
 * Portions Copyright (c) 2002 The MkLinux Project.
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


#include <sys/errno.h>
#include <IOKit/IOLib.h>
#include <IOKit/storage/IOStorage.h>
#include <IOKit/storage/IOBlockStorageDriver.h>
#include "FloppyMediaBSDClient.h"

#define super IOMediaBSDClient
OSDefineMetaClassAndStructors(FloppyMediaBSDClient, IOMediaBSDClient)

#define dIOLog IOLog

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool FloppyMediaBSDClient::start(IOService *provider)
{
    IOMedia * media = (IOMedia *) provider;
    u_int64_t size;
    IOStorage *storage;

    if (super::start(provider) == false) return false;

    media = getProvider();
    storage = media->getProvider();
    this->driver = OSDynamicCast(IOBlockStorageDriver, storage);

    if (!this->driver) return false;

    size = media->getSize() / (u_int64_t) 512;

    // Assume unformatted media is ours (since CDs and DVDs probe
    // higher, and anything else unformatted is... broken)
    switch (size) {
	case 0: // unformatted
	case 720: // 360k
	case 800: // 400k
	case 1440: // 720k
	case 1600: // 800k
	case 2880: // 1440k
	case 2881: // 1440k BUG BUG BUG... why do we need this?
	case 3360: // 1680k
	case 5760: // 2880k
	    dIOLog("Floppy Disk Media Detected.\n");
	    break;
	default:
	    dIOLog("Non-floppy disk media detected: %ld\n", (unsigned long)size);
	    return false;
    }
    return true;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int FloppyMediaBSDClient::ioctl( dev_t         dev,
                                u_long        cmd,
                                caddr_t       data,
                                int           flags,
                                struct proc * proc )
{
    //
    // Process a Floppy-specific ioctl.
    //

    // IOMemoryDescriptor * buffer = 0;
    int *buffer;
    int                  error  = 0;
    IOReturn             status = kIOReturnSuccess;
    int formatflags;

    switch(cmd) {
	case FD_VERIFY:
	    buffer = NULL;
	    break;
	default:
	    buffer = (int *)data;
	    if (!buffer) {
		dIOLog("ioctl (floppy): null buffer!\n");
		error=EINVAL;
		break;
	    }
    }

    switch (cmd)
    {
	case FD_FORMAT:
	{
	    UInt64 cap=0;

	    dIOLog("Got FD_FORMAT!\n");

	    formatflags = *buffer;

	    cap = 0;
	    switch(formatflags) {
		case FD_FORMAT_360K:
		    dIOLog("Format 360k\n");
		    cap = (360 * 1024);
		    break;
		case FD_FORMAT_400K:
		    dIOLog("Format 400k\n");
		    cap = (400 * 1024);
		    break;
		case FD_FORMAT_720K:
		    dIOLog("Format 720k\n");
		    cap = (720 * 1024);
		    break;
		case FD_FORMAT_800K:
		    dIOLog("Format 800k\n");
		    cap = (800 * 1024);
		    break;
		case FD_FORMAT_1440K:
		    dIOLog("Format 1440k\n");
		    cap = (1440 * 1024);
		    break;
		case FD_FORMAT_1680K:
		    dIOLog("Format 1680k\n");
		    cap = (1680 * 1024);
		    break;
		case FD_FORMAT_2880K:
		    dIOLog("Format 2880k\n");
		    cap = (2880 * 1024);
		    break;
		default:
		    dIOLog("Bogus Format\n");
		    error=EINVAL;
		    break;
	    }

	    /* DAG Truncation of capacity to 32 bits here, but not an issue
	       for floppies, since their capacity is small. */
	    IOLog("ioctl(FDFORMAT) called with flags %d, capacity %ld\n",
		formatflags, (unsigned long)cap);

	    if (!cap) break;

	    status = this->driver->formatMedia(cap);

	    if (status == kIOReturnSuccess) error=0;
	    else error=EINVAL;

	    break;
	}
	case FD_VERIFY:
	{
	    IOLog("Got FD_VERIFY -- not supported yet.\n");
	    error = ENOTTY;
	    break;
	}
	case FD_GETFORMATS:
	{
	    UInt64 capbuf[3];
	    int ncaps, i, blocks;

	    dIOLog("Got FD_GETFORMATS!\n");

	    ncaps=this->driver->getFormatCapacities(capbuf, (UInt32)3);

	    formatflags=0;
	    for (i=0; i<ncaps; i++) {
	        /* DAG Truncation of capacity to 32 bits here, but not an issue
	           for floppies, since their capacity is small. */
		dIOLog("fd: got Format Capacity 0x%lx",
			(unsigned long)capbuf[i]);
		blocks = capbuf[i] / 512;
		switch(capbuf[i]) {
        	    case 720: // 360k
			formatflags |= FD_FORMAT_360K; break;
        	    case 800: // 400k
			formatflags |= FD_FORMAT_400K; break;
        	    case 1440: // 720k
			formatflags |= FD_FORMAT_720K; break;
        	    case 1600: // 800k
			formatflags |= FD_FORMAT_800K; break;
        	    case 2880: // 1440k   
			formatflags |= FD_FORMAT_1440K; break;
        	    case 3360: // 1680k
			formatflags |= FD_FORMAT_1680K; break;
        	    case 5760: // 2880k
			formatflags |= FD_FORMAT_2880K; break;
		    default:
			formatflags |= FD_FORMAT_OTHER; break;
		}
	    }

	    if (!formatflags) {
		error = ENOTTY;
	    } else {
		dIOLog("fd: Format Flags are 0x%x\n", formatflags);
		*buffer = formatflags;
		error = 0;
	    }
	    break;
	}

        default:
        {
            //
            // A foreign ioctl was received.  Ask our superclass' opinion.
            //

	    IOLog("fd: unknown ioctl, calling parent.\n");
            error = super::ioctl(dev, cmd, data, flags, proc);

        } break;
    }

    return error;                                       // (return error status)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 0);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 1);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 2);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 3);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 4);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 5);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 6);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

OSMetaClassDefineReservedUnused(FloppyMediaBSDClient, 7);
