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
 *   try {
 *       const gameBytes = t2g.textToGame(text, null, null, false, false, null);
 *       const text = t2g.gameToText(gameUint8Array, null, null, null);
 *   } catch (e) {
 *       if (e instanceof module.T2gError && e.code === module.T2G_ERROR_WRONG_PASSWORD) { ... }
 *   } finally {
 *       t2g.destroy();
 *   }
 */

(function () {
    /* Error codes (mirror of the T2G_ERROR_* enum in t2g_types.h). */
    Module['T2G_ERROR_NONE']           = 0;
    Module['T2G_ERROR_FAILED']         = 1;
    Module['T2G_ERROR_INVALID_DATA']   = 2;
    Module['T2G_ERROR_WRONG_PASSWORD'] = 3;
    Module['T2G_ERROR_NO_MEMORY']      = 10;

    /* Human-readable message for each error code. Add new codes here. */
    var ERROR_MESSAGES = {
        [Module['T2G_ERROR_FAILED']]:         'Operation failed',
        [Module['T2G_ERROR_INVALID_DATA']]:   'Invalid or corrupt data',
        [Module['T2G_ERROR_WRONG_PASSWORD']]: 'Wrong password',
        [Module['T2G_ERROR_NO_MEMORY']]:      'Out of memory'
    };

    /**
     * Error thrown by Txt2gam methods on failure.
     * @property {number} code  One of the module.T2G_ERROR_* constants.
     */
    function T2gError(code) {
        this.name = 'T2gError';
        this.code = code;
        this.message = ERROR_MESSAGES[code] || ('Unknown error (code ' + code + ')');
        if (Error.captureStackTrace) Error.captureStackTrace(this, T2gError);
    }
    T2gError.prototype = Object.create(Error.prototype);
    T2gError.prototype.constructor = T2gError;
    Module['T2gError'] = T2gError;

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

    /*
     * Tracks every pointer allocated for a single call so they can all be
     * released at once, regardless of how many arguments a call has.
     */
    function Args() {
        this._ptrs = [];
    }
    Args.prototype.str = function (str) {
        var ptr = allocStr(str);
        this._ptrs.push(ptr);
        return ptr;
    };
    Args.prototype.bytes = function (bytes) {
        var ptr = allocBytes(bytes);
        this._ptrs.push(ptr);
        return ptr;
    };
    Args.prototype.free = function () {
        for (var i = 0; i < this._ptrs.length; i++) Module._free(this._ptrs[i]);
        this._ptrs.length = 0;
    };

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

        function checkError() {
            var code = Module._t2gWasmGetLastError();
            if (code !== Module['T2G_ERROR_NONE']) throw new T2gError(code);
        }

        /**
         * Parse raw text data (with optional BOM) to a JS string.
         *
         * @param {Uint8Array} data
         * @param {boolean} isUnicode  Encoding hint when no BOM is present.
         * @returns {string}
         * @throws {T2gError}
         */
        Txt2gam.prototype.parseText = function (data, isUnicode) {
            var args = new Args();
            var resultPtr = Module._t2gWasmParseTextData(
                args.bytes(data), data.length, isUnicode ? 1 : 0, this.lenPtr
            );
            args.free();
            checkError();
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
         * @returns {Uint8Array}
         * @throws {T2gError}
         */
        Txt2gam.prototype.textToGame = function (text, locStart, locEnd, isOldFormat, isUnicode, password) {
            var args = new Args();
            var resultPtr = Module._t2gWasmEncodeTextToGame(
                args.str(text), args.str(locStart), args.str(locEnd),
                isOldFormat ? 1 : 0, isUnicode ? 1 : 0, args.str(password), this.lenPtr
            );
            args.free();
            checkError();
            return consumeBytes(resultPtr, this.lenPtr);
        };

        /**
         * Convert QSP binary game data to a text string.
         *
         * @param {Uint8Array} gameBytes
         * @param {string|null} password   Password, or null for "No".
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @returns {string}
         * @throws {T2gError}
         */
        Txt2gam.prototype.gameToText = function (gameBytes, password, locStart, locEnd) {
            var args = new Args();
            var resultPtr = Module._t2gWasmDecodeGameToText(
                args.bytes(gameBytes), gameBytes.length,
                args.str(password), args.str(locStart), args.str(locEnd), this.lenPtr
            );
            args.free();
            checkError();
            return consumeStr(resultPtr, this.lenPtr);
        };

        /**
         * Extract in-game strings (or q-strings) from a text source.
         *
         * @param {string} text
         * @param {string|null} locStart   Location-start marker, or null for "#".
         * @param {string|null} locEnd     Location-end marker, or null for "--".
         * @param {boolean} toGetQStrings  True to extract q-strings, false for strings.
         * @returns {string}
         * @throws {T2gError}
         */
        Txt2gam.prototype.extractStrings = function (text, locStart, locEnd, toGetQStrings) {
            var args = new Args();
            var resultPtr = Module._t2gWasmExtractStrings(
                args.str(text), args.str(locStart), args.str(locEnd), toGetQStrings ? 1 : 0, this.lenPtr
            );
            args.free();
            checkError();
            return consumeStr(resultPtr, this.lenPtr);
        };

        return Txt2gam;
    }());
}());
