/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */
#include "video.h"


/*
 *--------------------------------------------------------------
 *
 * ComputeVector --
 *
 *	Computes motion vector given parameters previously parsed
 *      and reconstructed.
 *
 * Results:
 *      Reconstructed motion vector info is put into recon_* parameters
 *      passed to this function.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

void ComputeVector( long *recon_ptr,
		    int r_size, BOOLEAN full_pel_vector,
		    long motion_code, long motion_r )
{
  long lim, vec, diff;

  vec = *recon_ptr;
  vec >>= full_pel_vector;
  vec += motion_code << r_size;
  diff = (-1L << r_size) + motion_r + 1L;
  lim = 16L << r_size;
  if (motion_code > 0) {
    vec += diff;
    if (vec >= lim) {
      vec -= lim; vec -= lim;
    }
  }
  else /* if (motion_code < 0) */ {
    vec -= diff; lim = -lim;
    if (vec < lim) {
      vec -= lim; vec -= lim;
    }
  }
  vec <<= full_pel_vector;
  *recon_ptr = vec;
}
