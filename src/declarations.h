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

#include "txt2gam_config.h"
#include "t2g_types.h"

/* MEMWATCH */

#ifdef _DEBUG
    #define MEMWATCH
    #define MEMWATCH_STDIO

    #include "memwatch.h"
#endif

#ifndef QSP_DEFINES
    #define QSP_DEFINES

    #define QSP_TO_GAME_SB(a) qspReverseConvertUC(a, qspCP1251ToUTF16LETable)
    #define QSP_TO_GAME_UC(a) (a)
    #define QSP_FROM_GAME_SB(a) qspDirectConvertUC(a, qspCP1251ToUTF16LETable)
    #define QSP_FROM_GAME_UC(a) (a)

    enum
    {
        QSP_UNKNOWN,
        QSP_ANSI,
        QSP_UTF8,
        QSP_UTF16
    };

#endif
