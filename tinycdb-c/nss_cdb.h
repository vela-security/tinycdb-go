/* nss_cdb.h: nss_cdb common include file.
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


#include <sys/types.h>
#include <stdlib.h>
#include <nss.h>
#include "cdb.h"

#ifndef NSSCDB_DIR
# define NSSCDB_DIR "/etc"
#endif
#ifndef NSSCDB_DB
# define NSSCDB_DB(name) NSSCDB_DIR "/" name ".cdb"
#endif

typedef int (nss_parse_fn)(void *result, char *buf, size_t bufl);

struct nss_cdb {
  nss_parse_fn *parsefn;
  const char *dbname;
  int keepopen;
  unsigned lastpos;
  struct cdb cdb;
};

enum nss_status
__nss_cdb_setent(struct nss_cdb *dbp, int stayopen);
enum nss_status
__nss_cdb_endent(struct nss_cdb *dbp);
enum nss_status
__nss_cdb_getent(struct nss_cdb *dbp,
		 void *result, char *buf, size_t bufl, int *errnop);
enum nss_status
__nss_cdb_byname(struct nss_cdb *dbp, const char *name,
		 void *result, char *buf, size_t bufl, int *errnop);
enum nss_status
__nss_cdb_byid(struct nss_cdb *dbp, unsigned long id,
	       void *result, char *buf, size_t bufl, int *errnop);

#define nss_common(dbname,structname,entname) \
static int \
nss_##dbname##_parse(structname *result, char *buf, size_t bufl); \
static struct nss_cdb db = { \
  (nss_parse_fn*)&nss_##dbname##_parse, \
  NSSCDB_DB(#dbname),0,0,CDB_STATIC_INIT}; \
enum nss_status _nss_cdb_set##entname(int stayopen) { \
  return __nss_cdb_setent(&db, stayopen); \
} \
enum nss_status _nss_cdb_end##entname(void) { \
  return __nss_cdb_endent(&db); \
} \
enum nss_status \
_nss_cdb_get##entname##_r(structname *result, \
                           char *buf, size_t bufl, int *errnop) { \
  return __nss_cdb_getent(&db, result, buf, bufl, errnop); \
}

#define nss_getbyname(getbyname, structname) \
enum nss_status \
_nss_cdb_##getbyname##_r(const char *name, structname *result, \
		          char *buf, size_t bufl, int *errnop) { \
  return __nss_cdb_byname(&db, name, result, buf, bufl, errnop); \
}

#define nss_getbyid(getbyid, structname, idtype) \
enum nss_status \
_nss_cdb_##getbyid##_r(idtype id, structname *result, \
		           char *buf, size_t bufl, int *errnop) { \
  return __nss_cdb_byid(&db, id, result, buf, bufl, errnop); \
}

#define STRING_FIELD(line, variable) \
   variable = line; \
   while(*line != ':') \
     if (!*line++) return -1; \
   *line++ = '\0'

#define INT_FIELD(line, variable, convert) \
 { \
   char *endp; \
   variable = convert(strtoul(line, &endp, 10)); \
   if (endp == line) return -1; \
   if (*endp++ != ':') return -1; \
   line = endp; \
 }

#define INT_FIELD_MAYBE_NULL(line, variable, convert, default) \
 { \
   char *endp; \
   if (!*line) return -1; \
   variable = convert(strtoul(line, &endp, 10)); \
   if (*endp != ':') return -1; \
   if (endp == line) variable = convert(default); \
   line = endp; \
 }

