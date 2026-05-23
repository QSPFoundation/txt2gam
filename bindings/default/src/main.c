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
#include <string.h>

QSP_BOOL t2gInit();
void t2gTerminate();
int t2gTextToGame(const char *text, const char *locStart, const char *locEnd,
                  QSP_BOOL isOldFormat, QSP_BOOL isUCS2, const char *password, char *outBuf);
int t2gGameToText(const char *data, int dataLen, const char *password,
                  const char *locStart, const char *locEnd, char *outBuf);

static QSP_CHAR *qspStrFromUTF8(const char *utf8Str, QSP_CHAR *defaultStr);

static QSP_CHAR *qspStrFromUTF8(const char *utf8Str, QSP_CHAR *defaultStr)
{
    if (utf8Str)
        return qspUTF8ToQSPString((char *)utf8Str);
    return qspNewStr(defaultStr);
}

QSP_BOOL t2gInit()
{
    qspLocs = 0;
    qspLocsCount = 0;
    return qspInitLocProcessor();
}

void t2gTerminate()
{
    qspCreateWorld(0);
    qspLocs = 0;
    qspTerminateLocProcessor();
}

int t2gTextToGame(const char *text, const char *locStart, const char *locEnd,
                  QSP_BOOL isOldFormat, QSP_BOOL isUCS2, const char *password, char *outBuf)
{
    int locsCount, len;
    char *gameData;
    const char *textBuf;
    QSP_CHAR *textData, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!text) return -1;
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
    if (!textData) return -1;
    qspFormatLineEndings(textData);
    qspLocStart = qspStrFromUTF8(locStart, QSP_STARTLOC);
    if (!qspLocStart)
    {
        free(textData);
        return -1;
    }
    qspLocEnd = qspStrFromUTF8(locEnd, QSP_ENDLOC);
    if (!qspLocEnd)
    {
        free(textData);
        free(qspLocStart);
        return -1;
    }
    qspPassword = qspStrFromUTF8(password, QSP_PASSWD);
    if (!qspPassword)
    {
        free(textData);
        free(qspLocStart);
        free(qspLocEnd);
        return -1;
    }
    locsCount = qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_FALSE);
#ifdef _DEBUG
    qspPrint("Found %d locations\n", locsCount);
#endif
    qspCreateWorld(locsCount);
    qspOpenTextData(textData, qspLocStart, qspLocEnd, QSP_TRUE);
    free(textData);
    gameData = qspSaveQuest(isOldFormat, isUCS2, qspPassword, &len);
    free(qspLocStart);
    free(qspLocEnd);
    free(qspPassword);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!gameData) return -1;
    if (outBuf) memcpy(outBuf, gameData, len);
    free(gameData);
    return len;
}

int t2gGameToText(const char *data, int dataLen, const char *password,
                  const char *locStart, const char *locEnd, char *outBuf)
{
    int len;
    char *textData;
    QSP_CHAR *questText, *qspLocStart, *qspLocEnd, *qspPassword;
    if (!data || dataLen <= 0) return -1;
    qspPassword = qspStrFromUTF8(password, QSP_PASSWD);
    if (!qspPassword) return -1;
    if (!qspOpenQuest((char *)data, dataLen, qspPassword))
    {
        free(qspPassword);
        return -1;
    }
    free(qspPassword);
    qspLocStart = qspStrFromUTF8(locStart, QSP_STARTLOC);
    if (!qspLocStart)
    {
        qspCreateWorld(0);
        qspLocs = 0;
        return -1;
    }
    qspLocEnd = qspStrFromUTF8(locEnd, QSP_ENDLOC);
    if (!qspLocEnd)
    {
        qspCreateWorld(0);
        qspLocs = 0;
        free(qspLocStart);
        return -1;
    }
    questText = qspSaveQuestAsText(qspLocStart, qspLocEnd);
    free(qspLocStart);
    free(qspLocEnd);
    qspCreateWorld(0);
    qspLocs = 0;
    if (!questText) return -1;
    textData = qspQSPStringToUTF8(questText);
    free(questText);
    if (!textData) return -1;
    len = (int)strlen(textData) + 1;
    if (outBuf) memcpy(outBuf, textData, len);
    free(textData);
    return len;
}
