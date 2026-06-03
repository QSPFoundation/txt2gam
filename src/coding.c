/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "coding.h"
#include "text.h"

char16_t qspCP1251ToUTF16LETable[] =
{
    0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021,
    0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
    0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
    0x0020, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
    0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7,
    0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
    0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7,
    0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E, 0x041F,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 0x042F,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E, 0x043F,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 0x044F
};

static int qspUTF16StrLen(char *);
static int qspAddGameText(char **, char *, QSP_BOOL, int, int, QSP_BOOL);

static int qspUTF16StrLen(char *str)
{
    char16_t *ptr = (char16_t *)str;
    while (*ptr) ++ptr;
    return (int)(ptr - (char16_t *)str);
}

char16_t qspDirectConvertUC(char ch, char16_t *table)
{
    unsigned char ch2 = (unsigned char)ch;
    return (ch2 >= 0x80 ? table[ch2 - 0x80] : ch);
}

char qspReverseConvertUC(char16_t ch, char16_t *table)
{
    int i;
    if (ch < 0x80) return (char)ch;
    for (i = 127; i >= 0; --i)
        if (table[i] == ch) return (char)(i + 0x80);
    return 0x20;
}

char *qspQSPToGameString(QSP_CHAR *s, int len, QSP_BOOL isUnicode, QSP_BOOL isCode)
{
    char16_t uCh, *ptr;
    char ch, *ret;
    if (len < 0) len = qspStrLen(s);
    ret = (char *)malloc((len + 1) * (isUnicode ? 2 : 1));
    if (isUnicode)
    {
        ptr = (char16_t *)ret;
        ptr[len] = 0;
        if (isCode)
        {
            while (--len >= 0)
            {
                uCh = QSP_TO_GAME_UC(s[len]);
                if (uCh == QSP_CODREMOV)
                    uCh = (char16_t)-QSP_CODREMOV;
                else
                    uCh -= QSP_CODREMOV;
                ptr[len] = uCh;
            }
        }
        else
        {
            while (--len >= 0)
                ptr[len] = QSP_TO_GAME_UC(s[len]);
        }
    }
    else
    {
        ret[len] = 0;
        if (isCode)
        {
            while (--len >= 0)
            {
                ch = QSP_TO_GAME_SB(s[len]);
                if (ch == QSP_CODREMOV)
                    ch = -QSP_CODREMOV;
                else
                    ch -= QSP_CODREMOV;
                ret[len] = ch;
            }
        }
        else
        {
            while (--len >= 0)
                ret[len] = QSP_TO_GAME_SB(s[len]);
        }
    }
    return ret;
}

QSP_CHAR *qspGameToQSPString(char *s, int len, QSP_BOOL isUnicode, QSP_BOOL isCoded)
{
    char ch;
    char16_t uCh, *ptr;
    QSP_CHAR *ret;
    if (len < 0) len = (isUnicode ? qspUTF16StrLen(s) : (int)strlen(s));
    ret = (QSP_CHAR *)malloc((len + 1) * sizeof(QSP_CHAR));
    ret[len] = 0;
    if (isUnicode)
    {
        ptr = (char16_t *)s;
        if (isCoded)
        {
            while (--len >= 0)
            {
                uCh = ptr[len];
                if (uCh == (char16_t)-QSP_CODREMOV)
                    uCh = QSP_CODREMOV;
                else
                    uCh += QSP_CODREMOV;
                ret[len] = QSP_FROM_GAME_UC(uCh);
            }
        }
        else
        {
            while (--len >= 0)
                ret[len] = QSP_FROM_GAME_UC(ptr[len]);
        }
    }
    else
    {
        if (isCoded)
        {
            while (--len >= 0)
            {
                ch = s[len];
                if (ch == (char)-QSP_CODREMOV)
                    ch = QSP_CODREMOV;
                else
                    ch += QSP_CODREMOV;
                ret[len] = QSP_FROM_GAME_SB(ch);
            }
        }
        else
        {
            while (--len >= 0)
                ret[len] = QSP_FROM_GAME_SB(s[len]);
        }
    }
    return ret;
}

