/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "coding.h"
#include "sys.h"

QSPLocation *qspLocs = 0;
int qspLocsCount = 0;

static QSP_BOOL qspCheckQuest(QSPGameSeg *strs, int count, QSP_BOOL isUnicode, QSP_CHAR *password);
static void qspInitLocation(QSPLocation *loc);

static void qspInitLocation(QSPLocation *loc)
{
    memset(loc, 0, sizeof(QSPLocation));
}

static QSP_CHAR *qspSegToStr(QSPGameSeg seg, QSP_BOOL isUnicode, QSP_BOOL isCoded)
{
    return qspGameToQSPString(seg.Str, seg.Len, isUnicode, isCoded);
}

static QSP_BOOL qspCheckQuest(QSPGameSeg *strs, int count, QSP_BOOL isUnicode, QSP_CHAR *password)
{
    int i, ind, locsCount, actsCount;
    QSP_BOOL isOldFormat, hasInvalidPassword;
    QSP_CHAR *buf = qspSegToStr(strs[0], isUnicode, QSP_FALSE);
    isOldFormat = qspStrsComp(buf, QSP_GAMEID) != 0;
    free(buf);
    ind = (isOldFormat ? 30 : 4);
    if (ind > count) return QSP_FALSE;
    buf = (isOldFormat ? qspSegToStr(strs[1], isUnicode, QSP_TRUE) : qspSegToStr(strs[2], isUnicode, QSP_TRUE));
    hasInvalidPassword = qspStrsComp(buf, password);
    free(buf);
#ifdef SPEC_PASS
    hasInvalidPassword = hasInvalidPassword && qspStrsComp(QSP_FMT(SPEC_PASS), password);
#endif
    if (hasInvalidPassword) return QSP_FALSE;
    buf = (isOldFormat ? qspSegToStr(strs[0], isUnicode, QSP_FALSE) : qspSegToStr(strs[3], isUnicode, QSP_TRUE));
    locsCount = qspStrToNum(buf, 0);
    free(buf);
    if (locsCount <= 0) return QSP_FALSE;
    for (i = 0; i < locsCount; ++i)
    {
        if ((ind += 3) > count) return QSP_FALSE;
        if (isOldFormat)
            actsCount = 20;
        else
        {
            if (ind + 1 > count) return QSP_FALSE;
            buf = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
            actsCount = qspStrToNum(buf, 0);
            free(buf);
            if (actsCount < 0 || actsCount > QSP_MAXACTIONS) return QSP_FALSE;
        }
        if ((ind += (actsCount * (isOldFormat ? 2 : 3))) > count) return QSP_FALSE;
    }
    return QSP_TRUE;
}

void qspCreateWorld(int locsCount)
{
    int i, j;
    for (i = 0; i < qspLocsCount; ++i)
    {
        if (qspLocs[i].Name) free(qspLocs[i].Name);
        if (qspLocs[i].Desc) free(qspLocs[i].Desc);
        if (qspLocs[i].OnVisit) free(qspLocs[i].OnVisit);
        for (j = 0; j < QSP_MAXACTIONS; ++j)
        {
            if (qspLocs[i].Actions[j].Desc) free(qspLocs[i].Actions[j].Desc);
            if (qspLocs[i].Actions[j].Image) free(qspLocs[i].Actions[j].Image);
            if (qspLocs[i].Actions[j].Code) free(qspLocs[i].Actions[j].Code);
        }
    }
    if (qspLocsCount != locsCount)
    {
        qspLocsCount = locsCount;
        if (qspLocsCount)
            qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
        else
        {
            free(qspLocs);
            qspLocs = 0;
        }
    }
    for (i = 0; i < qspLocsCount; ++i)
        qspInitLocation(qspLocs + i);
}

