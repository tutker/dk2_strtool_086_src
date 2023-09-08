/******************************************************************************/
/** @file unitext.c
 * Library for r/w of DK2 STR text strings files.
 * @par Purpose:
 *     Allows reading/writing of DK2 STR (text strings) files.
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

#include "unitext.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lbfileio.h"

short str_wtos(char *dst,const short *src)
{
    unsigned int i=0;
    short chr=1;
    while (chr!=0)
    {
        chr=src[i];
        if (chr<256)
          dst[i]=chr;
        else
          dst[i]='_';
        i++;
    }
    return ERR_NONE;
}

int unicode_strlen(unsigned short *buf)
{
    int i=0;
    if (buf!=NULL)
      while (buf[i]!=0)
       i++;
    return i;
}

unsigned int unicode_buf_lines_count(unsigned short *buf,long buflen)
{
  unsigned int lncount=1;
  long i;
  i=0;
  while (i<buflen)
  {
      switch (buf[i])
      {
      case '\n':
          i++;
          lncount++;
          // skip "\r" if the line ends with "\n\r"
          if (buf[i]=='\r')
            i++;
          break;
      case '\r':
          i++;
          lncount++;
          // skip "\n" if the line ends with "\r\n"
          if (buf[i]=='\n')
            i++;
          break;
      default:
          i++;
          break;
      }
  }
  return lncount;
}

/**
 * Returns offset of the new line after offs, or -1 if not found.
 */
long unicode_buf_newln_offs(unsigned short *buf,long offs,long buflen)
{
  long i;
  i=offs;
  while (i<buflen)
  {
      switch (buf[i])
      {
      case '\n':
          i++;
          // skip "\r" if the line ends with "\n\r"
          if (buf[i]=='\r')
            i++;
          return i;
      case '\r':
          i++;
          // skip "\n" if the line ends with "\r\n"
          if (buf[i]=='\n')
            i++;
          return i;
      default:
          i++;
          break;
      }
  }
  return -1;
}

short txtuni_set_offsalloc(struct TXT_File *txtfile,unsigned int count)
{
  unsigned int prev_count=txtfile->offs_alloc;
  txtfile->offs_alloc=count;
  txtfile->offsets=realloc(txtfile->offsets,txtfile->offs_alloc*sizeof(long));
  if ((txtfile->offs_alloc!=0)&&(txtfile->offsets==NULL))
      return -1;
  if (prev_count<txtfile->offs_alloc)
  {
    int i;
    for (i=txtfile->offs_alloc-prev_count;i<txtfile->offs_alloc;i++)
        txtfile->offsets[i]=-1;
  }
  return ERR_NONE;
}

short txtuni_set_dataalloc(struct TXT_File *txtfile,unsigned long len)
{
  unsigned int prev_len=txtfile->data_alloc;
  txtfile->data_alloc=len;
  txtfile->data=realloc(txtfile->data,txtfile->data_alloc*sizeof(unsigned short));
  if ((txtfile->data_alloc!=0)&&(txtfile->data==NULL))
      return -1;
  if (prev_len<txtfile->data_alloc)
  {
    int i;
    for (i=txtfile->data_alloc-prev_len;i<txtfile->data_alloc;i++)
        txtfile->data[i]=0;
  }
  return ERR_NONE;
}

short txtuni_clear(struct TXT_File *txtfile)
{
  txtfile->offsets=NULL;
  txtfile->offs_alloc=0;
  txtfile->data=NULL;
  txtfile->data_alloc=0;
  txtfile->offs_count=0;
  txtfile->data_len=0;
  return ERR_NONE;
}

short txtuni_free(struct TXT_File *txtfile)
{
  free(txtfile->offsets);
  free(txtfile->data);
  free(txtfile);
  return ERR_NONE;
}

/**
 * Reads Unicode text file.
 */
short txtuni_read(struct TXT_File *txtfile,FILE *fp,short flags)
{
  if (txtfile==NULL)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Internal error");
      return -1;
  }
  long file_len=file_length_opened(fp);
  if (file_len<0)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Can't get text file size");
      return -1;
  }
  short result;
  // Reading file data
  result=txtuni_set_dataalloc(txtfile,(file_len>>1)+2);
  if (result!=ERR_NONE)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Cannot allocate memory for text file");
      return result;
  }
  txtfile->data_len = (file_len>>1);
  int nread;
  if (flags&STRFLAG_DEBUG)
      printf("reading %d entries from FILE at %08x into %08x\n",txtfile->data_len,fp,txtfile->data);
  nread=fread(txtfile->data,1,txtfile->data_len*sizeof(unsigned short),fp);
  if (nread!=txtfile->data_len*sizeof(unsigned short))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("%s when reading text file",strerror(errno));
      return -1;
  }
  // Counting text lines
  unsigned int lncount=unicode_buf_lines_count(txtfile->data,txtfile->data_len);
  txtuni_set_offsalloc(txtfile,lncount+2);
  unsigned int i;
  long offs;
  txtfile->offs_count=0;
  i=0;
  {
      offs=0;
      if (txtfile->data[i] == 0xfeff) offs++;
      txtfile->offsets[txtfile->offs_count]=offs;
      txtfile->offs_count++;
//      printf("text line %d at offset %d\n",i,offs);
  }
  for (i=1;i<=lncount;i++)
  {
      offs=unicode_buf_newln_offs(txtfile->data,offs,txtfile->data_len);
      txtfile->offsets[txtfile->offs_count]=offs;
      if (offs>=0)
        txtfile->offs_count++;
//      printf("text line %d at offset %d\n",i,offs);
  }
  return ERR_NONE;
}
