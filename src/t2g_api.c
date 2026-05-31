/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "t2g_api.h"
#include "coding.h"
#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "sys.h"
#include <string.h>

QSP_CHAR *t2gParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode)
{
    int encoding;
    char *buf;
    const char *src;
    QSP_CHAR *result;
    if (!data || dataLen < 0) return 0;
    /* Allocate a copy with 4 null bytes appended so wide-char functions terminate safely. */
    buf = (char *)malloc(dataLen + 4);
    if (!buf) return 0;
    memcpy(buf, data, dataLen);
    buf[dataLen] = buf[dataLen + 1] = buf[dataLen + 2] = buf[dataLen + 3] = 0;
    src = buf;
    if ((unsigned char)src[0] == (unsigned char)T2G_UTF16BOM[0]
        && (unsigned char)src[1] == (unsigned char)T2G_UTF16BOM[1])
    {
        src += 2; /* skip UTF-16 LE BOM */
        encoding = QSP_UTF16;
    }
    else if ((unsigned char)src[0] == (unsigned char)T2G_UTF8BOM[0]
        && (unsigned char)src[1] == (unsigned char)T2G_UTF8BOM[1]
        && (unsigned char)src[2] == (unsigned char)T2G_UTF8BOM[2])
    {
        src += 3; /* skip UTF-8 BOM */
        encoding = QSP_UTF8;
    }
    else if (src[0] && !src[1])
        encoding = QSP_UTF16; /* no BOM but looks like UTF-16 LE */
    else
        encoding = isUnicode ? QSP_UTF8 : QSP_ANSI;
    switch (encoding)
    {
    case QSP_UTF16:
        result = qspGameToQSPString((char *)src, QSP_TRUE, QSP_FALSE);
        break;
    case QSP_UTF8:
        result = qspUTF8ToQSPString((char *)src);
        break;
    default: /* QSP_ANSI */
        result = qspGameToQSPString((char *)src, QSP_FALSE, QSP_FALSE);
        break;
    }
    free(buf);
    if (result) qspFormatLineEndings(result);
    return result;
}

int t2gInit(void)
{
    qspLocs = 0;
    qspLocsCount = 0;
    return qspInitLocProcessor();
}

void t2gTerminate(void)
{
    qspCreateWorld(0);
    qspLocs = 0;
    qspTerminateLocProcessor();
}

char *t2gEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                          QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password, int *outLen)
{
    int locsCount, len;
    char *gameData;
    QSP_CHAR *textData, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!text || !outLen) return 0;
    textData = qspNewStr((QSP_CHAR *)text);
    if (!textData) return 0;
    qspFormatLineEndings(textData);
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    qspPassword = password ? qspNewStr((QSP_CHAR *)password) : qspNewStr(QSP_PASSWD);
    locsCount = qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_FALSE);
#ifdef _DEBUG
    qspPrint("Found %d locations\n", locsCount);
#endif
    qspCreateWorld(locsCount);
    qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_TRUE);
    free(textData);
    gameData = qspSaveQuest(isOldFormat, isUnicode, qspPassword, &len);
    free(qspLocStart);
    free(qspLocEnd);
    free(qspPassword);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!gameData) return 0;
    *outLen = len;
    return gameData;
}

QSP_CHAR *t2gDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                              const QSP_CHAR *locStart, const QSP_CHAR *locEnd, int *outLen)
{
    int len;
    char *dataCopy;
    QSP_CHAR *questText, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!data || dataLen <= 0 || !outLen) return 0;
    dataCopy = (char *)malloc(dataLen + 2);
    if (!dataCopy) return 0;
    memcpy(dataCopy, data, dataLen);
    dataCopy[dataLen] = dataCopy[dataLen + 1] = 0;
    qspPassword = password ? qspNewStr((QSP_CHAR *)password) : qspNewStr(QSP_PASSWD);
    if (!qspOpenQuest(dataCopy, dataLen, qspPassword))
    {
        free(qspPassword);
        free(dataCopy);
        return 0;
    }
    free(qspPassword);
    free(dataCopy);
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    questText = qspSaveQuestAsText(qspLocStart, qspLocEnd);
    free(qspLocStart);
    free(qspLocEnd);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!questText) return 0;
    len = (int)qspStrLen(questText) + 1;
    *outLen = len;
    return questText;
}

QSP_CHAR *t2gExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                           const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, int *outLen)
{
    QSP_CHAR *textData, *qspLocStart, *qspLocEnd, *result;
    if (!text || !outLen) return 0;
    textData = qspNewStr((QSP_CHAR *)text);
    if (!textData) return 0;
    qspFormatLineEndings(textData);
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    result = qspGetLocsStrings(textData, qspLocStart, qspLocEnd, toGetQStrings);
    free(textData);
    free(qspLocStart);
    free(qspLocEnd);
    /* qspGetLocsStrings returns 0 when no strings are found; expose that as an
     * empty (but allocated) string so 0 strictly means an error. */
    if (!result) result = qspNewStr(QSP_FMT(""));
    if (!result) return 0;
    *outLen = (int)qspStrLen(result) + 1;
    return result;
}
