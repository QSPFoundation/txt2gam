/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/**
 * Txt2gam — injected via --post-js into the Emscripten module.
 *
 *   const module = await createT2gModule();
 *   const t2g = new module.Txt2gam();
 *   const gameBytes = t2g.textToGame(text, null, null, false, false, null);
 *   const text = t2g.gameToText(gameUint8Array, null, null, null);
 *   t2g.destroy();
 */

(function () {
    /* Allocate a null-terminated UTF-16LE buffer for str. Returns 0 for null/undefined. */
    function allocStr(str) {
        if (str == null) return 0;
        var byteLen = Module.lengthBytesUTF16(str) + 2; /* +2 for null char16_t */
        var ptr = Module._malloc(byteLen);
        Module.stringToUTF16(str, ptr, byteLen);
        return ptr;
    }

    function allocBytes(bytes) {
        var ptr = Module._malloc(bytes.length);
        Module.HEAPU8.set(bytes, ptr);
        return ptr;
    }

    function consumeBytes(ptr, lenPtr) {
        if (!ptr) return null;
        var len = Module.HEAP32[lenPtr >> 2];
        var result = Module.HEAPU8.slice(ptr, ptr + len);
        Module._free(ptr);
        return result;
    }

    function consumeStr(ptr, lenPtr) {
        if (!ptr) return null;
        var len = Module.HEAP32[lenPtr >> 2];
        /* Decode straight from a heap view (no intermediate copy); the decode is
           synchronous and nothing grows the heap before we free below. */
        var view = Module.HEAPU8.subarray(ptr, ptr + len * 2);
        var result = new TextDecoder('utf-16le').decode(view);
        Module._free(ptr);
        return result;
    }

    Module['Txt2gam'] = /** @class */ (function () {
        function Txt2gam() {
            if (!Module._t2gInit()) throw new Error('TXT2GAM: initialisation failed');
            /* Reusable 4-byte cell for the out-length of every call. */
            this.lenPtr = Module._malloc(4);
        }

        /** Free library resources. */
        Txt2gam.prototype.destroy = function () {
            Module._t2gTerminate();
            Module._free(this.lenPtr);
            this.lenPtr = 0;
        };

        /**
         * Parse raw text data (with optional BOM) to a JS string.
         *
         * @param {Uint8Array} data
         * @param {boolean} isUnicode  Encoding hint when no BOM is present.
         * @returns {string|null}
         */
        Txt2gam.prototype.parseText = function (data, isUnicode) {
            var dataPtr   = allocBytes(data);
            var resultPtr = Module._t2gWasmParseTextData(
                dataPtr, data.length, isUnicode ? 1 : 0, this.lenPtr
            );
            Module._free(dataPtr);
            return consumeStr(resultPtr, this.lenPtr);
        };

        /**
         * Convert a text source to QSP binary game data.
         *
         * @param {string} text
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @param {boolean} isOldFormat
         * @param {boolean} isUnicode      True to encode game in Unicode.
         * @param {string|null} password   Password, or null for "No".
         * @returns {Uint8Array|null}
         */
        Txt2gam.prototype.textToGame = function (text, locStart, locEnd, isOldFormat, isUnicode, password) {
            var textPtr     = allocStr(text);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);
            var passwdPtr   = allocStr(password);

            var resultPtr = Module._t2gWasmEncodeTextToGame(
                textPtr, locStartPtr, locEndPtr,
                isOldFormat ? 1 : 0, isUnicode ? 1 : 0, passwdPtr, this.lenPtr
            );

            Module._free(textPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);
            Module._free(passwdPtr);

            return consumeBytes(resultPtr, this.lenPtr);
        };

        /**
         * Convert QSP binary game data to a text string.
         *
         * @param {Uint8Array} gameBytes
         * @param {string|null} password   Password, or null for "No".
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @returns {string|null}
         */
        Txt2gam.prototype.gameToText = function (gameBytes, password, locStart, locEnd) {
            var dataPtr     = allocBytes(gameBytes);
            var passwdPtr   = allocStr(password);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);

            var resultPtr = Module._t2gWasmDecodeGameToText(
                dataPtr, gameBytes.length,
                passwdPtr, locStartPtr, locEndPtr, this.lenPtr
            );

            Module._free(dataPtr);
            Module._free(passwdPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);

            return consumeStr(resultPtr, this.lenPtr);
        };

        /**
         * Extract in-game strings (or q-strings) from a text source.
         *
         * @param {string} text
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @param {boolean} toGetQStrings  True to extract q-strings, false for strings.
         * @returns {string|null}          Extracted strings, or null on error.
         */
        Txt2gam.prototype.extractStrings = function (text, locStart, locEnd, toGetQStrings) {
            var textPtr     = allocStr(text);
            var locStartPtr = allocStr(locStart);
            var locEndPtr   = allocStr(locEnd);

            var resultPtr = Module._t2gWasmExtractStrings(
                textPtr, locStartPtr, locEndPtr, toGetQStrings ? 1 : 0, this.lenPtr
            );

            Module._free(textPtr);
            Module._free(locStartPtr);
            Module._free(locEndPtr);

            return consumeStr(resultPtr, this.lenPtr);
        };

        return Txt2gam;
    }());
}());
