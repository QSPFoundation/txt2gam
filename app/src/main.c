/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "t2g_default.h"

enum Mode
{
    QSP_ERROR,
    QSP_SHOW_HELP,
    QSP_ENCODE_INTO_GAME,
    QSP_DECODE_INTO_TEXT,
    QSP_EXTRACT_STRINGS,
    QSP_EXTRACT_QSTRINGS
};

static void qspPrintError(const char *operation, int err);
static int qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **data);
static int qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode);
static int qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode);
static int qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_CHAR *passwd);
static int qspDecodeGameToText(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_CHAR *passwd);

static void qspPrintError(const char *operation, int err)
{
    switch (err)
    {
    case T2G_ERROR_WRONG_PASSWORD:
        t2gPrint("%s failed: wrong password!\n", operation);
        break;
    case T2G_ERROR_INVALID_DATA:
        t2gPrint("%s failed: invalid or corrupt data!\n", operation);
        break;
    case T2G_ERROR_NO_MEMORY:
        t2gPrint("%s failed: out of memory!\n", operation);
        break;
    default:
        t2gPrint("%s failed!\n", operation);
        break;
    }
}

static int qspLoadTextFile(char *file, QSP_BOOL isUnicode, QSP_CHAR **outData)
{
    int err, fileSize, textSize;
    char *buf;
    QSP_CHAR *textBuf;
    FILE *f;
    if (!(f = fopen(file, "rb"))) return T2G_ERROR_FAILED;
    fseek(f, 0, SEEK_END);
    fileSize = (int)ftell(f);
    if (fileSize < 0)
    {
        fclose(f);
        return T2G_ERROR_FAILED;
    }
    buf = (char *)malloc(fileSize);
    if (!buf)
    {
        fclose(f);
        return T2G_ERROR_NO_MEMORY;
    }
    fseek(f, 0, SEEK_SET);
    fread(buf, 1, fileSize, f);
    fclose(f);
    /* Call 1: get required buffer size */
    err = t2gReadTextData(buf, fileSize, isUnicode, 0, 0, &textSize);
    if (err != T2G_ERROR_NONE)
    {
        free(buf);
        return err;
    }
    textBuf = (QSP_CHAR *)malloc(textSize * sizeof(QSP_CHAR));
    if (!textBuf)
    {
        free(buf);
        return T2G_ERROR_NO_MEMORY;
    }
    /* Call 2: fill buffer */
    t2gReadTextData(buf, fileSize, isUnicode, textBuf, textSize, 0);
    free(buf);
    *outData = textBuf;
    return T2G_ERROR_NONE;
}

static int qspSaveTextFile(char *file, QSP_CHAR *data, QSP_BOOL isUnicode)
{
    FILE *f;
    int err, size;
    char *buf;
    err = t2gWriteTextData(data, isUnicode, 0, 0, &size);
    if (err != T2G_ERROR_NONE) return err;
    buf = (char *)malloc(size);
    if (!buf) return T2G_ERROR_NO_MEMORY;
    t2gWriteTextData(data, isUnicode, buf, size, 0);
    if (!(f = fopen(file, "wb")))
    {
        free(buf);
        return T2G_ERROR_FAILED;
    }
    fwrite(buf, 1, size, f);
    fclose(f);
    free(buf);
    return T2G_ERROR_NONE;
}

static int qspExportStrings(char *file, char *outFile, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, QSP_BOOL isUnicode)
{
    QSP_CHAR *data, *strBuf;
    int err, len;
    err = qspLoadTextFile(file, isUnicode, &data);
    if (err != T2G_ERROR_NONE) return err;
    /* Call 1: get required buffer size */
    err = t2gTextToStrings(data, locStart, locEnd, toGetQStrings, 0, 0, &len);
    if (err != T2G_ERROR_NONE)
    {
        free(data);
        return err;
    }
    strBuf = (QSP_CHAR *)malloc(len * sizeof(QSP_CHAR));
    if (!strBuf)
    {
        free(data);
        return T2G_ERROR_NO_MEMORY;
    }
    /* Call 2: fill buffer */
    t2gTextToStrings(data, locStart, locEnd, toGetQStrings, strBuf, len, 0);
    free(data);
    err = qspSaveTextFile(outFile, strBuf, isUnicode);
    free(strBuf);
    return err;
}

