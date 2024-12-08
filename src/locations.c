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

#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "coding.h"
#include "sys.h"

QSPLocation *qspLocs = 0;
int qspLocsCount = 0;

static QSP_BOOL qspCheckQuest(char **strs, int count, QSP_BOOL isUCS2, QSP_CHAR *password);

static QSP_BOOL qspCheckQuest(char **strs, int count, QSP_BOOL isUCS2, QSP_CHAR *password)
{
    int i, ind, locsCount, actsCount;
    QSP_BOOL isOldFormat, hasInvalidPassword;
    QSP_CHAR *buf = qspGameToQSPString(strs[0], isUCS2, QSP_FALSE);
    isOldFormat = qspStrsComp(buf, QSP_GAMEID) != 0;
    free(buf);
    ind = (isOldFormat ? 30 : 4);
    if (ind > count) return QSP_FALSE;
    buf = (isOldFormat ? qspGameToQSPString(strs[1], isUCS2, QSP_TRUE) : qspGameToQSPString(strs[2], isUCS2, QSP_TRUE));
    hasInvalidPassword = qspStrsComp(buf, password);
    free(buf);
#ifdef SPEC_PASS
    hasInvalidPassword = hasInvalidPassword && qspStrsComp(QSP_FMT(SPEC_PASS), password);
#endif
    if (hasInvalidPassword) return QSP_FALSE;
    buf = (isOldFormat ? qspGameToQSPString(strs[0], isUCS2, QSP_FALSE) : qspGameToQSPString(strs[3], isUCS2, QSP_TRUE));
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
            buf = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
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
        qspLocs = (QSPLocation *)realloc(qspLocs, qspLocsCount * sizeof(QSPLocation));
    }
    for (i = 0; i < qspLocsCount; ++i)
    {
        qspLocs[i].Name = 0;
        qspLocs[i].Desc = 0;
        qspLocs[i].OnVisit = 0;
        qspLocs[i].ActionsCount = 0;
        for (j = 0; j < QSP_MAXACTIONS; ++j)
        {
            qspLocs[i].Actions[j].Image = 0;
            qspLocs[i].Actions[j].Desc = 0;
            qspLocs[i].Actions[j].Code = 0;
        }
    }
}

