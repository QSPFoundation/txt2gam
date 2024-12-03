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

#include "main.h"
#include "coding.h"
#include "locations.h"
#include "locdata.h"
#include "text.h"

static QSP_BOOL qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data);
static QSP_BOOL qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode);
static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isGetQStrings, QSP_BOOL isUnicode);
static QSP_BOOL qspOpenQuestFromTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd);
static QSP_BOOL qspSaveGameFile(char *file, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd);
static QSP_BOOL qspOpenGameFile(char *file, QSP_CHAR *password);
static QSP_BOOL qspSaveQuestToTextFile(char *file, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isUnicode);

static QSP_BOOL qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data)
{
    int fileSize;
    char *buf, *resBuf;
    int encoding;
    FILE *f;
    /* Loading file's contents */
    if (!(f = fopen(file, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    buf = (char *)malloc(fileSize + 4);
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    fclose(f);
    buf[fileSize] = buf[fileSize + 1] = buf[fileSize + 2] = buf[fileSize + 3] = 0;
    resBuf = buf;
    if ((unsigned char)resBuf[0] == (unsigned char)TXT2GAM_UCS2BOM[0]
        && (unsigned char)resBuf[1] == (unsigned char)TXT2GAM_UCS2BOM[1])
    {
        resBuf += 2; /* skip BOM */
        encoding = QSP_UCS2;
    }
    else if (resBuf[0] && !resBuf[1])
        encoding = QSP_UCS2;
    else if ((unsigned char)resBuf[0] == (unsigned char)TXT2GAM_UTF8BOM[0]
        && (unsigned char)resBuf[1] == (unsigned char)TXT2GAM_UTF8BOM[1]
        && (unsigned char)resBuf[2] == (unsigned char)TXT2GAM_UTF8BOM[2])
    {
        resBuf += 3; /* skip BOM */
        encoding = QSP_UTF8;
    }
    else
        encoding = isUnicode ? QSP_UTF8 : QSP_ANSI;

    switch (encoding)
    {
    case QSP_UCS2:
        *data = qspGameToQSPString(resBuf, QSP_TRUE, QSP_FALSE);
        break;
    case QSP_UTF8:
        *data = qspUTF8ToQSPString(resBuf);
        break;
    case QSP_ANSI:
        *data = qspGameToQSPString(resBuf, QSP_FALSE, QSP_FALSE);
        break;
    }
    free(buf);

    if (*data)
    {
        qspFormatLineEndings(*data);
        return QSP_TRUE;
    }
    return QSP_FALSE;
}

static QSP_BOOL qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode)
{
    FILE *f;
    if (!(f = fopen(file, "wb")))
        return QSP_FALSE;
    if (isUnicode)
    {
        char *content = qspQSPStringToUTF8(data);
        if (!content)
        {
            fclose(f);
            return QSP_FALSE;
        }
        fwrite(TXT2GAM_UTF8BOM, 1, sizeof(TXT2GAM_UTF8BOM) - 1, f);
        fwrite(content, 1, strlen(content), f);
        free(content);
    }
    else
    {
        char *content = qspQSPToGameString(data, QSP_FALSE, QSP_FALSE);
        fwrite(content, 1, strlen(content), f);
        free(content);
    }
    fclose(f);
    return QSP_TRUE;
}

static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isGetQStrings, QSP_BOOL isUnicode)
{
    QSP_CHAR *data, *locsStrings;
    if (!qspLoadTextFile(file, isUnicode, &data)) return QSP_FALSE;
    locsStrings = qspGetLocsStrings(data, locStart, locEnd, isGetQStrings);
    free(data);
    if (locsStrings)
    {
        QSP_BOOL res = qspSaveTextFile(outFile, locsStrings, isUnicode);
        free(locsStrings);
        return res;
    }
    return QSP_TRUE;
}

static QSP_BOOL qspOpenQuestFromTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd)
{
    int locsCount;
    QSP_CHAR *data;
    if (!qspLoadTextFile(file, isUnicode, &data)) return QSP_FALSE;
    locsCount = qspOpenTextData(data, locStart, locEnd, QSP_FALSE);
    qspCreateWorld(locsCount);
    qspOpenTextData(data, locStart, locEnd, QSP_TRUE);
    free(data);
    return QSP_TRUE;
}

static QSP_BOOL qspSaveGameFile(char *file, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd)
{
    int len;
    FILE *f;
    char *gameData = qspSaveQuest(isOldFormat, isUCS2, passwd, &len);
    if (!gameData) return QSP_FALSE;
    if (!(f = fopen(file, "wb")))
    {
        free(gameData);
        return QSP_FALSE;
    }
    fwrite(gameData, 1, len, f);
    free(gameData);
    fclose(f);
    return QSP_TRUE;
}