static int qspEncodeTextToGame(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_CHAR *passwd)
{
    int err, binSize;
    FILE *f;
    char *gameBuf;
    QSP_CHAR *textData;
    err = qspLoadTextFile(inFile, isUnicode, &textData);
    if (err != T2G_ERROR_NONE) return err;
    /* Call 1: get required buffer size */
    err = t2gTextToGame(textData, locStart, locEnd, isOldFormat, isUnicode, passwd, 0, 0, &binSize);
    if (err != T2G_ERROR_NONE)
    {
        free(textData);
        return err;
    }
    gameBuf = (char *)malloc(binSize);
    if (!gameBuf)
    {
        free(textData);
        return T2G_ERROR_NO_MEMORY;
    }
    /* Call 2: fill buffer */
    t2gTextToGame(textData, locStart, locEnd, isOldFormat, isUnicode, passwd, gameBuf, binSize, 0);
    free(textData);
    if (!(f = fopen(outFile, "wb")))
    {
        free(gameBuf);
        return T2G_ERROR_FAILED;
    }
    fwrite(gameBuf, 1, binSize, f);
    fclose(f);
    free(gameBuf);
    return T2G_ERROR_NONE;
}

static int qspDecodeGameToText(char *inFile, char *outFile, QSP_BOOL isUnicode, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_CHAR *passwd)
{
    int err, textSize, fileSize;
    char *binData;
    QSP_CHAR *textBuf;
    FILE *f;
    if (!(f = fopen(inFile, "rb"))) return T2G_ERROR_FAILED;
    fseek(f, 0, SEEK_END);
    fileSize = (int)ftell(f);
    if (fileSize < 0)
    {
        fclose(f);
        return T2G_ERROR_FAILED;
    }
    binData = (char *)malloc(fileSize);
    if (!binData)
    {
        fclose(f);
        return T2G_ERROR_NO_MEMORY;
    }
    fseek(f, 0, SEEK_SET);
    fread(binData, 1, fileSize, f);
    fclose(f);
    /* Call 1: get required text buffer size */
    err = t2gGameToText(binData, fileSize, passwd, locStart, locEnd, 0, 0, &textSize);
    if (err != T2G_ERROR_NONE)
    {
        free(binData);
        return err;
    }
    textBuf = (QSP_CHAR *)malloc(textSize * sizeof(QSP_CHAR));
    if (!textBuf)
    {
        free(binData);
        return T2G_ERROR_NO_MEMORY;
    }
    /* Call 2: fill buffer */
    t2gGameToText(binData, fileSize, passwd, locStart, locEnd, textBuf, textSize, 0);
    free(binData);
    err = qspSaveTextFile(outFile, textBuf, isUnicode);
    free(textBuf);
    return err;
}

static void qspShowHelp(void)
{
    t2gPrint("TXT2GAM utility, ver. %S\n", T2G_VER);
    t2gPrint("Usage:\n");
    t2gPrint("  txt2gam [options] <input> <output>\n");
    t2gPrint("Options:\n");
    t2gPrint("  -h          Show this help\n");
    t2gPrint("  -c          Encode text file into game file (default)\n");
    t2gPrint("  -d          Decode game file into text file\n");
    t2gPrint("  -t          Extract strings from text file\n");
    t2gPrint("  -q          Extract q-strings from text file\n");
    t2gPrint("  -o          Save game in old format (default: new format)\n");
    t2gPrint("  -a          ANSI mode (default: Unicode / UTF-8 / UTF-16)\n");
    t2gPrint("  -s <value>  'Start of loc' prefix (default: '%S')\n", T2G_STARTLOC);
    t2gPrint("  -e <value>  'End of loc' prefix (default: '%S')\n", T2G_ENDLOC);
    t2gPrint("  -p <value>  Password (default: '%S')\n", T2G_PASSWD);
    t2gPrint("Examples:\n");
    t2gPrint("  txt2gam file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -a file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -o -p MyPassword file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -d -p MyPassword gamefile.qsp file.txt\n");
    t2gPrint("  txt2gam -o -e @ file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -s @ -e ~ file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -p \"My Password\" file.txt gamefile.qsp\n");
    t2gPrint("  txt2gam -t -a file.txt strsfile.txt\n");
}

