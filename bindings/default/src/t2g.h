/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef T2G_API_H
    #define T2G_API_H

    #ifdef __cplusplus
    extern "C" {
    #endif

    /* Initialize the text processor. Returns 1 on success, 0 on failure. */
    int t2gInit(void);

    /* Release all resources held by the processor. */
    void t2gTerminate(void);

    /*
     * Convert a text game source to a QSP binary game file using a 2-call approach.
     *
     * text     - null-terminated UTF-8 source (optional UTF-8 or UCS-2 BOM is detected
     *            automatically)
     * locStart - null-terminated UTF-8 location-start marker, or NULL for the default ("#")
     * locEnd   - null-terminated UTF-8 location-end marker, or NULL for the default ("--")
     * isOldFormat - 1 to save in the old QSP format, 0 for the new format
     * isUCS2   - 1 to encode game strings as UCS-2, 0 for UTF-8
     * password - null-terminated UTF-8 password, or NULL for the default ("No")
     * outBuf   - NULL on the first call (size query); on the second call a caller-allocated
     *            buffer of at least the size returned by the first call
     *
     * Returns the required (call 1) or written (call 2) buffer size in bytes,
     * or -1 on error.
     */
    int t2gTextToGame(const char *text, const char *locStart, const char *locEnd,
                      int isOldFormat, int isUCS2, const char *password, char *outBuf);

    /*
     * Convert a QSP binary game file to a UTF-8 text game source using a 2-call approach.
     *
     * data     - raw game file bytes
     * dataLen  - number of bytes in data
     * password - null-terminated UTF-8 password, or NULL for the default ("No")
     * locStart - null-terminated UTF-8 location-start marker, or NULL for the default ("#")
     * locEnd   - null-terminated UTF-8 location-end marker, or NULL for the default ("--")
     * outBuf   - NULL on the first call (size query); on the second call a caller-allocated
     *            buffer of at least the size returned by the first call
     *
     * Returns the required (call 1) or written (call 2) buffer size in bytes including the
     * null terminator, or -1 on error.
     */
    int t2gGameToText(const char *data, int dataLen, const char *password,
                      const char *locStart, const char *locEnd, char *outBuf);

    #ifdef __cplusplus
    }
    #endif

#endif
