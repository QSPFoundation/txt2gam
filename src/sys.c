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
#ifdef _UNICODE
                    QSP_CHAR *val = va_arg(arg_list, QSP_CHAR *);
                    char *utf8 = qspQSPStringToUTF8(val);
                    printf("%s", utf8);
                    free(utf8);
#else
                    char *val = va_arg(arg_list, char *);
                    printf("%s", val);
#endif
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
