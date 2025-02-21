/* cdb_make_add.c: basic cdb_make_add routine
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

#include <stdlib.h> /* for malloc */
#include "cdb_int.h"
#include <stdio.h>

int internal_function
_cdb_make_add(struct cdb_make *cdbmp, unsigned hval,
              const void *key, unsigned klen,
              const void *val, unsigned vlen)
{
  unsigned char rlen[8];
  struct cdb_rl *rl;
  unsigned i;
  if (klen > 0xffffffff - (cdbmp->cdb_dpos + 8) ||
      vlen > 0xffffffff - (cdbmp->cdb_dpos + klen + 8))
    return errno = ENOMEM, -1;
  i = hval & 255;
  rl = cdbmp->cdb_rec[i];
  if (!rl || rl->cnt >= sizeof(rl->rec)/sizeof(rl->rec[0])) {
    rl = (struct cdb_rl*)malloc(sizeof(struct cdb_rl));
    if (!rl)
      return errno = ENOMEM, -1;
    rl->cnt = 0;
    rl->next = cdbmp->cdb_rec[i];
    cdbmp->cdb_rec[i] = rl;
  }
  i = rl->cnt++;
  rl->rec[i].hval = hval;
  rl->rec[i].rpos = cdbmp->cdb_dpos;
  ++cdbmp->cdb_rcnt;
  cdb_pack(klen, rlen);
  cdb_pack(vlen, rlen + 4);
  if (_cdb_make_write(cdbmp, rlen, 8) < 0 ||
      _cdb_make_write(cdbmp, key, klen) < 0 ||
      _cdb_make_write(cdbmp, val, vlen) < 0)
    return -1;
  return 0;
}

int
cdb_make_add(struct cdb_make *cdbmp,
             const void *key, unsigned klen,
             const void *val, unsigned vlen) {
    
  return _cdb_make_add(cdbmp, cdb_hash(key, klen), key, klen, val, vlen);
}

int cdb_make_add_go(uintptr_t ptr , char *key, uint32_t klen, char *val, uint32_t vlen)
{
    if (ptr == 0) {
        printf("%d" , ptr);
        return 0;
    }

    struct cdb_make *cdbm;
    cdbm = (struct cdb_make *)ptr;
    return cdb_make_add(cdbm , key , klen , val , vlen);
}
