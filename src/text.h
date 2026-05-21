/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"

#ifndef QSP_TEXTDEFINES
    #define QSP_TEXTDEFINES

    #define QSP_STRSDELIM QSP_FMT("\r\n")
    #define QSP_SPACES QSP_FMT(" \t")
    #define QSP_QUOTS QSP_FMT("'\"")
    #define QSP_NEWLINE QSP_FMT('\n')
    #define QSP_LQUOT QSP_FMT('{')
    #define QSP_RQUOT QSP_FMT('}')
    #define QSP_COLON QSP_FMT(':')
    #define QSP_COMMA QSP_FMT(',')

    /* Helpers */
    #define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

    /* External functions */
    void qspFormatLineEndings(QSP_CHAR *data);
    QSP_CHAR *qspAllocateBuffer(int bufSize);
    int qspAddCharToBuffer(QSP_CHAR **buf, QSP_CHAR ch, int strLen, int *bufSize);
    int qspAddTextToBuffer(QSP_CHAR **buf, QSP_CHAR *val, int valLen, int strLen, int *bufSize);
    int qspAddText(QSP_CHAR **dest, QSP_CHAR *val, int destLen, int valLen, QSP_BOOL toCreate);
    QSP_BOOL qspIsInList(QSP_CHAR *list, QSP_CHAR ch);
    QSP_CHAR *qspSkipSpaces(QSP_CHAR *s);
    QSP_CHAR *qspDelSpc(QSP_CHAR *s);
    QSP_CHAR *qspNewStr(QSP_CHAR *s);
    QSP_CHAR *qspStrEnd(QSP_CHAR *s);
    int qspStrLen(QSP_CHAR *str);
    QSP_BOOL qspIsEmpty(QSP_CHAR *str);
    QSP_CHAR *qspStrChr(QSP_CHAR *str, QSP_CHAR ch);
    QSP_CHAR *qspStrStr(QSP_CHAR *str, QSP_CHAR *strSearch);
    QSP_CHAR *qspStrCopy(QSP_CHAR *strDest, QSP_CHAR *strSource);
    QSP_CHAR *qspStrNCopy(QSP_CHAR *strDest, QSP_CHAR *strSource, int maxLen);
    QSP_CHAR *qspReplaceText(QSP_CHAR *txt, QSP_CHAR *searchTxt, QSP_CHAR *repTxt);
    int qspStrsComp(QSP_CHAR *str1, QSP_CHAR *str2);
    QSP_BOOL qspIsEqual(QSP_CHAR *str1, QSP_CHAR *str2, int maxLen);
    int qspSplitStr(QSP_CHAR *str, QSP_CHAR *delim, QSP_CHAR ***res);
    void qspFreeStrs(void **strs, int count);
    QSP_BOOL qspIsDigit(QSP_CHAR ch);
    int qspStrToNum(QSP_CHAR *s, QSP_BOOL *isValid);
    QSP_CHAR *qspNumToStr(QSP_CHAR *buf, int val);

#endif
