/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "../src/t2g_api.h"

QSP_CHAR *t2gWasmParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = t2gParseTextData(data, dataLen, isUnicode, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}

char *t2gWasmEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                              QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password, int *outLen)
{
    int len = 0;
    char *result = t2gEncodeTextToGame(text, locStart, locEnd, isOldFormat, isUnicode, password, &len);
    *outLen = len;
    return result;
}

QSP_CHAR *t2gWasmDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                                  const QSP_CHAR *locStart, const QSP_CHAR *locEnd, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = t2gDecodeGameToText(data, dataLen, password, locStart, locEnd, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}

QSP_CHAR *t2gWasmExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                                const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, int *outLen)
{
    int len = 0;
    QSP_CHAR *result = t2gExtractStrings(text, locStart, locEnd, toGetQStrings, &len);
    *outLen = len > 0 ? len - 1 : 0;
    return result;
}
