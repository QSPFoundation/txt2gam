/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "t2g_api.h"
#include "t2g_default.h"
#include "coding.h"
#include "text.h"
#include "sys.h"

enum Mode
{
    QSP_ERROR,
    QSP_ENCODE_INTO_GAME,
    QSP_DECODE_INTO_TEXT,
    QSP_EXTRACT_STRINGS,
    QSP_EXTRACT_QSTRINGS
};

static QSP_BOOL qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data);
static QSP_BOOL qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode);
static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode);
static QSP_BOOL qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_CHAR *passwd);
static QSP_BOOL qspDecodeGameToText(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_CHAR *passwd);

static QSP_BOOL qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data)
{
    int fileSize;
    char *buf;
    FILE *f;
    if (!(f = fopen(file, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    buf = (char *)malloc(fileSize);
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    fclose(f);
    *data = t2gParseTextData(buf, fileSize, isUnicode);
    free(buf);
    return *data != 0;
}

static QSP_BOOL qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode)
{
    FILE *f;
    if (!(f = fopen(file, "wb")))
        return QSP_FALSE;
    if (isUnicode)
    {
        char *content = qspQSPStringToUTF8(data, -1);
        if (!content)
        {
            fclose(f);
            return QSP_FALSE;
        }
        fwrite(T2G_UTF8BOM, 1, sizeof(T2G_UTF8BOM) - 1, f);
        fwrite(content, 1, strlen(content), f);
        free(content);
    }
    else
    {
        char *content = qspQSPToGameString(data, -1, QSP_FALSE, QSP_FALSE);
        fwrite(content, 1, strlen(content), f);
        free(content);
    }
    fclose(f);
    return QSP_TRUE;
}

static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode)
{
    QSP_CHAR *data, *locsStrings;
    int len;
    if (!qspLoadTextFile(file, isUnicode, &data)) return QSP_FALSE;
    locsStrings = t2gExtractStrings(data, locStart, locEnd, toGetQStrings, &len);
    free(data);
    if (locsStrings)
    {
        QSP_BOOL res = qspSaveTextFile(outFile, locsStrings, isUnicode);
        free(locsStrings);
        return res;
    }
    return QSP_FALSE;
}

static QSP_BOOL qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_CHAR *passwd)
{
    int binSize;
    FILE *f;
    char *gameBuf;
    QSP_CHAR *textData;
    if (!qspLoadTextFile(inFile, isUnicode, &textData)) return QSP_FALSE;
    /* Call 1: get required buffer size */
    binSize = t2gTextToGame(textData, locStart, locEnd, isOldFormat, isUnicode, passwd, 0, 0);
    if (binSize < 0)
    {
        free(textData);
        return QSP_FALSE;
    }
    gameBuf = (char *)malloc(binSize);
    if (!gameBuf)
    {
        free(textData);
        return QSP_FALSE;
    }
    /* Call 2: fill buffer */
    t2gTextToGame(textData, locStart, locEnd, isOldFormat, isUnicode, passwd, gameBuf, binSize);
    free(textData);
    if (!(f = fopen(outFile, "wb")))
    {
        free(gameBuf);
        return QSP_FALSE;
    }
    fwrite(gameBuf, 1, binSize, f);
    fclose(f);
    free(gameBuf);
    return QSP_TRUE;
}

