/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "txt2gam_config.h"

/* MEMWATCH */

#ifdef _DEBUG
    #define MEMWATCH
    #define MEMWATCH_STDIO

    #include "memwatch.h"
#endif

#ifndef QSP_DEFINES
    #define QSP_DEFINES

    #ifdef _UNICODE
        typedef wchar_t QSP_CHAR;
        #define QSP_FMT2(x) L##x
        #define QSP_FMT(x) QSP_FMT2(x)

        #define QSP_STRFTIME wcsftime
        #define QSP_WCSTOMBSLEN(a) wcstombs(0, a, 0)
        #define QSP_WCSTOMBS wcstombs
        #define QSP_MBSTOWCSLEN(a) mbstowcs(0, a, 0)
        #define QSP_MBSTOWCS mbstowcs
        #define QSP_TO_GAME_SB(a) qspReverseConvertUC(a, qspCP1251ToUCS2LETable)
        #define QSP_TO_GAME_UC(a) (a)
        #define QSP_FROM_GAME_SB(a) qspDirectConvertUC(a, qspCP1251ToUCS2LETable)
        #define QSP_FROM_GAME_UC(a) (a)
    #else
        typedef char QSP_CHAR;
        #define QSP_FMT(x) x

        #define QSP_STRFTIME strftime
        #define QSP_WCSTOMBSLEN strlen
        #define QSP_WCSTOMBS strncpy
        #define QSP_MBSTOWCSLEN strlen
        #define QSP_MBSTOWCS strncpy
        #define QSP_TO_GAME_SB(a) (a)
        #define QSP_TO_GAME_UC(a) qspDirectConvertUC(a, qspCP1251ToUCS2LETable)
        #define QSP_FROM_GAME_SB(a) (a)
        #define QSP_FROM_GAME_UC(a) qspReverseConvertUC(a, qspCP1251ToUCS2LETable)
    #endif

    enum
    {
        QSP_UNKNOWN,
        QSP_ANSI,
        QSP_UTF8,
        QSP_UCS2
    };

    #define QSP_APPNAME QSP_FMT("TXT2GAM")
    #define QSP_VER QSP_FMT(TXT2GAM_VER_STR)
    #define QSP_LOCALE "C" /* default locale */

    #define QSP_TRUE 1
    #define QSP_FALSE 0

    /* Types */
    typedef int QSP_BOOL;

#endif
