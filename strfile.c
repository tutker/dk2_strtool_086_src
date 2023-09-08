/******************************************************************************/
/** @file strfile.c
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

#include "strfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lbfileio.h"
#include "unitext.h"
#include "strmaker.h"

/**
 * Clears the STR_File structure, dropping any old pointers.
 */
short str_clear(struct STR_File *strfile)
{
  strfile->str=NULL;
  strfile->str_count=0;
  strfile->alloc_count=0;
  strfile->file_id=0;
  return ERR_NONE;
}

/**
 * Sets the amount of strings allocated in STR_File structure.
 * Reallocates the structure, keeping the existing elements.
 */
short str_set_alloc(struct STR_File *strfile,unsigned int count)
{
  unsigned int prev_count=strfile->alloc_count;
  strfile->alloc_count=count;
  strfile->str=realloc(strfile->str,strfile->alloc_count*sizeof(unsigned short *));
  if ((strfile->alloc_count!=0)&&(strfile->str==NULL))
      return -1;
  if (prev_count<strfile->alloc_count)
  {
    int i;
    for (i=strfile->alloc_count-prev_count;i<strfile->alloc_count;i++)
        strfile->str[i]=NULL;
  }
  return ERR_NONE;
}

/**
 * Loads STR file from given filename and creates structure for maintaining it.
 */
struct STR_File *str_open(char *fname,short flags)
{
  struct STR_File *strfile;
  struct STR_Maker *mkstr;
  FILE *fp;
  strfile=malloc(sizeof(struct STR_File));
  mkstr=malloc(sizeof(struct STR_Maker));
  if ((strfile==NULL)||(mkstr==NULL))
  {
    if (flags&STRFLAG_VERBOSE)
      str_error("Cannot allocate STR_File memory");
    free(strfile);
    strmaker_free(mkstr);
    return NULL;
  }
  // Read source file
  strmaker_clear(mkstr);
  fp=fopen(fname,"rb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),fname);
    free(strfile);
    strmaker_free(mkstr);
    return NULL;
  }
  short result;
  result=strmaker_fread(mkstr,fp,flags);
  fclose(fp);
  if (result != ERR_NONE)
  {
    free(strfile);
    strmaker_free(mkstr);
    return NULL;
  }
  mkstr->iosize=0;
  str_clear(strfile);
  //Read codepage converter
  int path_len=filename_from_path(fname)-fname;
  if (path_len<0) path_len=0;
  char *mbfname=malloc(path_len+16);
  if (path_len>0)
    strncpy(mbfname,fname,path_len);
  strcpy(mbfname+path_len,"MBToUni.dat");
  fp=fopen(mbfname,"rb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),mbfname);
    free(strfile);
    strmaker_free(mkstr);
    return NULL;
  }
  result=str_mb2uni_fread(mkstr,fp,flags);
  fclose(fp);
  if (result != ERR_NONE)
  {
    free(mbfname);
    free(strfile);
    strmaker_free(mkstr);
    return NULL;
  }
  free(mbfname);
  // Do the conversion

  // new version - unfinished yet
  strfile->file_id=mkstr->file_id;
  str_set_alloc(strfile,mkstr->offs_count);
  int i;
  for (i=0;i<mkstr->offs_count;i++)
  {
      if (flags&STRFLAG_DEBUG)
          printf("Reading entry %d\n",i);
      unsigned short *udata;
      result=strmaker_get_unicode_entry(mkstr,&udata,i,flags);
      if (udata!=NULL)
      {
          strfile->str[strfile->str_count]=udata;
          strfile->str_count++;
      }
      if (result<ERR_NONE)
      {
          free(strfile);
          strmaker_free(mkstr);
          return NULL;
      }
  }
  if (flags&STRFLAG_DEBUG)
      printf("Total entries decoded: %d\n",strfile->str_count);
  strmaker_free(mkstr);
  return strfile;
}

