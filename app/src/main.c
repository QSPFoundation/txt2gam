/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "main.h"
#include "t2g.h"
#include "coding.h"
#include "locations.h"
#include "locdata.h"
#include "text.h"
#include "sys.h"

static QSP_BOOL qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data);
static QSP_BOOL qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode);
static QSP_BOOL qspSaveUTF8TextFile(char *file, const char *utf8Data, QSP_BOOL isUnicode);
static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode);
static QSP_BOOL qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd);
static QSP_BOOL qspDecodeGameToText(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_CHAR *passwd);

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

static QSP_BOOL qspSaveUTF8TextFile(char *file, const char *utf8Data, QSP_BOOL isUnicode)
{
    FILE *f;
    if (!(f = fopen(file, "wb")))
        return QSP_FALSE;
    if (isUnicode)
    {
        fwrite(TXT2GAM_UTF8BOM, 1, sizeof(TXT2GAM_UTF8BOM) - 1, f);
        fwrite(utf8Data, 1, strlen(utf8Data), f);
    }
    else
    {
        QSP_CHAR *qspStr = qspUTF8ToQSPString((char *)utf8Data);
        if (qspStr)
        {
            char *content = qspQSPToGameString(qspStr, QSP_FALSE, QSP_FALSE);
            free(qspStr);
            if (content)
            {
                fwrite(content, 1, strlen(content), f);
                free(content);
            }
        }
    }
    fclose(f);
    return QSP_TRUE;
}

static QSP_BOOL qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode)
{
    QSP_CHAR *data, *locsStrings;
    if (!qspLoadTextFile(file, isUnicode, &data)) return QSP_FALSE;
    locsStrings = qspGetLocsStrings(data, locStart, locEnd, toGetQStrings);
    free(data);
    if (locsStrings)
    {
        QSP_BOOL res = qspSaveTextFile(outFile, locsStrings, isUnicode);
        free(locsStrings);
        return res;
    }
    return QSP_TRUE;
}

static QSP_BOOL qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd)
{
    int binSize;
    FILE *f;
    char *utf8Text, *locStartUTF8, *locEndUTF8, *passwdUTF8, *gameBuf;
    QSP_CHAR *textData;
    if (!qspLoadTextFile(inFile, isUnicode, &textData)) return QSP_FALSE;
    utf8Text = qspQSPStringToUTF8(textData);
    free(textData);
    if (!utf8Text) return QSP_FALSE;
    locStartUTF8 = qspQSPStringToUTF8(locStart);
    locEndUTF8 = qspQSPStringToUTF8(locEnd);
    passwdUTF8 = qspQSPStringToUTF8(passwd);
    if (!locStartUTF8 || !locEndUTF8 || !passwdUTF8)
    {
        free(utf8Text);
        if (locStartUTF8) free(locStartUTF8);
        if (locEndUTF8) free(locEndUTF8);
        if (passwdUTF8) free(passwdUTF8);
        return QSP_FALSE;
    }
    /* Call 1: get required buffer size */
    binSize = t2gTextToGame(utf8Text, locStartUTF8, locEndUTF8, isOldFormat, isUCS2, passwdUTF8, 0);
    if (binSize < 0)
    {
        free(utf8Text);
        free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
        return QSP_FALSE;
    }
    gameBuf = (char *)malloc(binSize);
    if (!gameBuf)
    {
        free(utf8Text);
        free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
        return QSP_FALSE;
    }
    /* Call 2: fill buffer */
    t2gTextToGame(utf8Text, locStartUTF8, locEndUTF8, isOldFormat, isUCS2, passwdUTF8, gameBuf);
    free(utf8Text);
    free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
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
    char *binData, *textBuf, *locStartUTF8, *locEndUTF8, *passwdUTF8;
    FILE *f;
    if (!(f = fopen(inFile, "rb"))) return QSP_FALSE;
    fseek(f, 0, SEEK_END);
    fileSize = ftell(f);
    binData = (char *)malloc((size_t)fileSize);
    fseek(f, 0, SEEK_SET);
    fread(binData, 1, (size_t)fileSize, f);
    fclose(f);
    locStartUTF8 = qspQSPStringToUTF8(locStart);
    locEndUTF8 = qspQSPStringToUTF8(locEnd);
    passwdUTF8 = qspQSPStringToUTF8(passwd);
    if (!locStartUTF8 || !locEndUTF8 || !passwdUTF8)
    {
        free(binData);
        if (locStartUTF8) free(locStartUTF8);
        if (locEndUTF8) free(locEndUTF8);
        if (passwdUTF8) free(passwdUTF8);
        return QSP_FALSE;
    }
    /* Call 1: get required text buffer size */
    textSize = t2gGameToText(binData, (int)fileSize, passwdUTF8, locStartUTF8, locEndUTF8, 0);
    if (textSize < 0)
    {
        free(binData);
        free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
        return QSP_FALSE;
    }
    textBuf = (char *)malloc(textSize);
    if (!textBuf)
    {
        free(binData);
        free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
        return QSP_FALSE;
    }
    /* Call 2: fill buffer */
    t2gGameToText(binData, (int)fileSize, passwdUTF8, locStartUTF8, locEndUTF8, textBuf);
    free(binData);
    free(locStartUTF8); free(locEndUTF8); free(passwdUTF8);
    {
        QSP_BOOL res = qspSaveUTF8TextFile(outFile, textBuf, isUnicode);
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
    qspPrint("  a, A - ANSI mode, default is Unicode (UTF-8, UCS-2/UTF-16) mode\n");
    qspPrint("  o, O - Save game in old format, default is new format\n");
    qspPrint("  s[string], S[string] - 'Start of loc' prefix, default is '%s'\n", QSP_STARTLOC);
    qspPrint("  e[string], E[string] - 'End of loc' prefix, default is '%s'\n", QSP_ENDLOC);
    qspPrint("  p[pass], P[pass] - Password, default is '%s'\n", QSP_PASSWD);
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
                    if (!locStart)
                    {
                        qspPrint("Loc start symbol is invalid\n");
                        workMode = QSP_ERROR;
                    }
                    break;
                case 'e': case 'E':
                    free(locEnd);
                    locEnd = qspToQSPString(argv[i] + 1);
                    if (!locEnd)
                    {
                        qspPrint("Loc end symbol is invalid\n");
                        workMode = QSP_ERROR;
                    }
                    break;
                case 'p': case 'P':
                    free(passwd);
                    passwd = qspToQSPString(argv[i] + 1);
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
            if (isErr = !qspEncodeTextToGame(argv[1], argv[2], isUnicode, locStart, locEnd, isOldFormat, isUnicode, passwd))
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