QSP_CHAR *qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings)
{
    QSP_CHAR *name, *curStr, *pos, *res = 0, quot = 0;
    int locStartLen, locEndLen, curBufSize = 1024, curStrLen = 0, resLen = 0, quotsCount = 0, strsCount = 0, locsCount = 0;
    QSP_BOOL isInLoc = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    curStr = qspAllocateBuffer(curBufSize);
    while (*data)
    {
        if (isInLoc)
        {
            if (!quot && !quotsCount && *data == QSP_NEWLINE)
            {
                if (qspIsEqual(data + 1, locEnd, locEndLen))
                {
                    isInLoc = QSP_FALSE;
                    pos = qspStrChr(data + 1 + locEndLen, QSP_NEWLINE);
                    if (pos)
                    {
                        data = pos + 1;
                        continue; /* we're searching for a new loc now */
                    }
                    else
                        break;
                }
            }
            if (quot)
            {
                if (*data == quot)
                {
                    if (*(data + 1) == quot)
                    {
                        ++data;
                        if (toGetQStrings && quotsCount) /* keep escaped quotes inside q-strings */
                            curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
                    }
                    else
                        quot = 0;
                }
            }
            else
            {
                if (*data == QSP_LQUOT)
                    ++quotsCount;
                else if (*data == QSP_RQUOT)
                {
                    if (quotsCount) --quotsCount;
                }
                else if (qspIsInList(QSP_QUOTS, *data))
                    quot = *data;
            }
            if (toGetQStrings ? quotsCount : quot) /* we're inside quotes */
            {
                if (*data == QSP_NEWLINE)
                    curStrLen = qspAddTextToBuffer(&curStr, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), curStrLen, &curBufSize);
                else
                    curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
            }
            else if (curStrLen)
            {
                curStr[curStrLen] = 0;
                resLen = qspAddText(&res, curStr + 1, resLen, curStrLen - 1, QSP_FALSE); /* ignore initial quote */
                resLen = qspAddText(&res, QSP_STRSDELIM, resLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                curStrLen = 0;
                ++strsCount;
            }
            ++data;
        }
        else
        {
            pos = qspStrChr(data, QSP_NEWLINE);
            if (qspIsEqual(data, locStart, locStartLen))
            {
                ++locsCount;
                isInLoc = QSP_TRUE;
                data += locStartLen;
                if (pos)
                {
                    *pos = 0;
                    name = qspDelSpc(data);
                    *pos = QSP_NEWLINE;
                }
                else
                    name = qspDelSpc(data);
                if (resLen)
                {
                    /* Add an empty line to separate locations */
                    resLen = qspAddText(&res, QSP_STRSDELIM, resLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                }
                resLen = qspAddText(&res, QSP_FMT("Location: "), resLen, -1, QSP_FALSE);
                resLen = qspAddText(&res, name, resLen, -1, QSP_FALSE);
                free(name);
                resLen = qspAddText(&res, QSP_STRSDELIM, resLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                if (pos)
                {
                    data = pos;
                    continue; /* we're inside the loc now */
                }
            }
            if (pos)
                data = pos + 1; /* keep searching for a new loc */
            else
                break;
        }
    }
    free(curStr);
    t2gPrint("Extracted %d strings from %d locations\n", strsCount, locsCount);
    return res;
}

int qspOpenTextData(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    QSP_CHAR *locCode, *pos, quot = 0;
    int locStartLen, locEndLen, locBufSize = 1024, locCodeLen = 0, quotsCount = 0, curLoc = 0, locCapacity = qspLocsCount;
    QSP_BOOL isNewLine = QSP_TRUE, isInLoc = QSP_FALSE, isInBaseSection = QSP_FALSE, isInBaseAction = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    locCode = qspAllocateBuffer(locBufSize);
    while (*data)
    {
        if (isInLoc)
        {
            if (!quot && !quotsCount && isNewLine)
            {
                QSP_BOOL toSkipLine = QSP_FALSE;
                if (qspIsEqual(data, locEnd, locEndLen))
                {
                    data += locEndLen;
                    toSkipLine = QSP_TRUE; /* we're searching for a new loc now */

                    if (locCodeLen > QSP_LEN(QSP_STRSDELIM)) /* we have to remove the last line separator */
                    {
                        if (!isInBaseSection)
                        {
                            locCode[locCodeLen - QSP_LEN(QSP_STRSDELIM)] = 0;
                            qspUpdateLocationCode(locCode, qspLocs + curLoc);
                        }
                    }

                    locCodeLen = 0; /* reuse the code buffer */
                    isInLoc = QSP_FALSE;
                    isInBaseSection = QSP_FALSE;
                    isInBaseAction = QSP_FALSE;

                    ++curLoc;
                }
                else if (isInBaseSection)
                {
                    if (isInBaseAction)
                    {
                        if (qspIsEqual(data, QSP_BASEACTS_ACT_FOOTER, QSP_LEN(QSP_BASEACTS_ACT_FOOTER)))
                        {
                            data += QSP_LEN(QSP_BASEACTS_ACT_FOOTER);
                            toSkipLine = QSP_TRUE;
                            /* Update action code */
                            if (locCodeLen > QSP_LEN(QSP_STRSDELIM)) /* we have to remove the last line separator */
                            {
                                locCode[locCodeLen - QSP_LEN(QSP_STRSDELIM)] = 0;
                                qspUpdateActionCode(locCode, qspLocs + curLoc);
                            }
                            locCodeLen = 0; /* reuse the code buffer */
                            isInBaseAction = QSP_FALSE;
                        }
                    }
                    else if (qspIsEqual(data, QSP_BASESECTION_FOOTER, QSP_LEN(QSP_BASESECTION_FOOTER)))
                    {
                        data += QSP_LEN(QSP_BASESECTION_FOOTER);
                        toSkipLine = QSP_TRUE;

                        locCodeLen = 0; /* reuse the code buffer */
                        isInBaseSection = QSP_FALSE;
                    }
                    else if (qspIsEqual(data, QSP_BASEACTS_ACT_HEADER, QSP_LEN(QSP_BASEACTS_ACT_HEADER)))
                    {
                        data += QSP_LEN(QSP_BASEACTS_ACT_HEADER);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add an action */
                        qspParseBaseAction(&data, qspLocs + curLoc);

                        locCodeLen = 0; /* reuse the code buffer */
                        isInBaseAction = QSP_TRUE;
                    }
                    else if (qspIsInList(QSP_QUOTS, *data))
                    {
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        qspParseBaseDescriptionPrintLine(&data, qspLocs + curLoc);
                    }
                    else if (qspIsEqual(data, QSP_BASEDESC_PRINTLINE, QSP_LEN(QSP_BASEDESC_PRINTLINE)))
                    {
                        data += QSP_LEN(QSP_BASEDESC_PRINTLINE);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        qspParseBaseDescriptionPrintLine(&data, qspLocs + curLoc);
                    }
                    else if (qspIsEqual(data, QSP_BASEDESC_PRINT, QSP_LEN(QSP_BASEDESC_PRINT)))
                    {
                        data += QSP_LEN(QSP_BASEDESC_PRINT);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        qspParseBaseDescriptionPrint(&data, qspLocs + curLoc);
                    }
                }
                else if (qspIsEqual(data, QSP_BASESECTION_HEADER, QSP_LEN(QSP_BASESECTION_HEADER)))
                {
                    data += QSP_LEN(QSP_BASESECTION_HEADER);
                    toSkipLine = QSP_TRUE;

                    isInBaseSection = QSP_TRUE;
                    isInBaseAction = QSP_FALSE;
                }
                if (toSkipLine)
                {
                    pos = qspStrChr(data, QSP_NEWLINE);
                    if (pos)
                    {
                        data = pos + 1;
                        isNewLine = QSP_TRUE;
                        continue;
                    }
                    else
                        break;
                }
            }
            if (*data == QSP_NEWLINE)
            {
                locCodeLen = qspAddTextToBuffer(&locCode, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), locCodeLen, &locBufSize);
                isNewLine = QSP_TRUE;
            }
            else
            {
                locCodeLen = qspAddCharToBuffer(&locCode, *data, locCodeLen, &locBufSize);
                isNewLine = QSP_FALSE;
            }
            if (quot)
            {
                if (*data == quot)
                {
                    if (*(data + 1) == quot)
                    {
                        locCodeLen = qspAddCharToBuffer(&locCode, *data, locCodeLen, &locBufSize);
                        ++data;
                    }
                    else
                        quot = 0;
                }
            }
            else
            {
                if (*data == QSP_LQUOT)
                    ++quotsCount;
                else if (*data == QSP_RQUOT)
                {
                    if (quotsCount) --quotsCount;
                }
                else if (qspIsInList(QSP_QUOTS, *data))
                    quot = *data;
            }
            ++data;
        }
        else
        {
            /* We're not in any location */
            pos = qspStrChr(data, QSP_NEWLINE);
            if (qspIsEqual(data, locStart, locStartLen))
            {
                if (curLoc >= locCapacity)
                {
                    locCapacity += 32;
                    qspLocs = (QSPLocation *)realloc(qspLocs, locCapacity * sizeof(QSPLocation));
                }
                qspInitLocation(qspLocs + curLoc);
                qspLocsCount = curLoc + 1;
                data += locStartLen;
                if (pos)
                {
                    *pos = 0;
                    qspLocs[curLoc].Name = qspDelSpc(data);
                    *pos = QSP_NEWLINE;
                }
                else
                    qspLocs[curLoc].Name = qspDelSpc(data);

                t2gPrint("Location: %s\n", qspLocs[curLoc].Name);
                locCodeLen = 0; /* reuse the code buffer */
                isInLoc = QSP_TRUE;
                isInBaseSection = QSP_FALSE;
                isInBaseAction = QSP_FALSE;
            }
            isNewLine = QSP_TRUE;
            if (pos)
                data = pos + 1;
            else
                break;
        }
    }
    if (isInLoc)
    {
        /* The last location */
        if (locCodeLen)
        {
            locCode[locCodeLen] = 0;
            if (isInBaseAction)
                qspUpdateActionCode(locCode, qspLocs + curLoc);
            else if (!isInBaseSection)
                qspUpdateLocationCode(locCode, qspLocs + curLoc);
        }
        ++curLoc;
    }

    free(locCode);
    if (locCapacity > qspLocsCount)
        qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
    t2gPrint("%d locations were loaded\n", curLoc);

    return curLoc;
}

char *qspSaveQuest(QSP_BOOL isOldFormat, QSP_BOOL isUnicode, QSP_CHAR *passwd, int *dataLen)
{
    QSP_CHAR *verInfo;
    char narrowInfo[QSP_VERINFOSIZE], dateBuf[QSP_DATESTRSIZE];
    int i, j, len, actsCount;
    time_t currentTime;
    struct tm *localTime;
    char *buf = 0;

    time(&currentTime);
    localTime = localtime(&currentTime);
    strftime(dateBuf, QSP_DATESTRSIZE, "%Y-%m-%d", localTime);
    snprintf(narrowInfo, QSP_VERINFOSIZE, "%s (%s %s)", dateBuf, QSP_APPNAME, TXT2GAM_VER_STR);
    verInfo = qspUTF8ToQSPString(narrowInfo, -1);

    if (isOldFormat)
    {
        len = qspGameCodeWriteIntValLine(&buf, 0, qspLocsCount, isUnicode, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, passwd, isUnicode, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, verInfo, isUnicode, QSP_FALSE);
        for (i = 0; i < 27; ++i) len = qspGameCodeWriteValLine(&buf, len, 0, isUnicode, QSP_FALSE);
    }
    else
    {
        len = qspGameCodeWriteValLine(&buf, 0, QSP_GAMEID, isUnicode, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, verInfo, isUnicode, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, passwd, isUnicode, QSP_TRUE);
        len = qspGameCodeWriteIntValLine(&buf, len, qspLocsCount, isUnicode, QSP_TRUE);
    }
    for (i = 0; i < qspLocsCount; ++i)
    {
        t2gPrint("Saving location: %s\n", qspLocs[i].Name);

        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Name, isUnicode, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Desc, isUnicode, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].OnVisit, isUnicode, QSP_TRUE);

        if (isOldFormat)
        {
            for (j = 0; j < 20; ++j)
            {
                if (j < qspLocs[i].ActionsCount)
                {
                    len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Desc, isUnicode, QSP_TRUE);
                    len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Code, isUnicode, QSP_TRUE);
                }
                else
                {
                    len = qspGameCodeWriteValLine(&buf, len, 0, isUnicode, QSP_TRUE);
                    len = qspGameCodeWriteValLine(&buf, len, 0, isUnicode, QSP_TRUE);
                }
            }
        }
        else
        {
            actsCount = 0;
            for (j = 0; j < qspLocs[i].ActionsCount; ++j)
            {
                if (!qspLocs[i].Actions[j].Desc)
                    break;
                if (qspIsEmpty(qspLocs[i].Actions[j].Desc))
                    break;
                ++actsCount;
            }
            len = qspGameCodeWriteIntValLine(&buf, len, actsCount, isUnicode, QSP_TRUE);
            for (j = 0; j < actsCount; ++j)
            {
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Image, isUnicode, QSP_TRUE);
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Desc, isUnicode, QSP_TRUE);
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Code, isUnicode, QSP_TRUE);
            }
        }
    }

    t2gPrint("%d locations were saved\n", qspLocsCount);

    free(verInfo);
    *dataLen = isUnicode ? len * 2 : len;
    return buf;
}

