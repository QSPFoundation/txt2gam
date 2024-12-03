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
    buf = (isOldFormat ?
           qspGameToQSPString(strs[1], isUCS2, QSP_TRUE) : qspGameToQSPString(strs[2], isUCS2, QSP_TRUE));
    hasInvalidPassword = qspStrsComp(buf, password);
    free(buf);
#ifdef SPEC_PASS
    hasInvalidPassword = hasInvalidPassword && qspStrsComp(QSP_FMT(SPEC_PASS), password);
#endif
    if (hasInvalidPassword) return QSP_FALSE;
    buf = (isOldFormat ?
           qspGameToQSPString(strs[0], isUCS2, QSP_FALSE) : qspGameToQSPString(strs[3], isUCS2, QSP_TRUE));
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

QSP_CHAR *qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isGetQStrings)
{
    QSP_CHAR *name, *curStr, *line, *pos, *res = 0, quot = 0;
    int locStartLen, locEndLen, curBufSize = 1024, curStrLen = 0, resLen = 0, quotsCount = 0, strsCount = 0, locsCount = 0;
    QSP_BOOL isAddToString, isInLoc = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    curStr = (QSP_CHAR *)malloc(curBufSize * sizeof(QSP_CHAR));
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
                        if (isGetQStrings && quotsCount)
                            curStrLen = qspAddCharToBuffer(&curStr, *data, curStrLen, &curBufSize);
                        ++data;
                    }
                    else
                        quot = 0;
                }
                if (quot || (isGetQStrings && quotsCount))
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
                if (*data == QSP_LQUOT[0])
                    ++quotsCount;
                else if (*data == QSP_RQUOT[0])
                {
                    if (quotsCount)
                    {
                        --quotsCount;
                        if (isGetQStrings && !quotsCount)
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
                if (isGetQStrings && isAddToString)
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
                    /* Add a new line to separate locations */
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
    printf("Extracted %d strings from %d locations\n", strsCount, locsCount);
    return res;
}

int qspOpenTextData(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isFill)
{
    char *name;
    QSP_CHAR *locCode, *line, *pos, quot = 0;
    int locStartLen, locEndLen, locBufSize, locCodeLen = 0, quotsCount = 0, curLoc = 0;
    QSP_BOOL isInLoc = QSP_FALSE;
    locStartLen = qspStrLen(locStart);
    locEndLen = qspStrLen(locEnd);
    while (*data)
    {
        if (isInLoc)
        {
            if (!quot && !quotsCount && (!locCodeLen || *data == QSP_NEWLINE))
            {
                line = qspSkipSpaces(locCodeLen ? data + 1 : data);
                if (qspIsEqual(line, locEnd, locEndLen))
                {
                    isInLoc = QSP_FALSE;
                    if (isFill)
                    {
                        locCode[locCodeLen] = 0;
                        if (qspProcessLocationData(locCode, qspLocs + curLoc))
                            free(locCode);
                        else
                            qspLocs[curLoc].OnVisit = locCode;
                    }
                    ++curLoc;
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
            if (isFill)
            {
                if (*data == QSP_NEWLINE)
                    locCodeLen = qspAddTextToBuffer(&locCode, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), locCodeLen, &locBufSize);
                else
                    locCodeLen = qspAddCharToBuffer(&locCode, *data, locCodeLen, &locBufSize);
            }
            else
            {
                ++locCodeLen; /* used to check if location is empty */
            }
            if (quot)
            {
                if (*data == quot)
                {
                    if (*(data + 1) == quot)
                    {
                        locCodeLen = (isFill ? qspAddCharToBuffer(&locCode, *data, locCodeLen, &locBufSize) : locCodeLen + 1);
                        ++data;
                    }
                    else
                        quot = 0;
                }
            }
            else
            {
                if (*data == QSP_LQUOT[0])
                    ++quotsCount;
                else if (*data == QSP_RQUOT[0])
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
            line = qspSkipSpaces(data);
            pos = qspStrChr(line, QSP_NEWLINE);
            if (qspIsEqual(line, locStart, locStartLen))
            {
                isInLoc = QSP_TRUE;
                locCodeLen = 0;
                if (isFill)
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

                    name = qspFromQSPString(qspLocs[curLoc].Name);
                    printf("Location: %s\n", name);
                    free(name);

                    locBufSize = 1024;
                    locCode = (QSP_CHAR *)malloc(locBufSize * sizeof(QSP_CHAR));
                }
            }
            if (pos)
                data = pos + 1;
            else
                break;
        }
    }
    if (isInLoc)
    {
        if (isFill)
        {
            locCode[locCodeLen] = 0;
            if (qspProcessLocationData(locCode, qspLocs + curLoc))
                free(locCode);
            else
                qspLocs[curLoc].OnVisit = locCode;
        }
        ++curLoc;
    }

    if (isFill)
    {
        printf("%d locations were loaded\n", curLoc);
    }

    return curLoc;
}

char *qspSaveQuest(QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd, int *dataLen)
{
    int i, j, len, actsCount;
    char *name, *buf = 0;
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
        name = qspFromQSPString(qspLocs[i].Name);
        printf("Saving location: %s\n", name);
        free(name);

        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Name, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Desc, isUCS2, QSP_TRUE);
        len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].OnVisit, isUCS2, QSP_TRUE);

        if (isOldFormat)
        {
            actsCount = qspLocs[i].ActionsCount < 20 ? qspLocs[i].ActionsCount : 20;
            for (j = 0; j < actsCount; ++j)
            {
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Desc, isUCS2, QSP_TRUE);
                len = qspGameCodeWriteValLine(&buf, len, qspLocs[i].Actions[j].Code, isUCS2, QSP_TRUE);
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

    printf("%d locations were saved\n", qspLocsCount);

    *dataLen = isUCS2 ? len * 2 : len;
    return buf;
}

