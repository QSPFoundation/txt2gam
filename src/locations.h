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

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    #define QSP_GAMEID QSP_FMT("QSPGAME")
    #define QSP_PASSWD QSP_FMT("No")
    #define QSP_MAXACTIONS 50

    typedef struct
    {
        QSP_CHAR *Image;
        QSP_CHAR *Desc;
        QSP_CHAR *Code;
    } QSPLocAct;
    typedef struct
    {
        QSP_CHAR *Name;
        QSP_CHAR *Desc;
        QSP_CHAR *OnVisit;
        QSPLocAct Actions[QSP_MAXACTIONS];
        int ActionsCount; /* max number of actions */
    } QSPLocation;

    extern QSPLocation *qspLocs;
    extern int qspLocsCount;

    /* External functions */
    void qspCreateWorld(int locsCount);
    QSP_CHAR *qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isGetQStrings);
    int qspOpenTextData(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL isFill);
    char *qspSaveQuest(QSP_BOOL isOldFormat, QSP_BOOL isUCS2, QSP_CHAR *passwd, int *dataLen);
    QSP_BOOL qspOpenQuest(char *data, int dataSize, QSP_CHAR *password);
    QSP_CHAR *qspSaveQuestAsText(QSP_CHAR *locStart, QSP_CHAR *locEnd);

#endif