QSP_BOOL qspOpenQuest(char *data, int dataSize, QSP_CHAR *password)
{
    QSP_BOOL isOldFormat, isUnicode;
    int i, j, ind, count, locsCount, actsCount;
    QSP_CHAR *buf;
    QSPGameSeg *strs;
    /* Parse the data */
    if (dataSize < 2) return QSP_FALSE;
    count = qspSplitGameData(data, dataSize, isUnicode = !data[1], &strs);
    if (!qspCheckQuest(strs, count, isUnicode, password))
    {
        free(strs);
        return QSP_FALSE;
    }
    buf = qspSegToStr(strs[0], isUnicode, QSP_FALSE);
    isOldFormat = qspStrsComp(buf, QSP_GAMEID) != 0;
    free(buf);
    buf = (isOldFormat ? qspSegToStr(strs[0], isUnicode, QSP_FALSE) : qspSegToStr(strs[3], isUnicode, QSP_TRUE));
    locsCount = qspStrToNum(buf, 0);
    free(buf);
    qspCreateWorld(locsCount);
    ind = (isOldFormat ? 30 : 4);
    for (i = 0; i < locsCount; ++i)
    {
        qspLocs[i].Name = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
        qspLocs[i].Desc = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
        qspLocs[i].OnVisit = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);

        t2gPrint("Location: %s\n", qspLocs[i].Name);

        if (isOldFormat)
            actsCount = 20;
        else
        {
            buf = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
            actsCount = qspStrToNum(buf, 0);
            free(buf);
        }
        qspLocs[i].ActionsCount = actsCount;
        for (j = 0; j < actsCount; ++j)
        {
            qspLocs[i].Actions[j].Image = (isOldFormat ? 0 : qspSegToStr(strs[ind++], isUnicode, QSP_TRUE));
            qspLocs[i].Actions[j].Desc = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
            qspLocs[i].Actions[j].Code = qspSegToStr(strs[ind++], isUnicode, QSP_TRUE);
        }
    }

    t2gPrint("%d locations were loaded\n", locsCount);

    free(strs);
    return QSP_TRUE;
}

