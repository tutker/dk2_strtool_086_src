/******************************************************************************/
/** @file unitext.h
 * Library for r/w of DK2 STR text strings files.
 * @par Purpose:
 *     Header file. Defines exported routines from unitext.c.
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

#ifndef UNITEXT_H
#define UNITEXT_H

#include <stdio.h>

#define STRFLAG_VERBOSE         0x01
#define STRFLAG_DEBUG           0x02

#define ERR_NONE                0x00

struct TXT_File {
    unsigned int offs_alloc; // Allocated offset entries
    unsigned int offs_count; // Used entries
    long *offsets;           // Offsets of line starts in data
    unsigned long data_alloc;// Allocated data size
    long data_len;           // Size of used data
    unsigned short *data;    // File data
    };

// Routines

short txtuni_read(struct TXT_File *txtfile,FILE *fp,short flags);
short txtuni_free(struct TXT_File *txtfile);
short txtuni_clear(struct TXT_File *txtfile);
short txtuni_set_dataalloc(struct TXT_File *txtfile,unsigned long len);
short txtuni_set_offsalloc(struct TXT_File *txtfile,unsigned int count);
long unicode_buf_newln_offs(unsigned short *buf,long offs,long buflen);
unsigned int unicode_buf_lines_count(unsigned short *buf,long buflen);
short str_wtos(char *dst,const short *src);


#endif
