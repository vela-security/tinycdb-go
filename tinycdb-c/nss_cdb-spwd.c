/* nss_cdb-spwd.c: nss_cdb shadow passwd database routines.
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
#include <shadow.h>

nss_common(shadow, struct spwd, spent);
nss_getbyname(getspnam, struct spwd);

static int
nss_shadow_parse(struct spwd *result, char *buf, size_t bufl) {

  STRING_FIELD(buf, result->sp_namp);
  if (!result->sp_namp[0]) return -1;
  STRING_FIELD(buf, result->sp_pwdp);
  INT_FIELD_MAYBE_NULL(buf, result->sp_lstchg, (long), -1);
  INT_FIELD_MAYBE_NULL(buf, result->sp_min, (long), -1);
  INT_FIELD_MAYBE_NULL(buf, result->sp_max, (long), -1);
  INT_FIELD_MAYBE_NULL(buf, result->sp_warn, (long), -1);
  INT_FIELD_MAYBE_NULL(buf, result->sp_inact, (long), -1);
  INT_FIELD_MAYBE_NULL(buf, result->sp_expire, (long), -1);
  if (*buf) {
    result->sp_flag = strtoul(buf, &buf, 10);
    if (*buf) return -1;
  }
  else
    result->sp_flag = ~0ul;

  bufl = bufl;

  return 1;
}
