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

#ifndef _FLOPPYMEDIABSDCLIENT_H
#define _FLOPPYMEDIABSDCLIENT_H

#include <sys/ioctl.h>
#include <sys/types.h>

/* #include <IOKit/storage/IODVDTypes.h> */

/*
 * Definitions
 *
 * ioctl                        description
 * ---------------------------- ------------------------------------------------
 * DKIOCDVDREADSTRUCTURE        see IODVDMedia::readStructure()  in IODVDMedia.h
 *
 * DKIOCDVDREADDISCINFO         see IODVDMedia::readDiscInfo()   in IODVDMedia.h
 * DKIOCDVDREADRZONEINFO        see IODVDMedia::readRZoneInfo()  in IODVDMedia.h
 *
 * DKIOCDVDREPORTKEY            see IODVDMedia::reportKey()      in IODVDMedia.h
 * DKIOCDVDSENDKEY              see IODVDMedia::sendKey()        in IODVDMedia.h
 *
 * DKIOCDVDGETSPEED             see IODVDMedia::getSpeed()       in IODVDMedia.h
 * DKIOCDVDSETSPEED             see IODVDMedia::setSpeed()       in IODVDMedia.h
 */

typedef struct
{
    u_int8_t  format;

    u_int8_t  reserved0008[3];                     /* reserved, clear to zero */

    u_int32_t address;
    u_int8_t  grantID;
    u_int8_t  layer;

    u_int16_t bufferLength;
    void *    buffer;
} dk_dvd_read_structure_t;

typedef struct
{
    u_int8_t  format;
    u_int8_t  keyClass;

    u_int8_t  reserved0016[2];                     /* reserved, clear to zero */

    u_int32_t address;
    u_int8_t  grantID;

    u_int8_t  reserved0072[1];                     /* reserved, clear to zero */

    u_int16_t bufferLength;
    void *    buffer;
} dk_dvd_report_key_t;

typedef struct
{
    u_int8_t  format;
    u_int8_t  keyClass;

    u_int8_t  reserved0016[6];                     /* reserved, clear to zero */

    u_int8_t  grantID;

    u_int8_t  reserved0072[1];                     /* reserved, clear to zero */

    u_int16_t bufferLength;
    void *    buffer;
} dk_dvd_send_key_t;

typedef struct
{
    u_int8_t  reserved0000[10];                    /* reserved, clear to zero */

    u_int16_t bufferLength;                        /* actual length on return */
    void *    buffer;
} dk_dvd_read_disc_info_t;

typedef struct
{
    u_int8_t  reserved0000[4];                     /* reserved, clear to zero */

    u_int32_t address;
    u_int8_t  addressType;

    u_int8_t  reserved0072[1];                     /* reserved, clear to zero */

    u_int16_t bufferLength;                        /* actual length on return */
    void *    buffer;
} dk_dvd_read_rzone_info_t;

#define FD_FORMAT             _IOWR('F', 61, u_int32_t)
#define FD_VERIFY             _IOR('F',  68, u_int32_t)
#define FD_GETFORMATS         _IOR('F',  69, u_int32_t)

#define FD_FORMAT_360K  0x01 /* Obsolete */
#define FD_FORMAT_400K  0x02 /* Basically Obsolete */
#define FD_FORMAT_720K  0x04
#define FD_FORMAT_800K  0x08
#define FD_FORMAT_1440K 0x10
#define FD_FORMAT_1680K 0x20
#define FD_FORMAT_2880K 0x40
#define FD_FORMAT_OTHER 0x80

#ifdef KERNEL
#ifdef __cplusplus

/*
 * Kernel
 */

// #include "IOFloppyMedia.h"
#include <IOKit/storage/IOMediaBSDClient.h>

/*
 * Class
 */

class FloppyMediaBSDClient : public IOMediaBSDClient
{
    OSDeclareDefaultStructors(FloppyMediaBSDClient)

protected:

    IOBlockStorageDriver *driver;
    struct ExpansionData { /* */ };
    ExpansionData * _expansionData;

public:

    virtual bool FloppyMediaBSDClient::start(IOService *provider);
    /*
     * Obtain this object's provider.   We override the superclass's method
     * to return a more specific subclass of IOService -- IOMedia.  This
     * method serves simply as a convenience to subclass developers.
     */

    virtual IOMedia * getProvider() const;

    /*
     * Process a Floppy-specific ioctl.
     */

    virtual int ioctl(dev_t, u_long cmd, caddr_t data, int, struct proc *);

    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 0);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 1);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 2);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 3);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 4);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 5);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 6);
    OSMetaClassDeclareReservedUnused(FloppyMediaBSDClient, 7);
};

#endif /* __cplusplus */
#endif /* KERNEL */
#endif /* !_FLOPPYMEDIABSDCLIENT_H */
