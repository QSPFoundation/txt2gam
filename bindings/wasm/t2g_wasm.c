/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "t2g_api.h"

static int t2gWasmLastError = T2G_ERROR_NONE;

int t2gWasmGetLastError(void)
{
    return t2gWasmLastError;
}

QSP_CHAR *t2gWasmParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = 0;
    t2gWasmLastError = t2gParseTextData(data, dataLen, isUnicode, &result, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}

char *t2gWasmEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                              QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password, int *outLen)
{
    int len = 0;
    char *result = 0;
    t2gWasmLastError = t2gEncodeTextToGame(text, locStart, locEnd, isOldFormat, isUnicode, password, &result, &len);
    *outLen = len;
    return result;
}

QSP_CHAR *t2gWasmDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                                  const QSP_CHAR *locStart, const QSP_CHAR *locEnd, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = 0;
    t2gWasmLastError = t2gDecodeGameToText(data, dataLen, password, locStart, locEnd, &result, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}

QSP_CHAR *t2gWasmExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                                const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = 0;
    t2gWasmLastError = t2gExtractStrings(text, locStart, locEnd, toGetQStrings, &result, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}
