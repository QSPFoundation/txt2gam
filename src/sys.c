/* Copyright (C) 2001-2026 Val Argunov (byte AT qsp DOT org) */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "text.h"
#include "coding.h"
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

void qspPrint(const char *format, ...)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    va_list arg_list;
    va_start(arg_list, format);
    while (*format)
    {
        if (*format == '%')
        {
            ++format;
            switch (*format)
            {
            case 'd':
            case 'i':
                {
                    int val = va_arg(arg_list, int);
                    printf("%d", val);
                    break;
                }
            case 'u':
                {
                    unsigned int val = va_arg(arg_list, unsigned int);
                    printf("%u", val);
                    break;
                }
            case 's':
                {
                    QSP_CHAR *val = va_arg(arg_list, QSP_CHAR *);
                    char *utf8 = qspQSPStringToUTF8(val);
                    printf("%s", utf8);
                    free(utf8);
                    break;
                }
            default:
                {
                    printf("%%%c", *format);
                    break;
                }
            }
        }
        else
            printf("%c", *format);
        ++format;
    }
    va_end(arg_list);
}