QSP_CHAR *qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings)
{
    QSP_CHAR *name, *curStr, *line, *pos, *res = 0, quot = 0;
    int locStartLen, locEndLen, curBufSize = 1024, curStrLen = 0, resLen = 0, quotsCount = 0, strsCount = 0, locsCount = 0;
    QSP_BOOL isAddToString, isInLoc = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    curStr = qspAllocateBuffer(curBufSize);
    while (*data)
    {
        if (isInLoc)
        {
            if (!quot && !quotsCount && *data == QSP_NEWLINE)
            {
                line = qspSkipSpaces(data + 1);
                if (qspIsEqual(line, locEnd, locEndLen))
                {
                    isInLoc = QSP_FALSE;
                    pos = qspStrChr(line + locEndLen, QSP_NEWLINE);
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
                        if (toGetQStrings && quotsCount)
                            curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
                        ++data;
                    }
                    else
                        quot = 0;
                }
                if (quot || (toGetQStrings && quotsCount))
                {
                    if (*data == QSP_NEWLINE)
                        curStrLen = qspAddTextToBuffer(&curStr, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), curStrLen, &curBufSize);
                    else
                        curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
                }
                else
                {
                    if (curStrLen)
                    {
                        curStr[curStrLen] = 0;
                        resLen = qspAddText(&res, curStr, resLen, curStrLen, QSP_FALSE);
                        resLen = qspAddText(&res, QSP_STRSDELIM, resLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                        curStrLen = 0;
                        ++strsCount;
                    }
                }
            }
            else
            {
                isAddToString = (quotsCount > 0);
                if (*data == QSP_LQUOT)
                    ++quotsCount;
                else if (*data == QSP_RQUOT)
                {
                    if (quotsCount)
                    {
                        --quotsCount;
                        if (toGetQStrings && !quotsCount)
                        {
                            isAddToString = QSP_FALSE;
                            if (curStrLen)
                            {
                                curStr[curStrLen] = 0;
                                resLen = qspAddText(&res, curStr, resLen, curStrLen, QSP_FALSE);
                                resLen = qspAddText(&res, QSP_STRSDELIM, resLen, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                                curStrLen = 0;
                                ++strsCount;
                            }
                        }
                    }
                }
                else if (qspIsInList(QSP_QUOTS, *data))
                    quot = *data;
                if (toGetQStrings && isAddToString)
                {
                    if (*data == QSP_NEWLINE)
                        curStrLen = qspAddTextToBuffer(&curStr, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), curStrLen, &curBufSize);
                    else
                        curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
                }
            }
            ++data;
        }
        else
        {
            line = qspSkipSpaces(data);
            pos = qspStrChr(line, QSP_NEWLINE);
            if (qspIsEqual(line, locStart, locStartLen))
            {
                ++locsCount;
                isInLoc = QSP_TRUE;
                line += locStartLen;
                if (pos)
                {
                    *pos = 0;
                    name = qspDelSpc(line);
                    *pos = QSP_NEWLINE;
                }
                else
                    name = qspDelSpc(line);
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
    qspPrint("Extracted %d strings from %d locations\n", strsCount, locsCount);
    return res;
}

int qspOpenTextData(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toFill)
{
    QSP_CHAR *locCode, *line, *pos, quot = 0;
    int locStartLen, locEndLen, locBufSize = 1024, locCodeLen = 0, quotsCount = 0, curLoc = 0;
    QSP_BOOL isNewLine = QSP_TRUE, isInLoc = QSP_FALSE, isInBaseSection = QSP_FALSE, isInBaseAction = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    if (toFill)
    {
        /* Allocate buffer for code */
        locCode = qspAllocateBuffer(locBufSize);
    }
    while (*data)
    {
        if (isInLoc)
        {
            if (!quot && !quotsCount && isNewLine)
            {
                QSP_BOOL toSkipLine = QSP_FALSE;
                line = data;
                if (qspIsEqual(line, locEnd, locEndLen))
                {
                    line += locEndLen;
                    toSkipLine = QSP_TRUE; /* we're searching for a new loc now */

                    if (toFill && locCodeLen > QSP_LEN(QSP_STRSDELIM)) /* we have to remove the last line separator */
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
                        if (qspIsEqual(line, QSP_BASEACTS_ACT_FOOTER, QSP_LEN(QSP_BASEACTS_ACT_FOOTER)))
                        {
                            line += QSP_LEN(QSP_BASEACTS_ACT_FOOTER);
                            toSkipLine = QSP_TRUE;
                            /* Update action code */
                            if (toFill && locCodeLen > QSP_LEN(QSP_STRSDELIM)) /* we have to remove the last line separator */
                            {
                                locCode[locCodeLen - QSP_LEN(QSP_STRSDELIM)] = 0;
                                qspUpdateActionCode(locCode, qspLocs + curLoc);
                            }
                            locCodeLen = 0; /* reuse the code buffer */
                            isInBaseAction = QSP_FALSE;
                        }
                    }
                    else if (qspIsEqual(line, QSP_BASESECTION_FOOTER, QSP_LEN(QSP_BASESECTION_FOOTER)))
                    {
                        line += QSP_LEN(QSP_BASESECTION_FOOTER);
                        toSkipLine = QSP_TRUE;

                        locCodeLen = 0; /* reuse the code buffer */
                        isInBaseSection = QSP_FALSE;
                    }
                    else if (qspIsEqual(line, QSP_BASEACTS_ACT_HEADER, QSP_LEN(QSP_BASEACTS_ACT_HEADER)))
                    {
                        line += QSP_LEN(QSP_BASEACTS_ACT_HEADER);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add an action */
                        if (toFill)
                            qspParseBaseAction(&line, qspLocs + curLoc);
                        else
                            qspParseBaseAction(&line, 0);

                        locCodeLen = 0; /* reuse the code buffer */
                        isInBaseAction = QSP_TRUE;
                    }
                    else if (qspIsInList(QSP_QUOTS, *line))
                    {
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        if (toFill)
                            qspParseBaseDescriptionPrintLine(&line, qspLocs + curLoc);
                        else
                            qspParseBaseDescriptionPrintLine(&line, 0);
                    }
                    else if (qspIsEqual(line, QSP_BASEDESC_PRINTLINE, QSP_LEN(QSP_BASEDESC_PRINTLINE)))
                    {
                        line += QSP_LEN(QSP_BASEDESC_PRINTLINE);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        if (toFill)
                            qspParseBaseDescriptionPrintLine(&line, qspLocs + curLoc);
                        else
                            qspParseBaseDescriptionPrintLine(&line, 0);
                    }
                    else if (qspIsEqual(line, QSP_BASEDESC_PRINT, QSP_LEN(QSP_BASEDESC_PRINT)))
                    {
                        line += QSP_LEN(QSP_BASEDESC_PRINT);
                        toSkipLine = QSP_TRUE;
                        /* Parse & add base description */
                        if (toFill)
                            qspParseBaseDescriptionPrint(&line, qspLocs + curLoc);
                        else
                            qspParseBaseDescriptionPrint(&line, 0);
                    }
                }
                else if (qspIsEqual(line, QSP_BASESECTION_HEADER, QSP_LEN(QSP_BASESECTION_HEADER)))
                {
                    line += QSP_LEN(QSP_BASESECTION_HEADER);
                    toSkipLine = QSP_TRUE;

                    isInBaseSection = QSP_TRUE;
                    isInBaseAction = QSP_FALSE;
                }
                if (toSkipLine)
                {
                    pos = qspStrChr(line, QSP_NEWLINE);
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
            /* Always update the locCode */
            if (*data == QSP_NEWLINE)
            {
                if (toFill)
                    locCodeLen = qspAddTextToBuffer(&locCode, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), locCodeLen, &locBufSize);
                isNewLine = QSP_TRUE;
            }
            else
            {
                if (toFill)
                    locCodeLen = qspAddCharToBuffer(&locCode, *data, locCodeLen, &locBufSize);
                isNewLine = QSP_FALSE;
            }
            if (quot)
            {
                if (*data == quot)
                {
                    if (*(data + 1) == quot)
                    {
                        if (toFill)
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
            line = qspSkipSpaces(data);
            pos = qspStrChr(line, QSP_NEWLINE);
            if (qspIsEqual(line, locStart, locStartLen))
            {
                if (toFill)
                {
                    line += locStartLen;
                    if (pos)
                    {
                        *pos = 0;
                        qspLocs[curLoc].Name = qspDelSpc(line);
                        *pos = QSP_NEWLINE;
                    }
                    else
                        qspLocs[curLoc].Name = qspDelSpc(line);

                    qspPrint("Location: %s\n", qspLocs[curLoc].Name);
                }
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
        if (toFill && locCodeLen)
        {
            locCode[locCodeLen] = 0;
            if (isInBaseAction)
                qspUpdateActionCode(locCode, qspLocs + curLoc);
            else if (!isInBaseSection)
                qspUpdateLocationCode(locCode, qspLocs + curLoc);
        }
        ++curLoc;
    }

    if (toFill)
    {
        free(locCode);
        qspPrint("%d locations were loaded\n", curLoc);
    }

    return curLoc;
}

char *qspSaveQuest(QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd, int *dataLen)
{
    int i, j, len, actsCount;
    char *buf = 0;
    if (isOldFormat)
    {
        len = qspGameCodeWriteIntValLine(&buf, 0, qspLocsCount, isUCS2, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, passwd, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, QSP_VER, isUCS2, QSP_FALSE);
        for (i = 0; i < 27; ++i) len = qspGameCodeWriteValLine(&buf, len, 0, isUCS2, QSP_FALSE);
    }
    else
    {
        len = qspGameCodeWriteValLine(&buf, 0, QSP_GAMEID, isUCS2, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, QSP_VER, isUCS2, QSP_FALSE);
        len = qspGameCodeWriteValLine(&buf, len, passwd, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteIntValLine(&buf, len, qspLocsCount, isUCS2, QSP_TRUE);
    }
    for (i = 0; i < qspLocsCount; ++i)
    {
        qspPrint("Saving location: %s\n", qspLocs[i].Name);

        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Name, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Desc, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].OnVisit, isUCS2, QSP_TRUE);

        if (isOldFormat)
        {
            for (j = 0; j < 20; ++j)
            {
                if (j < qspLocs[i].ActionsCount)
                {
                    len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Desc, isUCS2, QSP_TRUE);
                    len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Code, isUCS2, QSP_TRUE);
                }
                else
                {
                    len = qspGameCodeWriteValLine(&buf, len, 0, isUCS2, QSP_TRUE);
                    len = qspGameCodeWriteValLine(&buf, len, 0, isUCS2, QSP_TRUE);
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
            len = qspGameCodeWriteIntValLine(&buf, len, actsCount, isUCS2, QSP_TRUE);
            for (j = 0; j < actsCount; ++j)
            {
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Image, isUCS2, QSP_TRUE);
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Desc, isUCS2, QSP_TRUE);
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Code, isUCS2, QSP_TRUE);
            }
        }
    }

    qspPrint("%d locations were saved\n", qspLocsCount);

    *dataLen = isUCS2 ? len * 2 : len;
    return buf;
}

QSP_BOOL qspOpenQuest(char *data, int dataSize, QSP_CHAR *password)
{
    QSP_BOOL isOldFormat, isUCS2;
    int i, j, ind, count, locsCount, actsCount;
    QSP_CHAR *buf;
    char **strs;
    /* Parse the data */
    if (dataSize < 2) return QSP_FALSE;
    count = qspSplitGameStr(data, isUCS2 = !data[1], QSP_STRSDELIM, &strs);
    if (!qspCheckQuest(strs, count, isUCS2, password))
    {
        qspFreeStrs((void **)strs, count);
        return QSP_FALSE;
    }
    buf = qspGameToQSPString(strs[0], isUCS2, QSP_FALSE);
    isOldFormat = qspStrsComp(buf, QSP_GAMEID) != 0;
    free(buf);
    buf = (isOldFormat ? qspGameToQSPString(strs[0], isUCS2, QSP_FALSE) : qspGameToQSPString(strs[3], isUCS2, QSP_TRUE));
    locsCount = qspStrToNum(buf, 0);
    free(buf);
    qspCreateWorld(locsCount);
    ind = (isOldFormat ? 30 : 4);
    for (i = 0; i < locsCount; ++i)
    {
        qspLocs[i].Name = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
        qspLocs[i].Desc = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
        qspLocs[i].OnVisit = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);

        qspPrint("Location: %s\n", qspLocs[i].Name);

        if (isOldFormat)
            actsCount = 20;
        else
        {
            buf = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
            actsCount = qspStrToNum(buf, 0);
            free(buf);
        }
        qspLocs[i].ActionsCount = actsCount;
        for (j = 0; j < actsCount; ++j)
        {
            qspLocs[i].Actions[j].Image = (isOldFormat ? 0 : qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE));
            qspLocs[i].Actions[j].Desc = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
            qspLocs[i].Actions[j].Code = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
        }
    }

    qspPrint("%d locations were loaded\n", locsCount);

    qspFreeStrs((void **)strs, count);
    return QSP_TRUE;
}

QSP_CHAR *qspSaveQuestAsText(QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    QSP_BOOL hasBaseDesc;
    int i, j, k, linesCount, baseActsCount, len = 0, bufSize = 4096;
    QSP_CHAR *temp, **lines, *buf = qspAllocateBuffer(bufSize);
    for (i = 0; i < qspLocsCount; ++i)
    {
        qspPrint("Saving location: %s\n", qspLocs[i].Name);

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

    qspPrint("%d locations were saved, total length: %d characters\n", qspLocsCount, len);

    return buf;
}
