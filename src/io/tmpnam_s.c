/*------------------------------------------------------------------
 * tmpnam_s.c
 *
 * September 2017, Reini Urban
 *
 * Copyright (c) 2017 by Reini Urban
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

/* Need restrict */
#include "config.h"
#include "safe_str_lib.h"
#include "safe_str_constraint.h"
#include <stdio.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif


/** 
 * @brief 
 *    Creates a unique valid file name (no longer than L_tmpnam in
 *    length) and stores it in character string pointed to by
 *    filename. The function is capable of generating up to TMP_MAX_S
 *    of unique filenames, but some or all of them may be in use in
 *    the filesystem and thus not suitable return values.
 *
 * @remark SPECIFIED IN
 *    * C11 standard (ISO/IEC 9899:2011):
 *    K.3.5.1.2 The tmpnam_s function (p: 587-588)
 *    http://en.cppreference.com/w/c/io/tmpnam
 *    * Deprecated in favor of mkstemp
 *
 * @param[out] filename_s pointer to the character array capable of holding at
 *                        least L_tmpnam_s bytes, to be used as a result buffer.
 * @param[in]  maxsize	  maximum number of characters the function is allowed
 *                        to write (typically the size of the filename_s array).
 *
 * @pre No more than TMP_MAX_S files may be opened
 * @pre filename_s is a null pointer
 * @pre maxsize is greater than RSIZE_MAX_STR
 * @pre maxsize is less than the generated file name string
 *
 * @return Returns zero and writes the file name to filename_s on
 * success. On error, returns non-zero and writes the null character
 * to filename_s[0] (only if filename_s is not null and maxsize is not
 * zero and is not greater than RSIZE_MAX_STR).
 *
 * @retval  EOK        on success
 * @retval  ESNULLP    when filename_s is a NULL pointer
 * @retval  ESZEROL    when maxsize = 0
 * @retval  ESLEMAX    when maxsize > RSIZE_MAX_STR or
 *                     more than TMP_MAX_S files were opened.
 * @retval  errno()   when tmpnam() failed, typically -ENOENT
 *
 * @note
 *   Although the names generated by tmpnam_s are difficult to guess, it
 *   is possible that a file with that name is created by another
 *   process between the moment tmpnam returns and the moment this
 *   program attempts to use the returned name to create a file. The
 *   standard function tmpfile and the POSIX function mkstemp do not
 *   have this problem (creating a unique directory using only the
 *   standard C library still requires the use of tmpnam_s).
 *
 *   POSIX systems additionally define the similarly named function
 *   tempnam(), which offers the choice of a directory (which defaults
 *   to the optionally defined macro P_tmpdir).
 */

errno_t tmpnam_s(char *filename_s, rsize_t maxsize)
{
    static int count = 0;
    char* result = NULL;

    if (unlikely(filename_s == NULL)) {
        invoke_safe_str_constraint_handler("tmpnam_s: filename_s is null",
                   NULL, ESNULLP);
        return ESNULLP;
    }

    if (unlikely(maxsize == 0)) {
        invoke_safe_str_constraint_handler("tmpnam_s: maxsize is 0",
                   NULL, ESZEROL);
        return ESZEROL;
    }

    if (unlikely(maxsize > RSIZE_MAX_STR || maxsize > L_tmpnam_s)) {
        invoke_safe_str_constraint_handler("tmpnam_s: maxsize exceeds max",
                   NULL, ESLEMAX);
        return ESLEMAX;
    }

    ++count;
    if (unlikely(count > TMP_MAX_S)) {
        invoke_safe_str_constraint_handler("tmpnam_s: exceeds TMP_MAX_S",
                   NULL, ESLEMAX);
        return ESLEMAX;
    }

#ifdef __clang
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic warning "-Wdeprecated-declarations"
#endif

    result = tmpnam(filename_s);

#ifdef __clang
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    if (result) {
        int len = strlen(result);

        if (unlikely((rsize_t)len > maxsize)) {
            invoke_safe_str_constraint_handler("tmpnam_s: length exceeds size",
                                               NULL, ESNOSPC);
            *result = '\0';
            return ESNOSPC;
        }

        if (unlikely(len > L_tmpnam_s)) {
            invoke_safe_str_constraint_handler("tmpnam_s: length exceeds L_tmpnam_s",
                                               NULL, ESLEMAX);
            *result = '\0';
            return ESLEMAX;
        }
#ifdef SAFECLIB_STR_NULL_SLACK
        memset_s(&result[len], maxsize, 0, maxsize-len);
#endif
        return EOK;
    } else {
        invoke_safe_str_constraint_handler("tmpnam_s: tmpnam() failed",
                   NULL, ESNOTFND);
        return errno;
    }
}
EXPORT_SYMBOL(tmpnam_s)