short str_write(struct STR_File *strfile,char *fname,short flags)
{
  if (strfile==NULL)
  {
      str_error("Internal error - NULL pointer");
      return -1;
  }
  // Allocating STR_Maker structure
  struct STR_Maker *mkstr;
  mkstr=malloc(sizeof(struct STR_Maker));
  if (mkstr==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_error("Cannot allocate STR_Maker memory");
    return -1;
  }
  short result;
  result=strmaker_clear(mkstr);
  // Filling STR_Maker structure
  mkstr->file_id=strfile->file_id;
  // Setting starting size of buffers will make the program work faster
  result=strmaker_set_offsalloc(mkstr,strfile->str_count+4);
  if (result==ERR_NONE)
    result=strmaker_set_dataalloc(mkstr,4096);
  int i;
  if (result!=ERR_NONE)
  {
    if (flags&STRFLAG_VERBOSE)
      str_error("Cannot allocate STR_Maker buffers");
    strmaker_free(mkstr);
    return -1;
  }
  FILE *fp;
  //Read codepage converter
  int path_len=filename_from_path(fname)-fname;
  if (path_len<0) path_len=0;
  char *mbfname=malloc(path_len+16);
  if (path_len>0)
    strncpy(mbfname,fname,path_len);
  // Loading MBToUni instead of UniToMB, as I have no idea how to use MBToUni.
  strcpy(mbfname+path_len,"MBToUni.dat");
  fp=fopen(mbfname,"rb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),mbfname);
    free(mbfname);
    strmaker_free(mkstr);
    return -1;
  }
  result=str_mb2uni_fread(mkstr,fp,flags);
  /*
  // Loading UniToMB; using it requires modification of the encoding function
  // str_data_encode() in strmaker.c
  strcpy(mbfname+path_len,"UniToMB.dat");
  fp=fopen(mbfname,"rb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),mbfname);
    free(mbfname);
    strmaker_free(mkstr);
    return -1;
  }
  result=str_uni2mb_fread(mkstr,fp,flags);
  */
  fclose(fp);
  if (result != ERR_NONE)
  {
    free(mbfname);
    strmaker_free(mkstr);
    return -1;
  }
  int max_idx=((int)strfile->str_count)-1;
  for (i=0;i<max_idx;i++)
  {
      if (flags&STRFLAG_DEBUG)
          printf("Adding entry %d\n",i);
      result=strmaker_add_unicode_entry(mkstr,strfile->str[i],flags);
      if (result!=ERR_NONE)
      {
          strmaker_free(mkstr);
          return -1;
      }
  }
  // Do the last entry separately, because if it's empty it should be skipped
  if (max_idx>=0)
    if (strfile->str[max_idx][0]!=0)
    {
      i=max_idx;
      if (flags&STRFLAG_DEBUG)
          printf("Adding last entry %d\n",i);
      result=strmaker_add_unicode_entry(mkstr,strfile->str[i],flags);
      if (result!=ERR_NONE)
      {
          strmaker_free(mkstr);
          return -1;
      }
    }
  if (flags&STRFLAG_DEBUG)
      printf("Total entries encoded: %d\n",strfile->str_count);
  // Open destination file
  fp=fopen(fname,"wb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),fname);
    strmaker_free(mkstr);
    return -1;
  }
  result=strmaker_fwrite(mkstr,fp,flags);
  fclose(fp);
  return result;
}

/*
 * Fils STR_File structure using Unicode data from TXT_File.
 * @param strfile Destination STR_File struct pointer.
 * @param txtfile Source TXT_File structure pointer.
 * @param flags Flags used to manage the behaviour of the function.
 */
short str_from_txtuni(struct STR_File *strfile,struct TXT_File *txtfile,short flags)
{
  unsigned int file_id=0;
  unsigned int k;
  k=0;
  if (k<txtfile->offs_count)
  {
      // Getting start and end of the line
      long offs;
      offs=txtfile->offsets[k];
      if (offs<0)
      {
          if (flags&STRFLAG_VERBOSE)
              str_ferror("Can't read first line of text file");
          return -1;
      }
      long end_offs=txtfile->data_len;
      if (((k+1)<txtfile->offs_count))
      {
        long tmp_end;
        tmp_end=txtfile->offsets[k+1];
        if ((tmp_end>offs)&&(tmp_end<end_offs))
            end_offs=tmp_end;
      }
      // Reading the line
      long i;
      for (i=offs;i<end_offs;i++)
      {
          unsigned short chr;
          chr=txtfile->data[i];
          if ((chr=='\n')||(chr=='\r')) break;
          if ((chr==',')||(chr=='.')||(chr==' ')||(chr=='\t')) continue;
          if ((chr<'0')||(chr>'9'))
          {
              if (flags&STRFLAG_VERBOSE)
                  str_ferror("Non-digit character in first line of text file");
              return -1;
          }
          file_id=(file_id*10)+(chr-'0');
      }
      k++;
  } else
  {
      if (flags&STRFLAG_VERBOSE)
          str_ferror("No lines in text file");
      return -1;
  }
  if (flags&STRFLAG_DEBUG)
      printf("got file_id=%d\n",file_id);
  strfile->file_id=file_id;
  str_set_alloc(strfile,txtfile->offs_count);
  while (k<txtfile->offs_count)
  {
      // Getting start and end of the line
      long offs;
      offs=txtfile->offsets[k];
      if (offs<0)
      {
          if (flags&STRFLAG_VERBOSE)
              str_ferror("Can't read line %d of text file",k);
          return -1;
      }
      long end_offs=txtfile->data_len;
      if (((k+1)<txtfile->offs_count))
      {
        long tmp_end;
        tmp_end=txtfile->offsets[k+1];
        if ((tmp_end>offs)&&(tmp_end<end_offs))
            end_offs=tmp_end;
      }
      long data_len;
      data_len=end_offs-offs;
      // Allocating memory for unicode string
      strfile->str[strfile->str_count]=malloc((data_len+1)*sizeof(unsigned short));
      if (strfile->str[strfile->str_count]==NULL)
      {
          if (flags&STRFLAG_VERBOSE)
            str_ferror("Can't malloc unicode string for entry %d",k);
          return -1;
      }
      memset(strfile->str[strfile->str_count],0,(data_len+1)*sizeof(unsigned short));
      // Copying string data without control characters
      long i;
      for (i=0;i<data_len;i++)
      {
          unsigned short chr;
          chr=txtfile->data[offs+i];
          if ((chr=='\n')||(chr=='\r')) break;
          if (chr=='\\')
          {
            i++;
            if (i>=data_len) break;
            chr=txtfile->data[offs+i];
            switch (chr)
            {
            case 'r':
                chr='\r';
                break;
            case 'n':
                chr='\n';
                break;
            case 't':
                chr='\t';
                break;
            case '\\':
            default:
                break;
            }
          }
          strfile->str[strfile->str_count][i]=chr;
      }
      strfile->str_count++;
      k++;
  }
  return ERR_NONE;
}

