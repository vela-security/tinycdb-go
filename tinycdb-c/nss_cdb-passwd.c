/* nss_cdb-passwd.c: nss_cdb passwd database routines.
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

#include "nss_cdb.h"
#include <pwd.h>
#include <stdlib.h>

nss_common(passwd, struct passwd, pwent);
nss_getbyname(getpwnam, struct passwd);
nss_getbyid(getpwuid, struct passwd, uid_t);

static int
nss_passwd_parse(struct passwd *result, char *buf, size_t bufl) {

  STRING_FIELD(buf, result->pw_name);
  if (!result->pw_name[0]) return -1;
  STRING_FIELD(buf, result->pw_passwd);
  INT_FIELD(buf, result->pw_uid, (uid_t));
  INT_FIELD(buf, result->pw_gid, (gid_t));
  STRING_FIELD(buf, result->pw_gecos);
  STRING_FIELD(buf, result->pw_dir);
  result->pw_shell = buf;

  bufl = bufl;

  return 1;
}

#ifdef TEST
#include <stdio.h>

static void printit(const struct passwd *p) {
  printf("name=`%s' pass=`%s' uid=%d gid=%d gecos=`%s' dir=`%s' shell=`%s'\n",
	 p->pw_name, p->pw_passwd, p->pw_uid, p->pw_gid, p->pw_gecos, p->pw_dir, p->pw_shell);
}

int main(int argc, char **argv) {
  struct passwd pw, *p;
  char buf[1024];
  int err, r;
  while(*++argv) {
    r = _nss_cdb_getpwuid_r(atoi(*argv), &pw, buf, sizeof(buf), &err);
    if (r == NSS_STATUS_SUCCESS)
      printit(&pw);
    else
      printf("cdb(%s): %d %s\n", *argv, r, strerror(err));
    p = getpwuid(atoi(*argv));
    if (p) printit(p);
    else printf("pwuid(%s): %m\n", *argv);
  }
  return 0;
}
#endif
