/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * Txt2gam — JavaScript API class injected into the Emscripten module via
 * --post-js. All WASM heap allocation and deallocation is handled internally.
 *
 * QSP_CHAR is char16_t (a fixed 2-byte UTF-16LE code unit) on every platform.
 *
 * Usage:
 *   const module = await createT2gModule();
 *   const t2g = new module.Txt2gam();
 *
 *   // JS string → binary game data
 *   const gameBytes = t2g.textToGame(textString, null, null, false, false, null);
 *
 *   // binary game data → JS string
 *   const text = t2g.gameToText(gameUint8Array, null, null, null);
 *
 *   t2g.destroy(); // releases library resources
 */

(function () {
    /**
     * Copy a JS string into a fresh malloc'd null-terminated QSP_CHAR (UTF-16LE)
     * buffer on the WASM heap. Returns 0 when str is null (treated as "use default").
     */
    function allocStr(str) {
        if (!str) return 0;
        /* lengthBytesUTF16 returns byte count excluding the null terminator. */
        var byteLen = Module.lengthBytesUTF16(str) + 2; /* +2 for null char16_t */
        var ptr = Module._malloc(byteLen);
        Module.stringToUTF16(str, ptr, byteLen);
        return ptr;
    }

    /**
     * Copy a Uint8Array into a fresh malloc'd buffer on the WASM heap.
     */
    function allocBytes(bytes) {
        var ptr = Module._malloc(bytes.length);
        Module.HEAPU8.set(bytes, ptr);
        return ptr;
    }

    /**
     * Copy len bytes from the WASM heap at ptr into a new Uint8Array, then
     * free the WASM buffer.
     */
    function consumeBytes(ptr, len) {
        var result = Module.HEAPU8.slice(ptr, ptr + len);
        Module._free(ptr);
        return result;
    }

    Module['Txt2gam'] = /** @class */ (function () {
        function Txt2gam() {
            Module._t2gInit();
        }

        /** Release all library resources. Call when done. */
        Txt2gam.prototype.destroy = function () {
            Module._t2gTerminate();
        };

        /**
         * Parse raw text bytes (with optional BOM) into a JS string.
         * Detects UTF-16 LE BOM (FF FE), UTF-8 BOM (EF BB BF), or falls back
         * to UTF-8 / ANSI depending on isUnicode.
         *
         * @param {Uint8Array} textBytes   Raw text file bytes.
         * @param {boolean} isUnicode      True = UTF-8 fallback, false = ANSI fallback.
         * @returns {string|null}          Parsed text, or null on error.
         */
        Txt2gam.prototype.parseText = function (textBytes, isUnicode) {
            var dataPtr = allocBytes(textBytes);
            var resultPtr = Module._t2gParseTextData(dataPtr, textBytes.length, isUnicode ? 1 : 0);
            Module._free(dataPtr);
            if (!resultPtr) return null;
            /* Result is a null-terminated QSP_CHAR (char16_t = 2 bytes) string. */
            var i = resultPtr >> 1; /* HEAPU16 index */
            var end = i;
            while (Module.HEAPU16[end]) end++;
            var bytes = Module.HEAPU8.slice(resultPtr, resultPtr + (end - i) * 2);
            var result = new TextDecoder('utf-16le').decode(bytes);
            Module._free(resultPtr);
            return result;
        };

        /**
         * Convert a text source to QSP binary game data.
         *
         * @param {string} text            Source text.
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @param {boolean} isOldFormat    True to save in old QSP format.
         * @param {boolean} isUnicode      True to encode game in Unicode.
         * @param {string|null} password   Password, or null for "No".
         * @returns {Uint8Array|null}      Binary game data, or null on error.
         */
        Txt2gam.prototype.textToGame = function (text, locStart, locEnd, isOldFormat, isUnicode, password) {
            var textPtr     = allocStr(text);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);
            var passwdPtr   = allocStr(password);
            var outLenPtr   = Module._malloc(4);

            var resultPtr = Module._t2gEncodeTextToGame(
                textPtr, locStartPtr, locEndPtr,
                isOldFormat ? 1 : 0, isUnicode ? 1 : 0,
                passwdPtr, outLenPtr
            );

            Module._free(textPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);
            Module._free(passwdPtr);

            if (!resultPtr) {
                Module._free(outLenPtr);
                return null;
            }

            var outLen = Module.HEAP32[outLenPtr >> 2];
            Module._free(outLenPtr);

            return consumeBytes(resultPtr, outLen);
        };

        /**
         * Convert QSP binary game data to a text string.
         *
         * @param {Uint8Array} gameBytes   Raw QSP game file bytes.
         * @param {string|null} password   Password, or null for "No".
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @returns {string|null}          Decoded text, or null on error.
         */
        Txt2gam.prototype.gameToText = function (gameBytes, password, locStart, locEnd) {
            var dataPtr     = allocBytes(gameBytes);
            var passwdPtr   = allocStr(password);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);
            var outLenPtr   = Module._malloc(4);

            var resultPtr = Module._t2gDecodeGameToText(
                dataPtr, gameBytes.length,
                passwdPtr, locStartPtr, locEndPtr,
                outLenPtr
            );

            Module._free(dataPtr);
            Module._free(passwdPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);

            if (!resultPtr) {
                Module._free(outLenPtr);
                return null;
            }

            /* outLen is in QSP_CHAR units (char16_t = 2 bytes); exclude null terminator. */
            var outLen = Module.HEAP32[outLenPtr >> 2];
            Module._free(outLenPtr);

            var bytes = Module.HEAPU8.slice(resultPtr, resultPtr + (outLen - 1) * 2);
            var result = new TextDecoder('utf-16le').decode(bytes);
            Module._free(resultPtr);
            return result;
        };

        /**
         * Extract the in-game strings (or q-strings) from a text source.
         *
         * @param {string} text            Source text.
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @param {boolean} toGetQStrings  True to extract q-strings, false for strings.
         * @returns {string|null}          Extracted strings (possibly empty), or null on error.
         */
        Txt2gam.prototype.extractStrings = function (text, locStart, locEnd, toGetQStrings) {
            var textPtr     = allocStr(text);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);
            var outLenPtr   = Module._malloc(4);

            var resultPtr = Module._t2gExtractStrings(
                textPtr, locStartPtr, locEndPtr,
                toGetQStrings ? 1 : 0, outLenPtr
            );

            Module._free(textPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);

            if (!resultPtr) {
                Module._free(outLenPtr);
                return null;
            }

            /* outLen is in QSP_CHAR units (char16_t = 2 bytes); exclude null terminator. */
            var outLen = Module.HEAP32[outLenPtr >> 2];
            Module._free(outLenPtr);

            var bytes = Module.HEAPU8.slice(resultPtr, resultPtr + (outLen - 1) * 2);
            var result = new TextDecoder('utf-16le').decode(bytes);
            Module._free(resultPtr);
            return result;
        };

        return Txt2gam;
    }());
}());