static QSP_BOOL qspDecodeGameToText(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_CHAR *passwd)
{
    int textSize;
    long fileSize;
    char *binData;
    QSP_CHAR *textBuf;
    FILE *f;
    if (!(f = fopen(inFile, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    binData = (char *)malloc((size_t)fileSize);
    fseek(f, 0, SEEK_SET);
    fread(binData, 1, (size_t)fileSize, f);
    fclose(f);
    /* Call 1: get required text buffer size */
    textSize = t2gGameToText(binData, fileSize, passwd, locStart, locEnd, 0, 0);
    if (textSize < 0)
    {
        free(binData);
        return QSP_FALSE;
    }
    textBuf = (QSP_CHAR *)malloc(textSize * sizeof(QSP_CHAR));
    if (!textBuf)
    {
        free(binData);
        return QSP_FALSE;
    }
    /* Call 2: fill buffer */
    t2gGameToText(binData, fileSize, passwd, locStart, locEnd, textBuf, textSize);
    free(binData);
    {
        QSP_BOOL res = qspSaveTextFile(outFile, textBuf, isUnicode);
        free(textBuf);
        return res;
    }
}

void ShowHelp()
{
    qspPrint("TXT2GAM utility, ver. %s\n", QSP_VER);
    qspPrint("Usage:\n");
    qspPrint("  txt2gam [input file] [output file] [options]\n");
    qspPrint("Options:\n");
    qspPrint("  a, A - ANSI mode, default is Unicode (UTF-8, UTF-16) mode\n");
    qspPrint("  o, O - Save game in old format, default is new format\n");
    qspPrint("  s[string], S[string] - 'Start of loc' prefix, default is '%s'\n", T2G_STARTLOC);
    qspPrint("  e[string], E[string] - 'End of loc' prefix, default is '%s'\n", T2G_ENDLOC);
    qspPrint("  p[pass], P[pass] - Password, default is '%s'\n", T2G_PASSWD);
    qspPrint("  c, C - Encode text file into game file (default mode)\n");
    qspPrint("  d, D - Decode game file into text file\n");
    qspPrint("  t, T - Extract strings from text\n");
    qspPrint("  q, Q - Extract q-strings from text\n");
    qspPrint("Examples:\n");
    qspPrint("  txt2gam file.txt gamefile.qsp pMyPassword\n");
    qspPrint("  txt2gam file.txt gamefile.qsp\n");
    qspPrint("  txt2gam file.txt gamefile.qsp a\n");
    qspPrint("  txt2gam gamefile.qsp file.txt d pMyPassword\n");
    qspPrint("  txt2gam file.txt gamefile.qsp o pMyPassword\n");
    qspPrint("  txt2gam file.txt gamefile.qsp o e@ pMyPassword\n");
    qspPrint("  txt2gam file.txt gamefile.qsp o \"pMy Password\"\n");
    qspPrint("  txt2gam file.txt gamefile.qsp a o \"pMy Password\"\n");
    qspPrint("  txt2gam file.txt gamefile.qsp o\n");
    qspPrint("  txt2gam file.txt gamefile.qsp o e@\n");
    qspPrint("  txt2gam file.txt gamefile.qsp s@ e~\n");
    qspPrint("  txt2gam file.txt strsfile.txt t a\n");
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
    qspAddText(&locStart, T2G_STARTLOC, 0, -1, QSP_TRUE);
    qspAddText(&locEnd, T2G_ENDLOC, 0, -1, QSP_TRUE);
    qspAddText(&passwd, T2G_PASSWD, 0, -1, QSP_TRUE);
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
                    locStart = qspUTF8ToQSPString(argv[i] + 1, -1);
                    if (!locStart)
                    {
                        qspPrint("Loc start symbol is invalid\n");
                        workMode = QSP_ERROR;
                    }
                    break;
                case 'e': case 'E':
                    free(locEnd);
                    locEnd = qspUTF8ToQSPString(argv[i] + 1, -1);
                    if (!locEnd)
                    {
                        qspPrint("Loc end symbol is invalid\n");
                        workMode = QSP_ERROR;
                    }
                    break;
                case 'p': case 'P':
                    free(passwd);
                    passwd = qspUTF8ToQSPString(argv[i] + 1, -1);
                    if (!passwd)
                    {
                        qspPrint("Password is invalid\n");
                        workMode = QSP_ERROR;
                    }
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
        if (workMode == QSP_ERROR)
            break;
    }
    if (workMode == QSP_ERROR)
    {
        if (locStart) free(locStart);
        if (locEnd) free(locEnd);
        if (passwd) free(passwd);
        return 1;
    }
    if (!t2gInit())
    {
        qspPrint("Can't initialize the loc processor!\n");
        return 1;
    }
    isErr = QSP_FALSE;
    switch (workMode)
    {
        case QSP_EXTRACT_STRINGS:
        case QSP_EXTRACT_QSTRINGS:
            if (isErr = !qspExportStrings(argv[1], argv[2], locStart, locEnd, workMode == QSP_EXTRACT_QSTRINGS, isUnicode))
                qspPrint("String extraction has failed!\n");
            break;
        case QSP_ENCODE_INTO_GAME:
            if (isErr = !qspEncodeTextToGame(argv[1], argv[2], isUnicode, locStart, locEnd, isOldFormat, passwd))
                qspPrint("Encoding text to game has failed!\n");
            break;
        case QSP_DECODE_INTO_TEXT:
            if (isErr = !qspDecodeGameToText(argv[1], argv[2], isUnicode, locStart, locEnd, passwd))
                qspPrint("Decoding game to text has failed!\n");
            break;
    }
    t2gTerminate();
    free(locStart);
    free(locEnd);
    free(passwd);
    return (isErr == QSP_TRUE);
}
