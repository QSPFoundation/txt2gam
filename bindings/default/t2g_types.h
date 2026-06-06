/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef T2G_TYPES_H
    #define T2G_TYPES_H

    /* Apple doesn't ship <uchar.h>; define char16_t from the compiler built-in
       so it matches the type of u"..." string literals on all Apple targets. */
    #if defined(__APPLE__)
        typedef __CHAR16_TYPE__ char16_t;
    #else
        #include <uchar.h>
    #endif

    /* QSP_CHAR is a fixed 16-bit UTF-16 code unit on every platform. */
    typedef char16_t QSP_CHAR;
    #define QSP_FMT2(x) u##x
    #define QSP_FMT(x) QSP_FMT2(x)

    typedef int QSP_BOOL;

    #define QSP_TRUE  1
    #define QSP_FALSE 0

#endif /* T2G_TYPES_H */