static QSP_BOOL qspOpenGameFile(char *file, QSP_CHAR *password)
{
    QSP_BOOL res;
    int fileSize;
    char *data;
    FILE *f;
    if (!(f = fopen(file, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    data = (char *)malloc(fileSize);
    fseek(f, 0, SEEK_SET);
    fread(data, 1, fileSize, f);
    fclose(f);
    res = qspOpenQuest(data, fileSize, password);
    free(data);
    return res;
}

static QSP_BOOL qspSaveQuestToTextFile(char *file, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isUnicode)
{
    QSP_BOOL res;
    QSP_CHAR *gameData = qspSaveQuestAsText(locStart, locEnd);
    if (!gameData) return QSP_FALSE;
    res = qspSaveTextFile(file, gameData, isUnicode);
    free(gameData);
    return res;
}

void ShowHelp()
{
    char *temp = qspFromQSPString(QSP_VER);
    printf("TXT2GAM utility, ver. %s\n", temp);
    free(temp);
    printf("Usage:\n");
    printf("  txt2gam [input file] [output file] [options]\n");
    printf("Options:\n");
    printf("  a, A - ANSI mode, default is Unicode (UTF-8, UCS-2/UTF-16) mode\n");
    printf("  o, O - Save game in old format, default is new format\n");
    temp = qspFromQSPString(QSP_STARTLOC);
    printf("  s[string], S[string] - 'Start of loc' prefix, default is '%s'\n", temp);
    free(temp);
    temp = qspFromQSPString(QSP_ENDLOC);
    printf("  e[string], E[string] - 'End of loc' prefix, default is '%s'\n", temp);
    free(temp);
    temp = qspFromQSPString(QSP_PASSWD);
    printf("  p[pass], P[pass] - Password, default is '%s'\n", temp);
    free(temp);
    printf("  c, C - Encode text file into game file (default mode)\n");
    printf("  d, D - Decode game file into text file\n");
    printf("  t, T - Extract strings from text\n");
    printf("  q, Q - Extract q-strings from text\n");
    printf("Examples:\n");
    printf("  txt2gam file.txt gamefile.qsp pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp\n");
    printf("  txt2gam file.txt gamefile.qsp a\n");
    printf("  txt2gam gamefile.qsp file.txt d pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp o pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp o e@ pMyPassword\n");
    printf("  txt2gam file.txt gamefile.qsp o \"pMy Password\"\n");
    printf("  txt2gam file.txt gamefile.qsp a o \"pMy Password\"\n");
    printf("  txt2gam file.txt gamefile.qsp o\n");
    printf("  txt2gam file.txt gamefile.qsp o e@\n");
    printf("  txt2gam file.txt gamefile.qsp s@ e~\n");
    printf("  txt2gam file.txt strsfile.txt t a\n");
}

int main(int argc, char **argv)
{
    int i, workMode;
    QSP_BOOL isOldFormat, isUnicode, isErr;
    QSP_CHAR *passwd, *locStart, *locEnd;
    setlocale(LC_ALL, QSP_LOCALE);
    if (argc < 3)
    {
        ShowHelp();
        return 0;
    }
    workMode = QSP_ENCODE_INTO_GAME;
    isOldFormat = QSP_FALSE;
    isUnicode = QSP_TRUE;
    qspAddText(&locStart, QSP_STARTLOC, 0, -1, QSP_TRUE);
    qspAddText(&locEnd, QSP_ENDLOC, 0, -1, QSP_TRUE);
    qspAddText(&passwd, QSP_PASSWD, 0, -1, QSP_TRUE);
    for (i = 3; i < argc; ++i)
    {
        switch (*argv[i])
        {
        case 'o': case 'O':
            isOldFormat = QSP_TRUE;
            break;
        case 'a': case 'A':
            isUnicode = QSP_FALSE;
            break;
        case 's': case 'S':
        case 'e': case 'E':
        case 'p': case 'P':
            if (argv[i][1])
            {
                switch (*argv[i])
                {
                case 's': case 'S':
                    free(locStart);
                    locStart = qspToQSPString(argv[i] + 1);
                    break;
                case 'e': case 'E':
                    free(locEnd);
                    locEnd = qspToQSPString(argv[i] + 1);
                    break;
                case 'p': case 'P':
                    free(passwd);
                    passwd = qspToQSPString(argv[i] + 1);
                    break;
                }
            }
            break;
        case 'c': case 'C':
            workMode = QSP_ENCODE_INTO_GAME;
            break;
        case 'd': case 'D':
            workMode = QSP_DECODE_INTO_TEXT;
            break;
        case 't': case 'T':
            workMode = QSP_EXTRACT_STRINGS;
            break;
        case 'q': case 'Q':
            workMode = QSP_EXTRACT_QSTRINGS;
            break;
        }
    }
    if (!qspInitLocProcessor())
    {
        printf("Can't initialize the loc processor!\n");
        return 1;
    }
    qspLocs = 0;
    qspLocsCount = 0;
    isErr = QSP_FALSE;
    switch (workMode)
    {
        case QSP_EXTRACT_STRINGS:
        case QSP_EXTRACT_QSTRINGS:
            if (isErr = !qspExportStrings(argv[1], argv[2], locStart, locEnd, workMode == QSP_EXTRACT_QSTRINGS, isUnicode))
                printf("String extraction has failed!\n");
            break;
        case QSP_ENCODE_INTO_GAME:
            do
            {
                if (isErr = !qspOpenQuestFromTextFile(argv[1], isUnicode, locStart, locEnd))
                {
                    printf("Loading text file has failed!\n");
                    break;
                }
                if (isErr = !qspSaveGameFile(argv[2], isOldFormat, isUnicode, passwd))
                    printf("Saving game has failed!\n");
            } while (0);
            break;
        case QSP_DECODE_INTO_TEXT:
            do
            {
                if (isErr = !qspOpenGameFile(argv[1], passwd))
                {
                    printf("Can't open game!\n");
                    break;
                }
                if (isErr = !qspSaveQuestToTextFile(argv[2], locStart, locEnd, isUnicode))
                    printf("Saving text file has failed!\n");
            } while (0);
            break;
    }
    qspCreateWorld(0);
    qspTerminateLocProcessor();
    free(locStart);
    free(locEnd);
    free(passwd);
    return (isErr == QSP_TRUE);
}
