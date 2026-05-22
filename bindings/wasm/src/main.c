/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "main.h"
#include "coding.h"
#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "sys.h"

#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define T2G_WASM_API EMSCRIPTEN_KEEPALIVE
#else
    #define T2G_WASM_API
#endif

T2G_WASM_API QSP_BOOL t2gInit();
T2G_WASM_API void t2gTerminate();
T2G_WASM_API char *t2gTextToGame(const char *text, const char *locStart, const char *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, const char *password, int *outLen);
T2G_WASM_API char *t2gGameToText(const char *data, int dataLen, const char *password, const char *locStart, const char *locEnd);
T2G_WASM_API void t2gFreeStr(void *ptr);

static QSP_CHAR *qspStrFromUTF8(const char *utf8Str, QSP_CHAR *defaultStr);

static QSP_CHAR *qspStrFromUTF8(const char *utf8Str, QSP_CHAR *defaultStr)
{
    if (utf8Str)
        return qspUTF8ToQSPString(utf8Str);
    return qspNewStr(defaultStr);
}

T2G_WASM_API QSP_BOOL t2gInit()
{
    qspLocs = 0;
    qspLocsCount = 0;
    return qspInitLocProcessor();
}

T2G_WASM_API void t2gTerminate()
{
    qspCreateWorld(0);
    qspLocs = 0;
    qspTerminateLocProcessor();
}

T2G_WASM_API char *t2gTextToGame(const char *text, const char *locStart, const char *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, const char *password, int *outLen)
{
    int locsCount;
    char *gameData;
    const char *textBuf;
    QSP_CHAR *textData, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!text) return 0;
    /* Detect encoding from BOM */
    textBuf = text;
    if ((unsigned char)textBuf[0] == (unsigned char)TXT2GAM_UCS2BOM[0]
        && (unsigned char)textBuf[1] == (unsigned char)TXT2GAM_UCS2BOM[1])
    {
        textBuf += 2; /* skip UCS-2 BOM */
        textData = qspGameToQSPString((char *)textBuf, QSP_TRUE, QSP_FALSE);
    }
    else if ((unsigned char)textBuf[0] == (unsigned char)TXT2GAM_UTF8BOM[0]
        && (unsigned char)textBuf[1] == (unsigned char)TXT2GAM_UTF8BOM[1]
        && (unsigned char)textBuf[2] == (unsigned char)TXT2GAM_UTF8BOM[2])
    {
        textBuf += 3; /* skip UTF-8 BOM */
        textData = qspUTF8ToQSPString((char *)textBuf);
    }
    else
        textData = qspUTF8ToQSPString((char *)textBuf);
    if (!textData) return 0;
    qspFormatLineEndings(textData);
    qspLocStart = qspStrFromUTF8(locStart, QSP_STARTLOC);
    if (!qspLocStart)
    {
        free(textData);
        return 0;
    }
    qspLocEnd = qspStrFromUTF8(locEnd, QSP_ENDLOC);
    if (!qspLocEnd)
    {
        free(textData);
        free(qspLocStart);
        return 0;
    }
    qspPassword = qspStrFromUTF8(password, QSP_PASSWD);
    if (!qspPassword)
    {
        free(textData);
        free(qspLocStart);
        free(qspLocEnd);
        return 0;
    }
    locsCount = qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_FALSE);
#ifdef _DEBUG
    qspPrint("Found %d locations\n", locsCount);
#endif
    qspCreateWorld(locsCount);
    qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_TRUE);
    free(textData);
    gameData = qspSaveQuest(isOldFormat, isUCS2, qspPassword, outLen);
    free(qspLocStart);
    free(qspLocEnd);
    free(qspPassword);
    qspCreateWorld(0);
    qspLocs = 0;
    return gameData;
}

T2G_WASM_API char *t2gGameToText(const char *data, int dataLen, const char *password, const char *locStart, const char *locEnd)
{
    char *textData;
    QSP_CHAR *questText, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!data || dataLen <= 0) return 0;
    qspPassword = qspStrFromUTF8(password, QSP_PASSWD);
    if (!qspPassword) return 0;
    if (!qspOpenQuest((char *)data, dataLen, qspPassword))
    {
        free(qspPassword);
        return 0;
    }
    free(qspPassword);
    qspLocStart = qspStrFromUTF8(locStart, QSP_STARTLOC);
    if (!qspLocStart)
    {
        qspCreateWorld(0);
        qspLocs = 0;
        return 0;
    }
    qspLocEnd = qspStrFromUTF8(locEnd, QSP_ENDLOC);
    if (!qspLocEnd)
    {
        qspCreateWorld(0);
        qspLocs = 0;
        free(qspLocStart);
        return 0;
    }
    questText = qspSaveQuestAsText(qspLocStart, qspLocEnd);
    free(qspLocStart);
    free(qspLocEnd);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!questText) return 0;
    textData = qspQSPStringToUTF8(questText);
    free(questText);
    return textData;
}

T2G_WASM_API void t2gFreeStr(void *ptr)
{
    free(ptr);
}
