/******************************************************************************/
/** @file strmaker.h
 * Library for r/w of DK2 STR text strings files.
 * @par Purpose:
 *     Header file. Defines exported routines from strmaker.c.
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

#ifndef STRMAKER_H
#define STRMAKER_H

#include <stdio.h>

enum DK2STR_ChunkType {
        CTSTR_END                = 0x00,
        CTSTR_PARAM              = 0x02,
        CTSTR_STRING             = 0x01,
    };

struct STR_Maker {
    char magic[4];
    unsigned int file_id;    // File ID is written in header of every STR
    unsigned int offs_alloc; // Allocated offset entries
    unsigned int offs_count; // Used entries
    long *offsets;           // Offsets of entries in data
    unsigned long data_alloc;// Allocated data size
    unsigned long data_len;  // Size of used data
    unsigned char *data;     // File data
    unsigned int mb2uni_count;
    unsigned short *mb2uni;
    unsigned int uni2mb_count;
    unsigned short *uni2mb;
    unsigned long iosize;
    long disksize;
    };

#define SIZEOF_STR_Header 12
#define SIZEOF_STR_ChunkHeader 4

// Routines

short str_error(const char *msg);
short str_ferror(const char *format, ...);

char *filename_from_path(const char *pathname);

short str_data_encode(unsigned char **edata,long *edata_len,
    const unsigned short *uni2mb,const long uni2mb_count,
    const unsigned short *udata,const long udata_len);
short str_data_encode_r(unsigned char **edata,long *edata_len,
    const unsigned short *mb2uni,const long mb2uni_count,
    const unsigned short *udata,const long udata_len);
short str_data_decode(unsigned short **udata,long *udata_len,
    const unsigned short *mb2uni,const long mb2uni_count,
    const unsigned char *edata,const long edata_len);

short strmaker_clear(struct STR_Maker *mkstr);
short strmaker_free(struct STR_Maker *mkstr);
short strmaker_add_entry(struct STR_Maker *mkstr,unsigned char *edata,unsigned long len);
short strmaker_add_unicode_entry(struct STR_Maker *mkstr,unsigned short *udata,short flags);
short strmaker_get_unicode_entry(const struct STR_Maker *mkstr,
    unsigned short **udata,int index,short flags);

short str_mb2uni_fread(struct STR_Maker *mkstr,FILE *fp,short flags);
short strmaker_fread(struct STR_Maker *mkstr,FILE *fp,short flags);
short strmaker_fwrite(struct STR_Maker *mkstr,FILE *fp,short flags);
int strmaker_get_entry(const struct STR_Maker *mkstr,char **edata,unsigned int entryidx,short flags);
short convert_mb2uni(struct STR_Maker *mkstr,char *edata,unsigned short *str,unsigned long data_len);


#endif
