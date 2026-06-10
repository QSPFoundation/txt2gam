/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <string.h>
#include "t2g_default.h"
#include "t2g_api.h"
#include "coding.h"

int t2gReadTextData(const char *data, int dataLen, QSP_BOOL isUnicode, QSP_CHAR *outBuf, int outBufSize, int *outSize)
{
    int len = 0;
    QSP_CHAR *result = 0;
    int err = t2gParseTextData(data, dataLen, isUnicode, &result, &len);
    if (err != T2G_ERROR_NONE) return err;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, result, len * sizeof(QSP_CHAR));
    free(result);
    if (outSize) *outSize = len;
    return T2G_ERROR_NONE;
}

int t2gWriteTextData(const QSP_CHAR *text, QSP_BOOL isUnicode, char *outBuf, int outBufSize, int *outSize)
{
    int len = 0;
    char *result = 0;
    int err = t2gEncodeTextData(text, isUnicode, &result, &len);
    if (err != T2G_ERROR_NONE) return err;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, result, len);
    free(result);
    if (outSize) *outSize = len;
    return T2G_ERROR_NONE;
}

int t2gTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                  QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password,
                  char *outBuf, int outBufSize, int *outSize)
{
    int len = 0;
    char *gameData = 0;
    int err = t2gEncodeTextToGame(text, locStart, locEnd, isOldFormat, isUnicode, password, &gameData, &len);
    if (err != T2G_ERROR_NONE) return err;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, gameData, len);
    free(gameData);
    if (outSize) *outSize = len;
    return T2G_ERROR_NONE;
}

int t2gGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                  const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                  QSP_CHAR *outBuf, int outBufSize, int *outSize)
{
    int len = 0;
    QSP_CHAR *textData = 0;
    int err = t2gDecodeGameToText(data, dataLen, password, locStart, locEnd, &textData, &len);
    if (err != T2G_ERROR_NONE) return err;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, textData, len * sizeof(QSP_CHAR));
    free(textData);
    if (outSize) *outSize = len;
    return T2G_ERROR_NONE;
}

int t2gTextToStrings(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                     QSP_BOOL toGetQStrings, QSP_CHAR *outBuf, int outBufSize, int *outSize)
{
    int len = 0;
    QSP_CHAR *strings = 0;
    int err = t2gExtractStrings(text, locStart, locEnd, toGetQStrings, &strings, &len);
    if (err != T2G_ERROR_NONE) return err;
    if (outBuf && outBufSize >= len)
        memcpy(outBuf, strings, len * sizeof(QSP_CHAR));
    free(strings);
    if (outSize) *outSize = len;
    return T2G_ERROR_NONE;
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
