/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_MAINDEFINES
    #define QSP_MAINDEFINES

    #define QSP_STARTLOC QSP_FMT("#")
    #define QSP_ENDLOC QSP_FMT("--")
    #define TXT2GAM_UCS2BOM "\xFF\xFE"
    #define TXT2GAM_UTF8BOM "\xEF\xBB\xBF"

    enum Mode
    {
        QSP_ERROR,
        QSP_ENCODE_INTO_GAME,
        QSP_DECODE_INTO_TEXT,
        QSP_EXTRACT_STRINGS,
        QSP_EXTRACT_QSTRINGS
    };

#endif
