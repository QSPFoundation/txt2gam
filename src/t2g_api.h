/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef T2G_API_H
    #define T2G_API_H

    #include "t2g_types.h"

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
     * Parse raw text bytes into a null-terminated QSP_CHAR string.
     *
     * Encoding is determined in this order:
     *  1. UTF-16 LE BOM (FF FE) → UTF-16 LE
     *  2. UTF-8 BOM (EF BB BF) → UTF-8
     *  3. Leading null byte heuristic → UTF-16 LE without BOM
     *  4. isUnicode fallback: QSP_TRUE → UTF-8, QSP_FALSE → ANSI/CP1251
     *
     * data      - raw text bytes
     * dataLen   - number of bytes in data
     * isUnicode - fallback when no BOM is found: QSP_TRUE = UTF-8, QSP_FALSE = ANSI
     *
     * Line endings are normalised to \n.
     * Returns a malloc'd null-terminated QSP_CHAR string. Caller must free.
     * Returns 0 on error.
     */
    QSP_CHAR *t2gParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode);

    /*
     * Convert a QSP_CHAR text source to QSP binary game data.
     *
     * text        - null-terminated QSP_CHAR source text
     * locStart    - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd      - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * isOldFormat - QSP_TRUE to save in the old QSP format, QSP_FALSE for new
     * isUnicode   - QSP_TRUE to encode game strings as UTF-16, QSP_FALSE for ANSI/CP1251
     * password    - null-terminated password, or 0 for the default ("No")
     * outLen      - receives the byte count of the returned buffer
     *
     * Returns a malloc'd buffer with the binary game data. Caller must free.
     * Returns 0 on error.
     */
    char *t2gEncodeTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart,
                              const QSP_CHAR *locEnd, QSP_BOOL isOldFormat, QSP_BOOL isUnicode,
                              const QSP_CHAR *password, int *outLen);

    /*
     * Convert QSP binary game data to a null-terminated QSP_CHAR text source.
     *
     * data     - raw game file bytes
     * dataLen  - number of bytes in data
     * password - null-terminated password, or 0 for the default ("No")
     * locStart - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd   - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * outLen   - receives the character count of the returned buffer (incl. null
     *            terminator)
     *
     * Returns a malloc'd null-terminated QSP_CHAR string. Caller must free.
     * Returns 0 on error.
     */
    QSP_CHAR *t2gDecodeGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                                  const QSP_CHAR *locStart, const QSP_CHAR *locEnd, int *outLen);

    /*
     * Extract the in-game strings (or q-strings) from a QSP_CHAR text source.
     *
     * text         - null-terminated QSP_CHAR source text
     * locStart     - null-terminated location-start marker, or 0 for T2G_STARTLOC ("#")
     * locEnd       - null-terminated location-end marker, or 0 for T2G_ENDLOC ("--")
     * toGetQStrings - QSP_TRUE to extract q-strings, QSP_FALSE for regular strings
     * outLen       - receives the character count of the returned buffer (incl. null
     *                terminator)
     *
     * Returns a malloc'd null-terminated QSP_CHAR string (possibly empty when no
     * strings are found). Caller must free. Returns 0 on error.
     */
    QSP_CHAR *t2gExtractStrings(const QSP_CHAR *text, const QSP_CHAR *locStart,
                               const QSP_CHAR *locEnd, QSP_BOOL toGetQStrings, int *outLen);

#endif /* T2G_API_H */
