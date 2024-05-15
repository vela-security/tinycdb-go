/* cdb_find.c: cdb_find routine
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

#include "cdb_int.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
  #ifdef _O_BINARY
    #define O_BINARY _O_BINARY
  #else
    #define O_BINARY 0 // system doesn't need O_BINARY
  #endif
#endif


int
cdb_find(struct cdb *cdbp, const void *key, unsigned klen)
{
  const unsigned char *htp;	/* hash table pointer */
  const unsigned char *htab;	/* hash table */
  const unsigned char *htend;	/* end of hash table */
  unsigned httodo;		/* ht bytes left to look */
  unsigned pos, n;

  unsigned hval;

  if (klen >= cdbp->cdb_dend)	/* if key size is too large */
    return 0;

  hval = cdb_hash(key, klen);

  /* find (pos,n) hash table to use */
  /* first 2048 bytes (toc) are always available */
  /* (hval % 256) * 8 */
  htp = cdbp->cdb_mem + ((hval << 3) & 2047); /* index in toc (256x8) */
  n = cdb_unpack(htp + 4);	/* table size */
  if (!n)			/* empty table */
    return 0;			/* not found */
  httodo = n << 3;		/* bytes of htab to lookup */
  pos = cdb_unpack(htp);	/* htab position */
  if (n > (cdbp->cdb_fsize >> 3) /* overflow of httodo ? */
      || pos < cdbp->cdb_dend /* is htab inside data section ? */
      || pos > cdbp->cdb_fsize /* htab start within file ? */
      || httodo > cdbp->cdb_fsize - pos) /* entrie htab within file ? */
    return errno = EPROTO, -1;

  htab = cdbp->cdb_mem + pos;	/* htab pointer */
  htend = htab + httodo;	/* after end of htab */
  /* htab starting position: rest of hval modulo htsize, 8bytes per elt */
  htp = htab + (((hval >> 8) % n) << 3);

  for(;;) {
    pos = cdb_unpack(htp + 4);	/* record position */
    if (!pos)
      return 0;
    if (cdb_unpack(htp) == hval) {
      if (pos > cdbp->cdb_dend - 8) /* key+val lengths */
	return errno = EPROTO, -1;
      if (cdb_unpack(cdbp->cdb_mem + pos) == klen) {
	if (cdbp->cdb_dend - klen < pos + 8)
	  return errno = EPROTO, -1;
	if (memcmp(key, cdbp->cdb_mem + pos + 8, klen) == 0) {
	  n = cdb_unpack(cdbp->cdb_mem + pos + 4);
	  pos += 8;
	  if (cdbp->cdb_dend < n || cdbp->cdb_dend - n < pos + klen)
	    return errno = EPROTO, -1;
	  cdbp->cdb_kpos = pos;
	  cdbp->cdb_klen = klen;
	  cdbp->cdb_vpos = pos + klen;
	  cdbp->cdb_vlen = n;
	  return 1;
	}
      }
    }
    httodo -= 8;
    if (!httodo)
      return 0;
    if ((htp += 8) >= htend)
      htp = htab;
  }
}

uintptr_t cdb_find_start_go(const char *file) {
    struct cdb *cdbp = (struct cdb *)malloc(sizeof(struct cdb));
    int fd , ret;
    fd = open(file , O_RDONLY|O_BINARY);
    if (fd < 0) {
        return 0;
    }

    ret = cdb_init(cdbp , fd);
    if (ret < 0) {
        return 0;
    }
    
    return (uintptr_t)cdbp;
}

int cdb_find_go(uintptr_t ptr, 
        const char *key , uint32_t klen , 
        int (*finder)(uintptr_t p , unsigned size)) { 
    int ret;
    if (ptr == 0 ) {
        return -1;
    }

    struct cdb *cdbp = (struct cdb *)ptr;
    if (cdbp == NULL) {
        return -1;
    }
    
    const char *item;
    ret = cdb_find(cdbp , key , klen);
    if (ret > 0 ) {
        finder((uintptr_t)cdb_getdata(cdbp) , cdb_datalen(cdbp));
    }

    return ret;
}

int cdb_find_all_go(uintptr_t ptr, 
        const char *key , uint32_t klen , 
        uint32_t (*finder)(uintptr_t p , unsigned size)) { 
    int ret;
    if (ptr == 0 ) {
        return -1;
    }

    struct cdb *cdbp = (struct cdb *)ptr;
    if (cdbp == NULL) {
        return -1;
    }
    
    struct cdb_find cdbf;
    cdb_findinit(&cdbf , cdbp , key , klen);
    
    uint32_t stop;
    while((ret = cdb_findnext(&cdbf))) {
        if (ret < 0) {
            return -1;
        }

        if (ret == 0) {
            return 0;
        }
    
        stop = finder((uintptr_t)cdb_getdata(cdbp) ,cdb_datalen(cdbp));
        if (stop == 1) {
            return 0;
        }
    }
    
    return 1;
}

int cdb_foreach_go(uintptr_t ptr, 
        uint32_t (*finder)(uintptr_t key , unsigned klen , uintptr_t val, unsigned vlen)) { 
    int ret;
    if (ptr == 0 ) {
        return -1;
    }

    struct cdb *cdbp = (struct cdb *)ptr;
    if (cdbp == NULL) {
        return -1;
    }

    unsigned pos;
    uint32_t stop;
    cdb_seqinit(&pos , cdbp);
    while((ret = cdb_seqnext(&pos,cdbp))) {
        if (ret < 0) {
            return ret;
        }

        if (ret == 0) {
            return 0;
        }

        stop = finder((uintptr_t)cdb_getkey(cdbp) ,cdb_keylen(cdbp) ,(uintptr_t)cdb_getdata(cdbp) , cdb_datalen(cdbp));
        if (stop == 1) {
            return 0;
        }
    }
    return 1;
}

int cdb_find_close(uintptr_t ptr) {

    if (ptr == 0) {
        return 0;
    }
    struct cdb *cdbp;
    cdbp = (struct cdb *)ptr;
    if (cdbp == NULL) {
        return 0;
    }

    close(cdbp->cdb_fd);
    return 0;
}
