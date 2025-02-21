/* cdb.h: public cdb include file
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

#ifndef TINYCDB_VERSION
#define TINYCDB_VERSION 0.80

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

typedef unsigned int cdbi_t; /* compatibility */

/* common routines */
unsigned cdb_hash(const void *buf, unsigned len);
unsigned cdb_unpack(const unsigned char buf[4]);
void cdb_pack(unsigned num, unsigned char buf[4]);

struct cdb {
  int cdb_fd;			/* file descriptor */
  /* private members */
  unsigned cdb_fsize;		/* datafile size */
  unsigned cdb_dend;		/* end of data ptr */
  const unsigned char *cdb_mem; /* mmap'ed file memory */
  unsigned cdb_vpos, cdb_vlen;	/* found data */
  unsigned cdb_kpos, cdb_klen;	/* found key */
};

#define CDB_STATIC_INIT {0,0,0,0,0,0,0,0}

#define cdb_datapos(c) ((c)->cdb_vpos)
#define cdb_datalen(c) ((c)->cdb_vlen)
#define cdb_keypos(c) ((c)->cdb_kpos)
#define cdb_keylen(c) ((c)->cdb_klen)
#define cdb_fileno(c) ((c)->cdb_fd)

int cdb_init(struct cdb *cdbp, int fd);
void cdb_free(struct cdb *cdbp);

int cdb_read(const struct cdb *cdbp,
             void *buf, unsigned len, unsigned pos);
#define cdb_readdata(cdbp, buf) \
        cdb_read((cdbp), (buf), cdb_datalen(cdbp), cdb_datapos(cdbp))
#define cdb_readkey(cdbp, buf) \
        cdb_read((cdbp), (buf), cdb_keylen(cdbp), cdb_keypos(cdbp))

const void *cdb_get(const struct cdb *cdbp, unsigned len, unsigned pos);
#define cdb_getdata(cdbp) \
        cdb_get((cdbp), cdb_datalen(cdbp), cdb_datapos(cdbp))
#define cdb_getkey(cdbp) \
        cdb_get((cdbp), cdb_keylen(cdbp), cdb_keypos(cdbp))

int cdb_find(struct cdb *cdbp, const void *key, unsigned klen);
uintptr_t cdb_find_start_go(const char *file);
int cdb_find_go(uintptr_t ptr , const char *key , uint32_t klen,
        int(*finder)(uintptr_t ptr , unsigned size));

int cdb_find_all_go(uintptr_t ptr , const char *key , uint32_t klen, 
        uint32_t (*finder)(uintptr_t ptr, unsigned size));
int cdb_foreach_go(uintptr_t ptr , 
        uint32_t (*finder)(uintptr_t key, unsigned klen , uintptr_t val, unsigned vlen));

int cdb_find_close(uintptr_t ptr); 

struct cdb_find {
  struct cdb *cdb_cdbp;
  unsigned cdb_hval;
  const unsigned char *cdb_htp, *cdb_htab, *cdb_htend;
  unsigned cdb_httodo;
  const void *cdb_key;
  unsigned cdb_klen;
};

int cdb_findinit(struct cdb_find *cdbfp, struct cdb *cdbp,
                 const void *key, unsigned klen);
int cdb_findnext(struct cdb_find *cdbfp);

#define cdb_seqinit(cptr, cdbp) ((*(cptr))=2048)
int cdb_seqnext(unsigned *cptr, struct cdb *cdbp);

/* old simple interface */
/* open file using standard routine, then: */
int cdb_seek(int fd, const void *key, unsigned klen, unsigned *dlenp);
int cdb_bread(int fd, void *buf, int len);

/* cdb_make */

struct cdb_make {
  int cdb_fd;			/* file descriptor */
  /* private */
  unsigned cdb_dpos;		/* data position so far */
  unsigned cdb_rcnt;		/* record count so far */
  unsigned char cdb_buf[4096];	/* write buffer */
  unsigned char *cdb_bpos;	/* current buf position */
  struct cdb_rl *cdb_rec[256];	/* list of arrays of record infos */
};

enum cdb_put_mode {
  CDB_PUT_ADD = 0,	/* add unconditionnaly, like cdb_make_add() */
#define CDB_PUT_ADD	CDB_PUT_ADD
  CDB_FIND = CDB_PUT_ADD,
  CDB_PUT_REPLACE,	/* replace: do not place to index OLD record */
#define CDB_PUT_REPLACE	CDB_PUT_REPLACE
  CDB_FIND_REMOVE = CDB_PUT_REPLACE,
  CDB_PUT_INSERT,	/* add only if not already exists */
#define CDB_PUT_INSERT	CDB_PUT_INSERT
  CDB_PUT_WARN,		/* add unconditionally but ret. 1 if exists */
#define CDB_PUT_WARN	CDB_PUT_WARN
  CDB_PUT_REPLACE0,	/* if a record exists, fill old one with zeros */
#define CDB_PUT_REPLACE0 CDB_PUT_REPLACE0
  CDB_FIND_FILL0 = CDB_PUT_REPLACE0
};

int cdb_make_start(struct cdb_make *cdmp, int fd);
int cdb_make_add(struct cdb_make *cdbmp,
                 const void *key, unsigned klen,
                 const void *val, unsigned vlen);
int cdb_make_exists(struct cdb_make *cdbmp,
                    const void *key, unsigned klen);
int cdb_make_find(struct cdb_make *cdbmp,
                  const void *key, unsigned klen,
                  enum cdb_put_mode mode);
int cdb_make_put(struct cdb_make *cdbmp,
                 const void *key, unsigned klen,
                 const void *val, unsigned vlen,
                 enum cdb_put_mode mode);

int cdb_make_finish(struct cdb_make *cdbmp);

uintptr_t cdb_make_start_go(char *file);

int cdb_make_add_go( uintptr_t ptr,
                 char *key, uint32_t klen,
                 char *val, uint32_t vlen);

int cdb_make_put_go(uintptr_t ptr,
                 const void *key, unsigned klen,
                 const void *val, unsigned vlen,
                 enum cdb_put_mode mode);

int cdb_make_finish_go(uintptr_t ptr);
int cdb_make_free_go(uintptr_t ptr);
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* include guard */
