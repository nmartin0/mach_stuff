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

#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "../FloppyMediaBSDClient/FloppyMediaBSDClient.h"

void printformats(int buffer);
int formatfromname(char *argptr);

main(int argc, char *argv[])
{
    int fd = -1;
    int ret;
    char buffer[4];
    int fail = 1;

    if ((argc == 3) && (!strcmp(argv[2], "-g") || !strcmp(argv[2],"-v")))
	fail = 0;
    if ((argc == 4) && !strcmp(argv[2], "-f") && formatfromname(argv[3]))
	fail = 0;
    if (fail) {
	fprintf(stderr, "Usage: fdformat <disk> -g           get list of valid formats\n");
	fprintf(stderr, "       fdformat <disk> -v           verify disk\n");
	fprintf(stderr, "       fdformat <disk> -f format    format disk with specified format\n");
	exit(1);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
	char *newname = (char *)malloc((strlen(argv[1]) + 6) * sizeof(char));
	if (!newname) {
	    fprintf(stderr, "Disk name too long.\n");
	    exit(-1);
	}
	strcpy(newname, "/dev");
	strcat(newname, argv[1]);
	fd = open(newname, O_RDONLY);
	free(newname);
    }
    if (fd == -1) {
	fprintf(stderr, "open() failed.  bailing.\n");
	exit(-1);
    }

    switch(argv[2][1]) {
	case 'g':
	    ioctl(fd, FD_GETFORMATS, &buffer);
	    printformats(*(int *)buffer);
	    break;
	case 'v':
	    ioctl(fd, FD_VERIFY, &buffer);
	    break;
	case 'f':
	  {
	    *(int *)buffer = formatfromname(argv[3]);
	    printf("format will be %d\n", *(int *)buffer);
	    printf("return is %d\n", ioctl(fd, FD_FORMAT, &buffer));
	    break;
	  }
    }

    close(fd);
}


void printformats(int buffer)
{
    printf("%d\n", buffer);
    if (buffer & FD_FORMAT_360K) printf("360K\n");
    if (buffer & FD_FORMAT_400K) printf("400K\n");
    if (buffer & FD_FORMAT_720K) printf("720K\n");
    if (buffer & FD_FORMAT_800K) printf("800K\n");
    if (buffer & FD_FORMAT_1440K) printf("1440K\n");
    if (buffer & FD_FORMAT_1680K) printf("1680K\n");
    if (buffer & FD_FORMAT_2880K) printf("2880K\n");
    if (buffer & FD_FORMAT_OTHER) printf("FD_FORMAT_OTHER\n");
}


int formatfromname(char *argptr)
{
    int ret = 0;

    if (!strcasecmp(argptr, "360K")) {
	ret = FD_FORMAT_360K;
    } else if (!strcasecmp(argptr, "400K")) {
	ret = FD_FORMAT_400K;
    } else if (!strcasecmp(argptr, "720K")) {
	ret = FD_FORMAT_720K;
    } else if (!strcasecmp(argptr, "800K")) {
	ret = FD_FORMAT_800K;
    } else if (!strcasecmp(argptr, "1440K")) {
	ret = FD_FORMAT_1440K;
    } else if (!strcasecmp(argptr, "1680K")) {
	ret = FD_FORMAT_1680K;
    } else if (!strcasecmp(argptr, "2880K")) {
	ret = FD_FORMAT_2880K;
    }

    return ret;
}