QSP_CHAR *qspSaveQuestAsText(QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    QSP_BOOL hasBaseDesc;
    int i, j, k, baseActsCount, linesCount, len = 0, bufSize = 4096;
    QSP_CHAR **lines;
    QSP_CHAR *temp, *buf = qspAllocateBuffer(bufSize);
    for (i = 0; i < qspLocsCount; ++i)
    {
        t2gPrint("Saving location: %s\n", qspLocs[i].Name);

        /* Write location header */
        len = qspAddTextToBuffer(&buf, locStart, -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, QSP_FMT(" "), -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, qspLocs[i].Name, -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);

        baseActsCount = 0;
        for (j = 0; j < qspLocs[i].ActionsCount; ++j)
        {
            if (!qspLocs[i].Actions[j].Desc)
                break;
            if (qspIsEmpty(qspLocs[i].Actions[j].Desc))
                break;
            ++baseActsCount;
        }
        hasBaseDesc = (qspLocs[i].Desc && !qspIsEmpty(qspLocs[i].Desc));

        if (hasBaseDesc || baseActsCount)
        {
            /* Add base section header */
            len = qspAddTextToBuffer(&buf, QSP_BASESECTION_HEADER, QSP_LEN(QSP_BASESECTION_HEADER), len, &bufSize);
            len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);

            /* Write base description of the location */
            if (hasBaseDesc)
            {
                temp = qspReplaceText(qspLocs[i].Desc, QSP_FMT("'"), QSP_FMT("''"));
                len = qspAddTextToBuffer(&buf, QSP_BASEDESC_PRINT QSP_FMT(" '"), -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, temp, -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, QSP_FMT("'"), -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
                free(temp);
            }

            /* Write base actions */
            for (j = 0; j < baseActsCount; ++j)
            {
                /* Write action header */
                temp = qspReplaceText(qspLocs[i].Actions[j].Desc, QSP_FMT("'"), QSP_FMT("''"));
                len = qspAddTextToBuffer(&buf, QSP_BASEACTS_ACT_HEADER QSP_FMT(" '"), -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, temp, -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, QSP_FMT("'"), -1, len, &bufSize);
                free(temp);

                if (qspLocs[i].Actions[j].Image && !qspIsEmpty(qspLocs[i].Actions[j].Image))
                {
                    temp = qspReplaceText(qspLocs[i].Actions[j].Image, QSP_FMT("'"), QSP_FMT("''"));
                    len = qspAddTextToBuffer(&buf, QSP_FMT(", '"), -1, len, &bufSize);
                    len = qspAddTextToBuffer(&buf, temp, -1, len, &bufSize);
                    len = qspAddTextToBuffer(&buf, QSP_FMT("'"), -1, len, &bufSize);
                    free(temp);
                }
                len = qspAddTextToBuffer(&buf, QSP_FMT(":"), -1, len, &bufSize);
                len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);

                /* Write action code */
                if (qspLocs[i].Actions[j].Code && !qspIsEmpty(qspLocs[i].Actions[j].Code))
                {
                    linesCount = qspSplitStr(qspLocs[i].Actions[j].Code, QSP_STRSDELIM, &lines);
                    for (k = 0; k < linesCount; ++k)
                    {
                        len = qspAddTextToBuffer(&buf, QSP_BASEACTS_LINE_PREFIX, QSP_LEN(QSP_BASEACTS_LINE_PREFIX), len, &bufSize);
                        len = qspAddTextToBuffer(&buf, lines[k], -1, len, &bufSize);
                        len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
                    }
                    qspFreeStrs((void **)lines, linesCount);
                }

                /* Write action footer */
                len = qspAddTextToBuffer(&buf, QSP_BASEACTS_ACT_FOOTER, QSP_LEN(QSP_BASEACTS_ACT_FOOTER), len, &bufSize);
                len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
            }

            /* Add base section footer */
            len = qspAddTextToBuffer(&buf, QSP_BASESECTION_FOOTER, QSP_LEN(QSP_BASESECTION_FOOTER), len, &bufSize);
            len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
        }

        /* Write location code */
        if (qspLocs[i].OnVisit && !qspIsEmpty(qspLocs[i].OnVisit))
        {
            len = qspAddTextToBuffer(&buf, qspLocs[i].OnVisit, -1, len, &bufSize);
            len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
        }

        /* Write location footer */
        len = qspAddTextToBuffer(&buf, locEnd, -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, QSP_FMT(" "), -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, qspLocs[i].Name, -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, QSP_FMT(" ---------------------------------") QSP_STRSDELIM, -1, len, &bufSize);
        len = qspAddTextToBuffer(&buf, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), len, &bufSize);
    }

    t2gPrint("%d locations were saved, total length: %d characters\n", qspLocsCount, len);

    buf[len] = 0;
    return (QSP_CHAR *)realloc(buf, (len + 1) * sizeof(QSP_CHAR));
}
