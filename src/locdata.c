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

#include "locdata.h"
#include "text.h"
#include "sys.h"

QSP_CHAR *qspReadString(QSP_CHAR **str)
{
    int len = 0, bufSize = 64;
    QSP_CHAR quot, *buf, *ptr = qspSkipSpaces(*str);
    if (!qspIsInList(QSP_QUOTS, *ptr))
    {
        *str = ptr;
        return 0;
    }
    buf = qspAllocateBuffer(bufSize);
    quot = *ptr++;
    while (*ptr)
    {
        if (*ptr == quot)
        {
            if (*(ptr + 1) != quot)
            {
                buf[len] = 0;
                *str = qspSkipSpaces(ptr + 1);
                return buf;
            }
            ++ptr;
        }
        len = qspAddCharToBuffer(&buf, *ptr, len, &bufSize);
        ++ptr;
    }
    free(buf);
    *str = ptr;
    return 0;
}

QSP_BOOL qspInitLocProcessor()
{
    return QSP_TRUE;
}

void qspTerminateLocProcessor()
{
}

QSP_BOOL qspParseBaseDescriptionPrint(QSP_CHAR **code, QSPLocation *loc)
{
    QSP_CHAR *desc, *ptr = *code;
    desc = qspReadString(&ptr);
    if (!desc)
    {
        qspPrint("Warning: can't get base description\n");
        *code = ptr;
        return QSP_FALSE;
    }
    if (loc)
    {
        qspPrint("\tAdding base description\n");
        qspAddText(&loc->Desc, desc, -1, -1, QSP_FALSE);
    }
    free(desc);
    *code = ptr;
    return QSP_TRUE;
}

QSP_BOOL qspParseBaseDescriptionPrintLine(QSP_CHAR **code, QSPLocation *loc)
{
    QSP_CHAR *desc, *ptr = *code;
    desc = qspReadString(&ptr);
    if (loc)
    {
        qspPrint("\tAdding base description\n");
        if (desc) /* optional parameter */
            qspAddText(&loc->Desc, desc, -1, -1, QSP_FALSE);
        qspAddText(&loc->Desc, QSP_STRSDELIM, -1, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
    }
    if (desc) free(desc);
    *code = ptr;
    return QSP_TRUE;
}

QSP_BOOL qspParseBaseAction(QSP_CHAR **code, QSPLocation *loc)
{
    QSP_CHAR *actImage, *actName, *ptr = *code;
    actName = qspReadString(&ptr);
    if (!actName)
    {
        qspPrint("Warning: can't get the action name\n");
        *code = ptr;
        return QSP_FALSE;
    }
    if (*ptr == QSP_COMMA)
    {
        ++ptr;
        actImage = qspReadString(&ptr);
        if (!actImage)
        {
            qspPrint("Warning: can't get the image\n");
            free(actName);
            *code = ptr;
            return QSP_FALSE;
        }
    }
    else
        actImage = 0;
    if (*ptr != QSP_COLON)
    {
        qspPrint("Warning: base action is incorrect\n");
        free(actName);
        if (actImage) free(actImage);
        *code = ptr;
        return QSP_FALSE;
    }
    if (loc)
    {
        int actionsCount = loc->ActionsCount;
        if (actionsCount >= QSP_MAXACTIONS)
        {
            qspPrint("Warning: too many base actions\n");
            free(actName);
            if (actImage) free(actImage);
            *code = ptr;
            return QSP_FALSE;
        }
        qspPrint("\tAction: %s\n", actName);
        loc->Actions[actionsCount].Desc = actName;
        loc->Actions[actionsCount].Image = actImage;
        loc->Actions[actionsCount].Code = 0;
        loc->ActionsCount++;
    }
    else
    {
        free(actName);
        if (actImage) free(actImage);
    }
    *code = qspSkipSpaces(ptr + 1);
    return QSP_TRUE;
}

QSP_BOOL qspUpdateActionCode(QSP_CHAR *code, QSPLocation *loc)
{
    QSP_CHAR *formattedCode, *formattedLine, **lines;
    int i, bufSize, formattedCodeLen, linesCount, actionsCount = loc->ActionsCount;
    if (!actionsCount)
    {
        qspPrint("Warning: no actions to update\n");
        return QSP_FALSE;
    }
    bufSize = 512;
    formattedCode = qspAllocateBuffer(bufSize);
    formattedCodeLen = 0;
    linesCount = qspSplitStr(code, QSP_STRSDELIM, &lines);
    for (i = 0; i < linesCount; i++)
    {
        if (qspIsEqual(lines[i], QSP_BASEACTS_LINE_PREFIX, QSP_LEN(QSP_BASEACTS_LINE_PREFIX)))
            formattedLine = lines[i] + QSP_LEN(QSP_BASEACTS_LINE_PREFIX);
        else
            formattedLine = lines[i];
        formattedCodeLen = qspAddTextToBuffer(&formattedCode, formattedLine, -1, formattedCodeLen, &bufSize);
        if (i == linesCount - 1) break;
        formattedCodeLen = qspAddTextToBuffer(&formattedCode, QSP_STRSDELIM, QSP_LEN(QSP_STRSDELIM), formattedCodeLen, &bufSize);
    }
    qspFreeStrs((void **)lines, linesCount);
    loc->Actions[actionsCount - 1].Code = formattedCode;
    return QSP_TRUE;
}

QSP_BOOL qspUpdateLocationCode(QSP_CHAR *code, QSPLocation *loc)
{
    loc->OnVisit = qspNewStr(code);
    return QSP_TRUE;
}
