/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef T2G_DEFAULT_H
    #define T2G_DEFAULT_H

    #include "t2g_types.h"

    #ifdef __cplusplus
    extern "C" {
    #endif

    /* Initialize the text processor. Returns 1 on success, 0 on failure. */
    int t2gInit(void);

    /* Release all resources held by the processor. */
    void t2gTerminate(void);

    /*
     * Parse raw text bytes (with optional BOM) into a null-terminated QSP_CHAR string.
     * See t2g_api.h / t2gParseTextData for full documentation.
     * Caller must free the returned pointer.
     */
    QSP_CHAR *t2gParseTextData(const char *data, int dataLen, QSP_BOOL isUnicode);

    /*
     * Convert a text game source to a QSP binary game file using a 2-call approach.
     *
     * data        - raw text bytes; encoding is auto-detected from BOM, with
     *               isUnicode as a fallback (see t2gParseTextData)
     * dataLen     - number of bytes in data
     * isUnicode   - fallback encoding hint: QSP_TRUE = UTF-8, QSP_FALSE = ANSI
     * locStart    - null-terminated location-start marker, or 0 for the default ("#")
     * locEnd      - null-terminated location-end marker, or 0 for the default ("--")
     * isOldFormat - QSP_TRUE to save in the old QSP format, QSP_FALSE for new
     * isUnicodeOut - QSP_TRUE to encode game strings as UTF-16, QSP_FALSE for ANSI
     * password    - null-terminated password, or 0 for the default ("No")
     * outBuf      - 0 on the first call (size query); on the second call a
     *               caller-allocated buffer of at least the size returned by the
     *               first call
     * outBufSize  - size of outBuf in bytes; ignored when outBuf is 0
     *
     * Returns the required (call 1) or written (call 2) buffer size in bytes,
     * or -1 on error. Returns the required size if outBuf is too small.
     */
    int t2gTextToGame(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                      QSP_BOOL isOldFormat, QSP_BOOL isUnicode, const QSP_CHAR *password,
                      char *outBuf, int outBufSize);

    /*
     * Convert a QSP binary game file to a QSP_CHAR text source using a 2-call approach.
     *
     * data        - raw game file bytes
     * dataLen     - number of bytes in data
     * password    - null-terminated password, or 0 for the default ("No")
     * locStart    - null-terminated location-start marker, or 0 for the default ("#")
     * locEnd      - null-terminated location-end marker, or 0 for the default ("--")
     * outBuf      - 0 on the first call (size query); on the second call a
     *               caller-allocated buffer of at least the size returned by the
     *               first call
     * outBufSize  - size of outBuf in QSP_CHAR units; ignored when outBuf is 0
     *
     * Returns the required (call 1) or written (call 2) buffer size in QSP_CHAR
     * units including the null terminator, or -1 on error. Returns the required
     * size if outBuf is too small.
     */
    int t2gGameToText(const char *data, int dataLen, const QSP_CHAR *password,
                      const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                      QSP_CHAR *outBuf, int outBufSize);

    /*
     * Extract the in-game strings (or q-strings) from a QSP_CHAR text source using
     * a 2-call approach.
     *
     * text         - null-terminated QSP_CHAR source text
     * locStart     - null-terminated location-start marker, or 0 for the default ("#")
     * locEnd       - null-terminated location-end marker, or 0 for the default ("--")
     * toGetQStrings - QSP_TRUE to extract q-strings, QSP_FALSE for regular strings
     * outBuf       - 0 on the first call (size query); on the second call a
     *                caller-allocated buffer of at least the size returned by the
     *                first call
     * outBufSize   - size of outBuf in QSP_CHAR units; ignored when outBuf is 0
     *
     * Returns the required (call 1) or written (call 2) buffer size in QSP_CHAR
     * units including the null terminator, or -1 on error. Returns the required
     * size if outBuf is too small.
     */
    int t2gTextToStrings(const QSP_CHAR *text, const QSP_CHAR *locStart, const QSP_CHAR *locEnd,
                         QSP_BOOL toGetQStrings, QSP_CHAR *outBuf, int outBufSize);

    #ifdef __cplusplus
    }
    #endif

#endif /* T2G_DEFAULT_H */
