/******************************************************************************/
/** @file strfile.h
 * Library for r/w of DK2 STR text strings files.
 * @par Purpose:
 *     Header file. Defines exported routines from strfile.c.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     29 Jul 2008 - 15 Aug 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef STRFILE_H
#define STRFILE_H

#include <stdio.h>
#include "strtool_private.h"

struct STR_File {
    unsigned int file_id;
    unsigned int alloc_count;// Allocated entries
    unsigned int str_count;  // Used entries
    unsigned short **str;    // String are stored in unicode
    };

// Routines

struct STR_File *str_open(char *fname,short flags);
struct STR_File *str_open_unicode(char *fname,short flags);
short str_close(struct STR_File *strfile,short flags);


#endif
