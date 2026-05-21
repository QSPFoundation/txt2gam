/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_CODINGDEFINES
    #define QSP_CODINGDEFINES

    #define QSP_CODREMOV 5

    extern wchar_t qspCP1251ToUCS2LETable[];

    /* External functions */
    wchar_t qspDirectConvertUC(char, wchar_t *);
    char qspReverseConvertUC(wchar_t, wchar_t *);
    char *qspFromQSPString(QSP_CHAR *);
    QSP_CHAR *qspToQSPString(char *);
    char *qspQSPToGameString(QSP_CHAR *, QSP_BOOL, QSP_BOOL);
    QSP_CHAR *qspGameToQSPString(char *, QSP_BOOL, QSP_BOOL);
    int qspSplitGameStr(char *, QSP_BOOL, QSP_CHAR *, char ***);
    int qspGameCodeWriteVal(char **, int, QSP_CHAR *, QSP_BOOL, QSP_BOOL);
    int qspGameCodeWriteIntValLine(char **, int, int, QSP_BOOL, QSP_BOOL);
    int qspGameCodeWriteValLine(char **, int, QSP_CHAR *, QSP_BOOL, QSP_BOOL);
    char *qspQSPStringToUTF8(QSP_CHAR *s);
    QSP_CHAR *qspUTF8ToQSPString(char *s);

#endif