int qspSplitGameData(char *data, int dataLen, QSP_BOOL isUnicode, char ***res)
{
    char *delimStr, **ret, *segStart = data, *dataEnd = data + dataLen, *pos;
    int charSize = (isUnicode ? 2 : 1);
    int delimSize, segSize, count = 0, bufSize = 8;
    delimStr = qspQSPToGameString(QSP_STRSDELIM, -1, isUnicode, QSP_FALSE);
    delimSize = QSP_LEN(QSP_STRSDELIM) * charSize;
    ret = (char **)malloc(bufSize * sizeof(char *));
    do
    {
        for (pos = segStart; pos + delimSize <= dataEnd; pos += charSize)
            if (!memcmp(pos, delimStr, delimSize)) break;
        if (pos + delimSize > dataEnd) pos = dataEnd;
        segSize = (int)(pos - segStart);
        if (count >= bufSize)
        {
            bufSize += 16;
            ret = (char **)realloc(ret, bufSize * sizeof(char *));
        }
        ret[count] = (char *)malloc(segSize + charSize);
        memcpy(ret[count], segStart, segSize);
        memset(ret[count] + segSize, 0, charSize);
        ++count;
        segStart = pos + delimSize;
    } while (pos != dataEnd);
    free(delimStr);
    *res = ret;
    return count;
}

static int qspAddGameText(char **dest, char *val, QSP_BOOL isUnicode, int destLen, int valLen, QSP_BOOL isCreate)
{
    char *destPtr;
    char16_t *destUTF16, *valUTF16;
    int ret, charSize = (isUnicode ? 2 : 1);
    if (valLen < 0) valLen = (isUnicode ? qspUTF16StrLen(val) : (int)strlen(val));
    if (!isCreate && *dest)
    {
        if (destLen < 0) destLen = (isUnicode ? qspUTF16StrLen(*dest) : (int)strlen(*dest));
        ret = destLen + valLen;
        destPtr = (char *)realloc(*dest, (ret + 1) * charSize);
        *dest = destPtr;
        destPtr += destLen * charSize;
    }
    else
    {
        ret = valLen;
        destPtr = (char *)malloc((ret + 1) * charSize);
        *dest = destPtr;
    }
    if (isUnicode)
    {
        valUTF16 = (char16_t *)val;
        destUTF16 = (char16_t *)destPtr;
        while (valLen && (*destUTF16++ = *valUTF16++)) --valLen;
        *destUTF16 = 0;
    }
    else
    {
        strncpy(destPtr, val, valLen);
        destPtr[valLen] = 0;
    }
    return ret;
}