/*
 * Creates STR_File structure from Unicode Text file.
 * @param fname Destination file name.
 * @param flags Flags used to manage the behaviour of the function.
 * @return Returns he STR_File struct pointer, or NULL.
 */
struct STR_File *str_open_unicode(char *fname,short flags)
{
  struct STR_File *strfile;
  struct TXT_File *txtfile;
  FILE *fp;
  strfile=malloc(sizeof(struct STR_File));
  txtfile=malloc(sizeof(struct TXT_File));
  if ((strfile==NULL)||(txtfile==NULL))
  {
    if (flags&STRFLAG_VERBOSE)
      str_error("Cannot allocate memory for structures");
    free(strfile);
    free(txtfile);
    return NULL;
  }
  txtuni_clear(txtfile);
  str_clear(strfile);
  // Read source file
  fp=fopen(fname,"rb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),fname);
    txtuni_free(txtfile);
    free(strfile);
    return NULL;
  }
  short result;
  result=txtuni_read(txtfile,fp,flags);
  fclose(fp);
  if (result != ERR_NONE)
  {
    txtuni_free(txtfile);
    free(strfile);
    return NULL;
  }
  result=str_from_txtuni(strfile,txtfile,flags);
  txtuni_free(txtfile);
  if (result != ERR_NONE)
  {
    free(strfile);
    return NULL;
  }
  return strfile;
}

/**
 * Writes the unicode text file from given STR_File.
 * As for now, it writes the file directly, without use of TXT_File.
 * @param strfile The STR_File struct pointer.
 * @param fname Destination file name.
 * @param flags Flags used to manage the behaviour of the function.
 */
short str_write_unicode(struct STR_File *strfile,char *fname,short flags)
{
  if (strfile==NULL) return -1;
  // Open destination file
  FILE *fp;
  fp=fopen(fname,"wb");
  if (fp==NULL)
  {
    if (flags&STRFLAG_VERBOSE)
      str_ferror("%s when opening %s",strerror(errno),fname);
    return -1;
  }
  fwrite("\xff\xfe",1,2,fp); // This seems to be an Unicode identifier in Windows
  int i,k;
  char buf[16];
  sprintf(buf,"%d\r\n",strfile->file_id);
  i=0;
  while (buf[i]!=0)
  {
      fputc(buf[i],fp);
      fputc(0,fp);
      i++;
  }
  for (k=0;k<strfile->str_count;k++)
  {
    i=0;
    if (strfile->str[k]!=NULL)
      while (strfile->str[k][i]!=0)
      {
          unsigned short chr=strfile->str[k][i];
          // Support some special characters
          switch (chr)
          {
          case (unsigned char)'\n':
              fwrite("\\\0\n\0",1,4,fp);
              break;
          case (unsigned char)'\r':
              fwrite("\\\0\r\0",1,4,fp);
              break;
          case (unsigned char)'\t':
              fwrite("\\\0\t\0",1,4,fp);
              break;
          case (unsigned char)'\\': // Write "\" as "\\".
              fwrite("\\\0\\\0",1,4,fp);
              break;
          default:
              fwrite(&(chr),1,2,fp);
              break;
          }
          i++;
      }
      fwrite("\r\0\n\0",1,4,fp);
  }
  fclose(fp);
  return ERR_NONE;
}

/**
 * Frees the given structure for maintaining STR file.
 * @param strfile The STR_File struct pointer.
 * @param flags Flags used to manage the behaviour of the function.
 */
short str_close(struct STR_File *strfile,short flags)
{
  if (strfile==NULL) return -1;
  if (strfile->str!=NULL)
  {
    int i;
    for (i=0;i<strfile->str_count;i++)
    {
      if (strfile->str[i]!=NULL)
      {
        free(strfile->str[i]);
      }
    }
    free(strfile->str);
  }
  free(strfile);
  return ERR_NONE;
}

