/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "locations.h"

#ifndef QSP_LOCDATADEFINES
    #define QSP_LOCDATADEFINES

    #define QSP_BASESECTION_HEADER QSP_FMT("! BASE")
    #define QSP_BASESECTION_FOOTER QSP_FMT("! END BASE")
    #define QSP_BASEDESC_PRINT QSP_FMT("*P")
    #define QSP_BASEDESC_PRINTLINE QSP_FMT("*PL")
    #define QSP_BASEACTS_ACT_HEADER QSP_FMT("ACT")
    #define QSP_BASEACTS_ACT_FOOTER QSP_FMT("END")
    #define QSP_BASEACTS_LINE_PREFIX QSP_FMT("\t")

    /* External functions */
    QSP_BOOL qspInitLocProcessor();
    void qspTerminateLocProcessor();
    QSP_BOOL qspParseBaseDescriptionPrint(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspParseBaseDescriptionPrintLine(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspParseBaseAction(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspUpdateActionCode(QSP_CHAR *code, QSPLocation *loc);
    QSP_BOOL qspUpdateLocationCode(QSP_CHAR *code, QSPLocation *loc);

#endif
