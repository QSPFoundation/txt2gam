/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef T2G_API_H
    #define T2G_API_H

    #include "t2g_types.h"

    #define T2G_APPNAME  "TXT2GAM"
    #define T2G_STARTLOC QSP_FMT("#")
    #define T2G_ENDLOC   QSP_FMT("--")
    #define T2G_PASSWD   QSP_FMT("No")
    #define T2G_UTF16BOM "\xFF\xFE"
    #define T2G_UTF8BOM  "\xEF\xBB\xBF"

    /* Initialize the text processor. Returns 1 on success, 0 on failure. */
    int t2gInit(void);

    /* Release all resources held by the processor. */
    void t2gTerminate(void);

    /*
     * Parse raw text bytes (with optional BOM) into a malloc'd null-terminated
     * QSP_CHAR string.
     *
     * Encoding is determined in this order:
     *  1. UTF-16 LE BOM (FF FE) → UTF-16 LE
     *  2. UTF-8 BOM (EF BB BF) → UTF-8
     *  3. Leading null byte heuristic → UTF-16 LE without BOM
     *  4. isUnicode fallback: QSP_TRUE → UTF-8, QSP_FALSE → ANSI/CP1251
     *
     * data      - raw text bytes
     * dataLen   - number of bytes in data
     * isUnicode - fallback when no BOM is found
     * outText   - receives a malloc'd QSP_CHAR string; caller must free
     * outLen    - receives the character count (incl. null terminator)
     *
     * Line endings are normalised to \n.
     * Returns T2G_ERROR_NONE on success.
     */
    int t2gParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode, QSP_CHAR **outText, int *outLen);

    /*
     * Encode a null-terminated QSP_CHAR string to raw text bytes.
     *
     * isUnicode  - QSP_TRUE: UTF-8 with UTF-8 BOM prepended
     *              QSP_FALSE: ANSI/CP1251, no BOM
     * outData    - receives a malloc'd char buffer; caller must free
     * outLen     - receives the byte count (without null terminator)
     *
     * Returns T2G_ERROR_NONE on success.
     */
    int t2gEncodeTextData(const QSP_CHAR *text, QSP_BOOL isUnicode, char **outData, int *outLen);

    /*
     * Convert a QSP_CHAR text source to QSP binary game data.
     *
     * text        - null-terminated QSP_CHAR source text (line endings must be
     *               normalised to \n, e.g. via t2gReadTextData)
     * locStart    - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd      - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * isOldFormat - QSP_TRUE to save in the old QSP format, QSP_FALSE for new
     * isUnicode   - QSP_TRUE to encode game strings as UTF-16, QSP_FALSE for ANSI/CP1251
     * password    - null-terminated password, or 0 for the default ("No")
     * outData     - receives a malloc'd buffer with the binary game data; caller must free
     * outLen      - receives the byte count of the returned buffer
     *
     * Returns T2G_ERROR_NONE on success.
     */
    int t2gEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart,
                             const QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUnicode,
                             const QSP_CHAR *password, char **outData, int *outLen);

    /*
     * Convert QSP binary game data to a null-terminated QSP_CHAR text source.
     *
     * data     - raw game file bytes (copied internally; not modified)
     * dataLen  - number of bytes in data
     * password - null-terminated password, or 0 for the default ("No")
     * locStart - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd   - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * outText  - receives a malloc'd null-terminated QSP_CHAR string; caller must free
     * outLen   - receives the character count of the returned buffer (incl. null
     *            terminator)
     *
     * Returns T2G_ERROR_NONE on success, T2G_ERROR_WRONG_PASSWORD if the password
     * does not match.
     */
    int t2gDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                             const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                             QSP_CHAR **outText, int *outLen);

    /*
     * Extract the in-game strings (or q-strings) from a QSP_CHAR text source.
     *
     * text         - null-terminated QSP_CHAR source text
     * locStart     - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd       - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * toGetQStrings - QSP_TRUE to extract q-strings, QSP_FALSE for regular strings
     * outText      - receives a malloc'd null-terminated QSP_CHAR string; caller must free
     * outLen       - receives the character count of the returned buffer (incl. null
     *                terminator)
     *
     * Returns T2G_ERROR_NONE on success.
     */
    int t2gExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                           const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings,
                           QSP_CHAR **outText, int *outLen);

#endif /* T2G_API_H */
