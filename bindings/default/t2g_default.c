/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "t2g_default.h"
#include "../src/t2g_api.h"
#include "../src/coding.h"
#include <stdlib.h>
#include <string.h>

int t2gReadTextData(const char *data, int dataLen, QSP_BOOL isUnicode, QSP_CHAR *outBuf, int outBufSize)
{
    int len = 0;
    QSP_CHAR *result = t2gParseTextData(data, dataLen, isUnicode, &len);
    if (!result) return -1;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, result, len * sizeof(QSP_CHAR));
    free(result);
    return len;
}

int t2gWriteTextData(const QSP_CHAR *text, QSP_BOOL isUnicode, char *outBuf, int outBufSize)
{
    int len = 0;
    char *result = t2gEncodeTextData(text, isUnicode, &len);
    if (!result) return -1;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, result, len);
    free(result);
    return len;
}

int t2gTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                  QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password,
                  char *outBuf, int outBufSize)
{
    int len;
    char *gameData = t2gEncodeTextToGame(text, locStart, locEnd, isOldFormat, isUnicode, password, &len);
    if (!gameData) return -1;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, gameData, len);
    free(gameData);
    return len;
}

int t2gGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                  const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                  QSP_CHAR *outBuf, int outBufSize)
{
    int len;
    QSP_CHAR *textData = t2gDecodeGameToText(data, dataLen, password, locStart, locEnd, &len);
    if (!textData) return -1;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, textData, len * sizeof(QSP_CHAR));
    free(textData);
    return len;
}

int t2gTextToStrings(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                     QSP_BOOL toGetQStrings, QSP_CHAR *outBuf, int outBufSize)
{
    int len;
    QSP_CHAR *strings = t2gExtractStrings(text, locStart, locEnd, toGetQStrings, &len);
    if (!strings) return -1;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, strings, len * sizeof(QSP_CHAR));
    free(strings);
    return len;
}

char *t2gQSPStringToUTF8(const QSP_CHAR *s, int len)
{
    return qspQSPStringToUTF8((QSP_CHAR *)s, len);
}

QSP_CHAR *t2gUTF8ToQSPString(const char *s, int len)
{
    return qspUTF8ToQSPString((char *)s, len);
}

void t2gFreeData(void *ptr)
{
    if (ptr) free(ptr);
}