int main(int argc, char **argv)
{
    int lastOpErr, i, workMode;
    QSP_BOOL isOldFormat, isUnicode;
    QSP_CHAR *passwd, *locStart, *locEnd;
    char *inFile, *outFile;
    setlocale(LC_ALL, T2G_LOCALE);
    workMode = QSP_ENCODE_INTO_GAME;
    isOldFormat = QSP_FALSE;
    isUnicode = QSP_TRUE;
    inFile = 0;
    outFile = 0;
    locStart = 0;
    locEnd = 0;
    passwd = 0;
    for (i = 1; i < argc; ++i)
    {
        char opt;
        char *val;
        if (argv[i][0] == '-' && argv[i][1])
        {
            opt = argv[i][1];
            /* Value may be attached (-pFoo) or separate (-p Foo) */
            val = argv[i][2] ? argv[i] + 2 : 0;
        }
        else if (!inFile)
        {
            inFile = argv[i];
            continue;
        }
        else if (!outFile)
        {
            outFile = argv[i];
            continue;
        }
        else
        {
            /* Legacy no-dash format (deprecated): bare option after both filenames */
            opt = argv[i][0];
            val = argv[i][1] ? argv[i] + 1 : 0;
        }
        switch (opt)
        {
        case 'h': case 'H':
            workMode = QSP_SHOW_HELP;
            break;
        case 'o': case 'O':
            isOldFormat = QSP_TRUE;
            break;
        case 'a': case 'A':
            isUnicode = QSP_FALSE;
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
        case 's': case 'S':
        case 'e': case 'E':
        case 'p': case 'P':
            if (!val)
            {
                if (i + 1 >= argc)
                {
                    t2gPrint("Option %c requires an argument\n", opt);
                    workMode = QSP_ERROR;
                    break;
                }
                val = argv[++i];
            }
            switch (opt)
            {
            case 's': case 'S':
                t2gFreeData(locStart);
                locStart = t2gUTF8ToQSPString(val, -1);
                if (!locStart) { t2gPrint("Loc start value is invalid\n"); workMode = QSP_ERROR; }
                break;
            case 'e': case 'E':
                t2gFreeData(locEnd);
                locEnd = t2gUTF8ToQSPString(val, -1);
                if (!locEnd) { t2gPrint("Loc end value is invalid\n"); workMode = QSP_ERROR; }
                break;
            case 'p': case 'P':
                t2gFreeData(passwd);
                passwd = t2gUTF8ToQSPString(val, -1);
                if (!passwd) { t2gPrint("Password is invalid\n"); workMode = QSP_ERROR; }
                break;
            }
            break;
        default:
            t2gPrint("Unknown option: %c\n", opt);
            workMode = QSP_ERROR;
            break;
        }
        if (workMode == QSP_ERROR)
            break;
    }
    if (workMode != QSP_ERROR && (workMode == QSP_SHOW_HELP || !inFile || !outFile))
    {
        qspShowHelp();
        t2gFreeData(locStart);
        t2gFreeData(locEnd);
        t2gFreeData(passwd);
        return 0;
    }
    if (workMode == QSP_ERROR)
    {
        t2gFreeData(locStart);
        t2gFreeData(locEnd);
        t2gFreeData(passwd);
        return 1;
    }
    if (!t2gInit())
    {
        t2gPrint("Can't initialize the loc processor!\n");
        return 1;
    }
    lastOpErr = T2G_ERROR_NONE;
    switch (workMode)
    {
        case QSP_EXTRACT_STRINGS:
        case QSP_EXTRACT_QSTRINGS:
            lastOpErr = qspExportStrings(inFile, outFile, locStart, locEnd, workMode == QSP_EXTRACT_QSTRINGS, isUnicode);
            if (lastOpErr != T2G_ERROR_NONE) qspPrintError("String extraction", lastOpErr);
            break;
        case QSP_ENCODE_INTO_GAME:
            lastOpErr = qspEncodeTextToGame(inFile, outFile, isUnicode, locStart, locEnd, isOldFormat, passwd);
            if (lastOpErr != T2G_ERROR_NONE) qspPrintError("Encoding", lastOpErr);
            break;
        case QSP_DECODE_INTO_TEXT:
            lastOpErr = qspDecodeGameToText(inFile, outFile, isUnicode, locStart, locEnd, passwd);
            if (lastOpErr != T2G_ERROR_NONE) qspPrintError("Decoding", lastOpErr);
            break;
    }
    t2gTerminate();
    t2gFreeData(locStart);
    t2gFreeData(locEnd);
    t2gFreeData(passwd);
    return lastOpErr;
}
