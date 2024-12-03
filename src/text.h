/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"

#ifndef QSP_TEXTDEFINES
    #define QSP_TEXTDEFINES

    #define QSP_STRSDELIM QSP_FMT("\r\n")
    #define QSP_NEWLINE QSP_FMT('\n')
    #define QSP_SPACES QSP_FMT(" \t")
    #define QSP_QUOTS QSP_FMT("'\"")
    #define QSP_LQUOT QSP_FMT("{")
    #define QSP_RQUOT QSP_FMT("}")

    /* Helpers */
    #define QSP_LEN(x) (sizeof(x) / sizeof(QSP_CHAR) - 1)

    /* External functions */
    void qspFormatLineEndings(QSP_CHAR *data);
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