QSP_BOOL qspOpenQuest(char *data, int dataSize, QSP_CHAR *password)
{
    QSP_BOOL isOldFormat, isUCS2;
    int i, j, ind, count, locsCount, actsCount;
    QSP_CHAR *buf;
    char **strs, *name;
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
    buf = (isOldFormat ?
           qspGameToQSPString(strs[0], isUCS2, QSP_FALSE) : qspGameToQSPString(strs[3], isUCS2, QSP_TRUE));
    locsCount = qspStrToNum(buf, 0);
    free(buf);
    qspCreateWorld(locsCount);
    ind = (isOldFormat ? 30 : 4);
    for (i = 0; i < locsCount; ++i)
    {
        qspLocs[i].Name = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
        qspLocs[i].Desc = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);
        qspLocs[i].OnVisit = qspGameToQSPString(strs[ind++], isUCS2, QSP_TRUE);

        name = qspFromQSPString(qspLocs[i].Name);
        printf("Location: %s\n", name);
        free(name);

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

    printf("%d locations were loaded\n", locsCount);

    qspFreeStrs((void **)strs, count);
    return QSP_TRUE;
}

QSP_CHAR *qspSaveQuestAsText(QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    char *name;
    int i, j, k, linesCount, len = 0;
    QSP_CHAR *temp, **tempStrs, *buf = 0;
    for (i = 0; i < qspLocsCount; ++i)
    {
        name = qspFromQSPString(qspLocs[i].Name);
        printf("Saving location: %s\n", name);
        free(name);

        /* Write location header */
        len = qspAddText(&buf, locStart, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_FMT(" "), len, -1, QSP_FALSE);
        len = qspAddText(&buf, qspLocs[i].Name, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);

        /* Write main description of the location */
        if (qspLocs[i].Desc && !qspIsEmpty(qspLocs[i].Desc))
        {
            temp = qspReplaceText(qspLocs[i].Desc, QSP_FMT("'"), QSP_FMT("''"));
            linesCount = qspSplitStr(temp, QSP_STRSDELIM, &tempStrs);
            free(temp);
            if (linesCount)
            {
                for (j = 0; j < linesCount - 1; ++j)
                {
                    if (qspIsEmpty(tempStrs[j]))
                    {
                        len = qspAddText(&buf, QSP_FMT("*NL"), len, -1, QSP_FALSE);
                    }
                    else
                    {
                        len = qspAddText(&buf, QSP_FMT("*PL '"), len, -1, QSP_FALSE);
                        len = qspAddText(&buf, tempStrs[j], len, -1, QSP_FALSE);
                        len = qspAddText(&buf, QSP_FMT("'"), len, -1, QSP_FALSE);
                    }
                    len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                }
                if (!qspIsEmpty(tempStrs[linesCount - 1]))
                {
                    len = qspAddText(&buf, QSP_FMT("*P '"), len, -1, QSP_FALSE);
                    len = qspAddText(&buf, tempStrs[linesCount - 1], len, -1, QSP_FALSE);
                    len = qspAddText(&buf, QSP_FMT("'"), len, -1, QSP_FALSE);
                    len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                }
            }
            qspFreeStrs((void **)tempStrs, linesCount);
        }
        /* Write actions */
        for (j = 0; j < qspLocs[i].ActionsCount; ++j)
        {
            if (!qspLocs[i].Actions[j].Desc)
                break;
            if (qspIsEmpty(qspLocs[i].Actions[j].Desc))
                break;

            /* Write action header */
            len = qspAddText(&buf, QSP_FMT("ACT '"), len, -1, QSP_FALSE);
            temp = qspReplaceText(qspLocs[i].Actions[j].Desc, QSP_FMT("'"), QSP_FMT("''"));
            len = qspAddText(&buf, temp, len, -1, QSP_FALSE);
            free(temp);
            len = qspAddText(&buf, QSP_FMT("'"), len, -1, QSP_FALSE);

            if (qspLocs[i].Actions[j].Image && !qspIsEmpty(qspLocs[i].Actions[j].Image))
            {
                len = qspAddText(&buf, QSP_FMT(", '"), len, -1, QSP_FALSE);
                temp = qspReplaceText(qspLocs[i].Actions[j].Image, QSP_FMT("'"), QSP_FMT("''"));
                len = qspAddText(&buf, temp, len, -1, QSP_FALSE);
                free(temp);
                len = qspAddText(&buf, QSP_FMT("'"), len, -1, QSP_FALSE);
            }
            len = qspAddText(&buf, QSP_FMT(":"), len, -1, QSP_FALSE);
            len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);

            /* Write action code */
            if (qspLocs[i].Actions[j].Code && !qspIsEmpty(qspLocs[i].Actions[j].Code))
            {
                linesCount = qspSplitStr(qspLocs[i].Actions[j].Code, QSP_STRSDELIM, &tempStrs);
                for (k = 0; k < linesCount; ++k)
                {
                    len = qspAddText(&buf, QSP_FMT("\t"), len, -1, QSP_FALSE);
                    len = qspAddText(&buf, tempStrs[k], len, -1, QSP_FALSE);
                    len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
                }
                qspFreeStrs((void **)tempStrs, linesCount);
            }

            /* Write action footer */
            len = qspAddText(&buf, QSP_FMT("END"), len, -1, QSP_FALSE);
            len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
        }
        /* Write location code */
        len = qspAddText(&buf, qspLocs[i].OnVisit, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);

        /* Write location footer */
        len = qspAddText(&buf, locEnd, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_FMT(" "), len, -1, QSP_FALSE);
        len = qspAddText(&buf, qspLocs[i].Name, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_FMT(" ---------------------------------") QSP_STRSDELIM, len, -1, QSP_FALSE);
        len = qspAddText(&buf, QSP_STRSDELIM, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
    }

    printf("%d locations were saved, total length: %d characters\n", qspLocsCount, len);

    return buf;
}