int qspGameCodeWriteIntValLine(char **s, int len, int val, QSP_BOOL isUnicode, QSP_BOOL isCode)
{
    char *temp;
    QSP_CHAR buf[12];
    qspNumToStr(buf, val);
    temp = qspQSPToGameString(buf, -1, isUnicode, isCode);
    len = qspAddGameText(s, temp, isUnicode, len, -1, QSP_FALSE);
    free(temp);
    temp = qspQSPToGameString(QSP_STRSDELIM, -1, isUnicode, QSP_FALSE);
    len = qspAddGameText(s, temp, isUnicode, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
    free(temp);
    return len;
}

int qspGameCodeWriteVal(char **s, int len, QSP_CHAR *val, QSP_BOOL isUnicode, QSP_BOOL isCode)
{
    char *temp = qspQSPToGameString(val, -1, isUnicode, isCode);
    len = qspAddGameText(s, temp, isUnicode, len, -1, QSP_FALSE);
    free(temp);
    return len;
}

int qspGameCodeWriteValLine(char **s, int len, QSP_CHAR *val, QSP_BOOL isUnicode, QSP_BOOL isCode)
{
    char *temp;
    if (val)
    {
        len = qspGameCodeWriteVal(s, len, val, isUnicode, isCode);
    }
    temp = qspQSPToGameString(QSP_STRSDELIM, -1, isUnicode, QSP_FALSE);
    len = qspAddGameText(s, temp, isUnicode, len, QSP_LEN(QSP_STRSDELIM), QSP_FALSE);
    free(temp);
    return len;
}

char *qspQSPStringToUTF8(QSP_CHAR *s, int len)
{
    QSP_CHAR *ptr, *end;
    unsigned int codepoint = 0;
    int outLen = 0, maxLen;
    char *ret;
    if (len < 0) len = qspStrLen(s);
    maxLen = len * 4; /* we're very conservative here */
    ret = (char *)malloc(maxLen + 1);
    ret[maxLen] = 0;
    end = s + len;
    for (ptr = s; ptr < end; ++ptr)
    {
        if (*ptr >= 0xd800 && *ptr <= 0xdbff)
            codepoint = ((*ptr - 0xd800) << 10) + 0x10000;
        else
        {
            if (*ptr >= 0xdc00 && *ptr <= 0xdfff)
                codepoint |= *ptr - 0xdc00;
            else
                codepoint = *ptr;

            if (codepoint <= 0x7f)
                ret[outLen++] = (char)codepoint;
            else if (codepoint <= 0x7ff)
            {
                ret[outLen++] = (char)(0xc0 | ((codepoint >> 6) & 0x1f));
                ret[outLen++] = (char)(0x80 | (codepoint & 0x3f));
            }
            else if (codepoint <= 0xffff)
            {
                ret[outLen++] = (char)(0xe0 | ((codepoint >> 12) & 0x0f));
                ret[outLen++] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
                ret[outLen++] = (char)(0x80 | (codepoint & 0x3f));
            }
            else
            {
                /* Astral codepoint (from a UTF-16 surrogate pair): 4-byte UTF-8. */
                ret[outLen++] = (char)(0xf0 | ((codepoint >> 18) & 0x07));
                ret[outLen++] = (char)(0x80 | ((codepoint >> 12) & 0x3f));
                ret[outLen++] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
                ret[outLen++] = (char)(0x80 | (codepoint & 0x3f));
            }
            codepoint = 0;
        }
    }
    ret[outLen] = 0;
    return (char *)realloc(ret, outLen + 1);
}

QSP_CHAR *qspUTF8ToQSPString(char *s, int len)
{
    unsigned int codepoint = 0;
    int outLen = 0, maxLen;
    char *end;
    QSP_CHAR *ret;
    if (len < 0) len = (int)strlen(s);
    maxLen = len; /* we're very conservative here */
    ret = (QSP_CHAR *)malloc((maxLen + 1) * sizeof(QSP_CHAR));
    ret[maxLen] = 0;
    end = s + len;
    while (s < end)
    {
        unsigned char ch = (unsigned char)*s;
        if (ch <= 0x7f)
            codepoint = ch;
        else if (ch <= 0xbf)
            codepoint = (codepoint << 6) | (ch & 0x3f);
        else if (ch <= 0xdf)
            codepoint = ch & 0x1f;
        else if (ch <= 0xef)
            codepoint = ch & 0x0f;
        else
            codepoint = ch & 0x07;
        ++s;
        /* End of buffer counts as a non-continuation byte, flushing the codepoint. */
        if ((s >= end || (*s & 0xc0) != 0x80) && codepoint <= 0x10ffff)
        {
            if (codepoint > 0xffff)
            {
                /* Astral codepoint: encode as a UTF-16 surrogate pair. */
                codepoint -= 0x10000;
                ret[outLen++] = (QSP_CHAR)(0xd800 + (codepoint >> 10));
                ret[outLen++] = (QSP_CHAR)(0xdc00 + (codepoint & 0x03ff));
            }
            else if (codepoint < 0xd800 || codepoint >= 0xe000)
                ret[outLen++] = (QSP_CHAR)codepoint;
        }
    }
    ret[outLen] = 0;
    return (QSP_CHAR *)realloc(ret, (outLen + 1) * sizeof(QSP_CHAR));
}
