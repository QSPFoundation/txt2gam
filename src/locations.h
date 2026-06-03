/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "declarations.h"
#include "t2g_api.h"

#ifndef QSP_LOCSDEFINES
    #define QSP_LOCSDEFINES

    #define QSP_GAMEID QSP_FMT("QSPGAME")
    #define QSP_PASSWD T2G_PASSWD
    #define QSP_VERINFOSIZE 100
    #define QSP_DATESTRSIZE 32
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
    QSP_CHAR *qspGetLocsStrings(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd, QSP_BOOL toGetQStrings);
    int qspOpenTextData(QSP_CHAR *data, QSP_CHAR *locStart, QSP_CHAR *locEnd);
    char *qspSaveQuest(QSP_BOOL isOldFormat, QSP_BOOL isUnicode, QSP_CHAR *passwd, int *dataLen);
    QSP_BOOL qspOpenQuest(char *data, int dataSize, QSP_CHAR *password);
    QSP_CHAR *qspSaveQuestAsText(QSP_CHAR *locStart, QSP_CHAR *locEnd);

#endif
