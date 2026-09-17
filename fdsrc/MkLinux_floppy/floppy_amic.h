/*
 * MkLinux Public Source License
 * v1.0
 *
 * Portions Copyright (c) 1999-2000 The MkLinux Project.
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
 *	This product includes software developed by The MkLinux Project
 *	and its contributors.
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
 * requirements.
 *
 * License is hereby granted to freely redistribute this code under the terms
 * of the GPL or any other certified open source license agreement provided that
 * this copyright notice remain intact, and provided that all copies of the source
 * code contain, in some reasonable, human-readable form, information telling
 * where any party may obtain a copy of this source code that is not burdened
 * by the GPL or other license's potentially more restrictive licensing terms.
 */
/*
 * MkLinux
 */

#ifndef __floppy_amic_h__
#define __floppy_amic_h__

void	floppy_amic_init(void);
void	floppy_amic_setup(vm_offset_t address, unsigned int count);
void	floppy_amic_start(boolean_t isread);
void	floppy_amic_end(void);
void	floppy_amic_ack(void);

struct dma_softc {
	volatile unsigned char	*base;
	volatile unsigned char	*ctrl;
	volatile unsigned char  *bytecount_low;
	volatile unsigned char  *bytecount_high;
} floppy_amic_softc[NFD];

typedef struct dma_softc dma_softc_t;

#endif 	/* __floppy_amic_h__ */

