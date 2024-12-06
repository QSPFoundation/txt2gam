/* Copyright (C) 2001-2020 Valeriy Argunov (byte AT qsp DOT org) */
/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "declarations.h"
#include "locations.h"

#ifndef QSP_LOCDATADEFINES
    #define QSP_LOCDATADEFINES

    #define QSP_BASEDESC QSP_FMT("! BASE DESC")
    #define QSP_BASEACTS QSP_FMT("! BASE ACTIONS")
    #define QSP_LOCCODE QSP_FMT("! LOC CODE")
    #define QSP_BASEDESC_PRINT QSP_FMT("*P")
    #define QSP_BASEDESC_PRINTLINE QSP_FMT("*PL")
    #define QSP_BASEACTS_ACT_HEADER QSP_FMT("ACT")
    #define QSP_BASEACTS_ACT_FOOTER QSP_FMT("END")
    #define QSP_BASEACTS_LINE_PREFIX QSP_FMT("\t")

    /* External functions */
    QSP_BOOL qspInitLocProcessor();
    void qspTerminateLocProcessor();
    QSP_BOOL qspParseBaseDescriptionPrint(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspParseBaseDescriptionPrintLine(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspParseBaseAction(QSP_CHAR **code, QSPLocation *loc);
    QSP_BOOL qspUpdateActionCode(QSP_CHAR *code, QSPLocation *loc);
    QSP_BOOL qspUpdateLocationCode(QSP_CHAR *code, QSPLocation *loc);

#endif
