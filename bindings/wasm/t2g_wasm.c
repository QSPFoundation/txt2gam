/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

/*
 * WASM binding stub.
 *
 * All exported symbols (_t2gInit, _t2gTerminate, _t2gParseTextData,
 * _t2gEncodeTextToGame, _t2gDecodeGameToText, _t2gExtractStrings) come from the txt2gam library
 * and are listed in EXPORTED_FUNCTIONS in CMakeLists.txt. The JavaScript
 * Txt2gam class is injected via --post-js t2g_wasm.js.
 */
