/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <string.h>
#include "t2g_api.h"
#include "coding.h"
#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "sys.h"

int t2gParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode, QSP_CHAR **outText, int *outLen)
{
    int encoding, len;
    const char *src;
    QSP_CHAR *result;
    if (!outText) return T2G_ERROR_FAILED;
    *outText = 0;
    if (!data || dataLen < 0) return T2G_ERROR_INVALID_DATA;
    src = data;
    len = dataLen;
    if (dataLen >= 2
        && (unsigned char)src[0] == (unsigned char)T2G_UTF16BOM[0]
        && (unsigned char)src[1] == (unsigned char)T2G_UTF16BOM[1])
    {
        src += 2; /* skip UTF-16 LE BOM */
        len -= 2;
        encoding = QSP_UTF16;
    }
    else if (dataLen >= 3
        && (unsigned char)src[0] == (unsigned char)T2G_UTF8BOM[0]
        && (unsigned char)src[1] == (unsigned char)T2G_UTF8BOM[1]
        && (unsigned char)src[2] == (unsigned char)T2G_UTF8BOM[2])
    {
        src += 3; /* skip UTF-8 BOM */
        len -= 3;
        encoding = QSP_UTF8;
    }
    else if (dataLen >= 2 && src[0] && !src[1])
        encoding = QSP_UTF16; /* no BOM but looks like UTF-16 LE */
    else
        encoding = isUnicode ? QSP_UTF8 : QSP_ANSI;
    switch (encoding)
    {
    case QSP_UTF16:
        result = qspGameToQSPString((char *)src, len / 2, QSP_TRUE, QSP_FALSE);
        break;
    case QSP_UTF8:
        result = qspUTF8ToQSPString((char *)src, len);
        break;
    default: /* QSP_ANSI */
        result = qspGameToQSPString((char *)src, len, QSP_FALSE, QSP_FALSE);
        break;
    }
    if (!result) return T2G_ERROR_FAILED;
    qspFormatLineEndings(result);
    if (outLen) *outLen = (int)qspStrLen(result) + 1;
    *outText = result;
    return T2G_ERROR_NONE;
}

int t2gEncodeTextData(const QSP_CHAR *text, QSP_BOOL isUnicode, char **outData, int *outLen)
{
    char *encoded, *result;
    int encodedLen, bomLen;
    if (!outData) return T2G_ERROR_FAILED;
    *outData = 0;
    if (!text) return T2G_ERROR_INVALID_DATA;
    if (isUnicode)
    {
        encoded = qspQSPStringToUTF8((QSP_CHAR *)text, -1);
        bomLen = sizeof(T2G_UTF8BOM) - 1;
    }
    else
    {
        encoded = qspQSPToGameString((QSP_CHAR *)text, -1, QSP_FALSE, QSP_FALSE);
        bomLen = 0;
    }
    if (!encoded) return T2G_ERROR_FAILED;
    encodedLen = (int)strlen(encoded);
    result = (char *)malloc(bomLen + encodedLen + 1);
    if (!result)
    {
        free(encoded);
        return T2G_ERROR_NO_MEMORY;
    }
    if (bomLen) memcpy(result, T2G_UTF8BOM, bomLen);
    memcpy(result + bomLen, encoded, encodedLen + 1);
    free(encoded);
    if (outLen) *outLen = bomLen + encodedLen;
    *outData = result;
    return T2G_ERROR_NONE;
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

int t2gEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                        QSP_BOOL isOldFormat, QSP_BOOL isUnicode,
                        const QSP_CHAR *password, char **outData, int *outLen)
{
    int len;
    char *gameData;
    QSP_CHAR *qspLocStart, *qspLocEnd, *qspPassword;
    QSP_CHAR *textData, *textNorm = 0;
    if (!outData) return T2G_ERROR_FAILED;
    *outData = 0;
    if (!text) return T2G_ERROR_INVALID_DATA;
    textData = (QSP_CHAR *)text;
    if (qspStrChr(textData, QSP_OLDNEWLINE))
    {
        textNorm = qspNewStr(textData);
        qspFormatLineEndings(textNorm);
        textData = textNorm;
    }
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    qspPassword = password ? qspNewStr((QSP_CHAR *)password) : qspNewStr(QSP_PASSWD);
    qspCreateWorld(0);
    qspOpenTextData(textData, qspLocStart, qspLocEnd);
    gameData = qspSaveQuest(isOldFormat, isUnicode, qspPassword, &len);
    if (textNorm) free(textNorm);
    free(qspLocStart);
    free(qspLocEnd);
    free(qspPassword);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!gameData) return T2G_ERROR_FAILED;
    if (outLen) *outLen = len;
    *outData = gameData;
    return T2G_ERROR_NONE;
}

int t2gDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                        const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                        QSP_CHAR **outText, int *outLen)
{
    int err;
    QSP_CHAR *questText, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!outText) return T2G_ERROR_FAILED;
    *outText = 0;
    if (!data || dataLen <= 0) return T2G_ERROR_INVALID_DATA;
    qspPassword = password ? qspNewStr((QSP_CHAR *)password) : qspNewStr(QSP_PASSWD);
    err = qspOpenQuest((char *)data, dataLen, qspPassword);
    free(qspPassword);
    if (err != T2G_ERROR_NONE) return err;
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    questText = qspSaveQuestAsText(qspLocStart, qspLocEnd);
    free(qspLocStart);
    free(qspLocEnd);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!questText) return T2G_ERROR_FAILED;
    if (outLen) *outLen = (int)qspStrLen(questText) + 1;
    *outText = questText;
    return T2G_ERROR_NONE;
}

int t2gExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                      const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings,
                      QSP_CHAR **outText, int *outLen)
{
    QSP_CHAR *textData, *qspLocStart, *qspLocEnd, *result, *textNorm = 0;
    if (!outText) return T2G_ERROR_FAILED;
    *outText = 0;
    if (!text) return T2G_ERROR_INVALID_DATA;
    textData = (QSP_CHAR *)text;
    if (qspStrChr(textData, QSP_OLDNEWLINE))
    {
        textNorm = qspNewStr(textData);
        qspFormatLineEndings(textNorm);
        textData = textNorm;
    }
    qspLocStart = locStart ? qspNewStr((QSP_CHAR *)locStart) : qspNewStr(T2G_STARTLOC);
    qspLocEnd   = locEnd   ? qspNewStr((QSP_CHAR *)locEnd)   : qspNewStr(T2G_ENDLOC);
    result = qspGetLocsStrings(textData, qspLocStart, qspLocEnd, toGetQStrings);
    if (textNorm) free(textNorm);
    free(qspLocStart);
    free(qspLocEnd);
    /* qspGetLocsStrings returns 0 when no strings are found; expose that as an
     * empty (but allocated) string so 0 strictly means an error. */
    if (!result) result = qspNewStr(QSP_FMT(""));
    if (!result) return T2G_ERROR_NO_MEMORY;
    if (outLen) *outLen = (int)qspStrLen(result) + 1;
    *outText = result;
    return T2G_ERROR_NONE;
}
