/* cdb_findnext.c: sequential cdb_find routines
 *
 * This file is a part of tinycdb package.
 * Copyright (C) 2001-2023 Michael Tokarev <mjt+cdb@corpit.ru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* see cdb_find.c for comments */

#include "cdb_int.h"

int
cdb_findinit(struct cdb_find *cdbfp, struct cdb *cdbp,
             const void *key, unsigned klen)
{
  unsigned n, pos;

  cdbfp->cdb_cdbp = cdbp;
  cdbfp->cdb_key = key;
  cdbfp->cdb_klen = klen;
  cdbfp->cdb_hval = cdb_hash(key, klen);

  cdbfp->cdb_htp = cdbp->cdb_mem + ((cdbfp->cdb_hval << 3) & 2047);
  n = cdb_unpack(cdbfp->cdb_htp + 4);
  cdbfp->cdb_httodo = n << 3;
  if (!n)
    return 0;
  pos = cdb_unpack(cdbfp->cdb_htp);
  if (n > (cdbp->cdb_fsize >> 3)
      || pos < cdbp->cdb_dend
      || pos > cdbp->cdb_fsize
      || cdbfp->cdb_httodo > cdbp->cdb_fsize - pos)
    return errno = EPROTO, -1;

  cdbfp->cdb_htab = cdbp->cdb_mem + pos;
  cdbfp->cdb_htend = cdbfp->cdb_htab + cdbfp->cdb_httodo;
  cdbfp->cdb_htp = cdbfp->cdb_htab + (((cdbfp->cdb_hval >> 8) % n) << 3);

  return 1;
}

int
cdb_findnext(struct cdb_find *cdbfp) {
  struct cdb *cdbp = cdbfp->cdb_cdbp;
  unsigned pos, n;
  unsigned klen = cdbfp->cdb_klen;

  while(cdbfp->cdb_httodo) {
    pos = cdb_unpack(cdbfp->cdb_htp + 4);
    if (!pos)
      return 0;
    n = cdb_unpack(cdbfp->cdb_htp) == cdbfp->cdb_hval;
    if ((cdbfp->cdb_htp += 8) >= cdbfp->cdb_htend)
      cdbfp->cdb_htp = cdbfp->cdb_htab;
    cdbfp->cdb_httodo -= 8;
    if (n) {
      if (pos > cdbp->cdb_fsize - 8)
	return errno = EPROTO, -1;
      if (cdb_unpack(cdbp->cdb_mem + pos) == klen) {
	if (cdbp->cdb_fsize - klen < pos + 8)
	  return errno = EPROTO, -1;
	if (memcmp(cdbfp->cdb_key,
	    cdbp->cdb_mem + pos + 8, klen) == 0) {
	  n = cdb_unpack(cdbp->cdb_mem + pos + 4);
	  pos += 8;
	  if (cdbp->cdb_fsize < n ||
              cdbp->cdb_fsize - n < pos + klen)
	    return errno = EPROTO, -1;
	  cdbp->cdb_kpos = pos;
	  cdbp->cdb_klen = klen;
	  cdbp->cdb_vpos = pos + klen;
	  cdbp->cdb_vlen = n;
	  return 1;
	}
      }
    }
  }

  return 0;

}
