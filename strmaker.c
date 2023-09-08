/******************************************************************************/
/** @file strmaker.c
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

#include "strmaker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lbfileio.h"
#include "unitext.h"

const char str_magic[]="BFST";
const char mb2uni_magic[]="BFMU";
const char uni2mb_magic[]="BFUM";

/*
 * Displays simple error message.
 */
short str_error(const char *msg)
{
    printf("STR Support: %s\n",msg);
    return 1;
}

/*
 * Displays formatted error message.
 */
short str_ferror(const char *format, ...)
{
    static char errmessage[255];
    va_list val;
    va_start(val, format);
    vsprintf(errmessage,format,val);
    va_end(val);
    short result;
    result=str_error(errmessage);
    return result;
}

/**
 * Returns file name pointer from given filename with path.
 * @param pathname The source filename, possibly with path.
 * @return Pointer to the name in pathname string, with path skipped.
 */
char *filename_from_path(const char *pathname)
{
    const char *fname = NULL;
    if (pathname)
    {
        fname = strrchr (pathname, '/') + 1;
        char *fname2 = strrchr (pathname, '\\') + 1;
        if ((!fname)||(fname2>fname))
            fname = fname2;
    }
    if (!fname)
        fname=pathname;
    return (char *)fname;
}

/**
 * Encodes an unicode string into STR file entry. This version
 * tries to use UniToMb, which is fast and proper,
 * but we don't know how to use UniToMb correctly, so it's bugged.
 * @return Returns ERR_NONE on success.
 */
short str_data_encode(unsigned char **edata,long *edata_len,
    const unsigned short *uni2mb,const long uni2mb_count,
    const unsigned short *udata,const long udata_len)
{
  int i;
  int sidx,eidx;
  int blockpos;
  blockpos=0;
  (*edata_len)=blockpos+(SIZEOF_STR_ChunkHeader<<1)+(udata_len>>2);
  (*edata)=malloc((*edata_len)+1);
  if ((*edata)==NULL)
  {
    str_error("Can't allocate memory to start encoding STR entry from Unicode");
    return -1;
  }
  unsigned int chunk_type;
  eidx=0;
  chunk_type=CTSTR_STRING;
  //printf("Starting encoding loop...");
  for (i=0;i<udata_len;i++)
  {
      sidx=udata[i];
      if (sidx=='\\')
      {
        i++;
        switch (udata[i])
        {
        case '\\':
            sidx='\\';
        case 'n':
            sidx='\n';
        case 't':
            sidx='\t';
        default:
            sidx='_';
        }
      }
      unsigned short chr;
/*
      //Try 1 of using UniToMB
      if (sidx>255)
      {
          sidx=uni2mb_count+255-sidx;
          if (sidx>=uni2mb_count)
              chr='_';
          else
              chr=uni2mb[sidx];
      } else
      {
          if (sidx+8>=uni2mb_count)
              chr='_';
          else
              chr=uni2mb[sidx+8];
      }
      */
      //Try 2 of using UniToMB
      int j,k;
      j=sidx+10;  // character index counter
      k=0;  // uni2mb array position
      while (1)
      {
          if (k>=uni2mb_count)
          {
              chr='_';
              break;
          }
          chr=uni2mb[k];
          if ((chr&0xff)!=0xff)
            j-=(1<<(chr>>8));
          else
            j--;
          if (j<=0) break;
          k++;
      }
      //printf(" *%04x",k);
      //printf("%c",sidx);
      unsigned short chrlen;
      if ((chr&0xff)==0xff)
          chrlen=2;
      else
          chrlen=1;
      if ((blockpos+SIZEOF_STR_ChunkHeader+eidx+chrlen)>(*edata_len))
      {
          (*edata_len)=(blockpos+(SIZEOF_STR_ChunkHeader<<1)+eidx+chrlen);
          (*edata)=realloc((*edata),(*edata_len)+1);
          if ((*edata)==NULL)
          {
            str_error("Can't allocate memory when encoding STR entry from Unicode");
            return -1;
          }
      }
      (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=(unsigned char)(chr&0xff);
      //printf(" %02x",(*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]);
      eidx++;
      if (chrlen>1)
      {
          (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=(unsigned char)(chr>>8);
          //printf("-%02x",(chr>>8));
          eidx++;
      }
  }
  if ((blockpos+(SIZEOF_STR_ChunkHeader<<1)+eidx)!=(*edata_len))
  {
      (*edata_len)=(blockpos+(SIZEOF_STR_ChunkHeader<<1)+eidx);
      (*edata)=realloc((*edata),(*edata_len)+1);
      if ((*edata)==NULL)
      {
          str_error("Can't allocate memory to end encoding STR entry from Unicode");
          return -1;
      }
  }
  write_int32_le_buf((*edata)+blockpos,chunk_type+(eidx<<8));
  chunk_type=CTSTR_END;
  write_int32_le_buf((*edata)+blockpos+SIZEOF_STR_ChunkHeader+eidx,chunk_type);
  //printf("Finished\n");
  return ERR_NONE;
}

/**
 * Encodes an unicode string into STR file entry. This special version
 * uses MbToUni conversion array instead of UniToMb, which is slower,
 * but as we don't know how to use UniToMb, that's the only way.
 * @return Returns ERR_NONE on success.
 */
short str_data_encode_r(unsigned char **edata,long *edata_len,
    const unsigned short *mb2uni,const long mb2uni_count,
    const unsigned short *udata,const long udata_len)
{
  int i;
  int sidx,eidx;
  int blockpos;
  blockpos=0;
  (*edata_len)=blockpos+(SIZEOF_STR_ChunkHeader<<1)+udata_len;
  (*edata)=malloc((*edata_len)+1);
  if ((*edata)==NULL)
  {
    str_error("Can't allocate memory to start encoding STR entry from Unicode");
    return -1;
  }
  unsigned int chunk_type;
  eidx=0;
  chunk_type=CTSTR_STRING;
  //printf("Starting encoding loop...");
  for (i=0;i<udata_len;i++)
  {
      sidx=udata[i];
      if (sidx=='\\')
      {
        i++;
        switch (udata[i])
        {
        case '\\':
            sidx='\\';
        case 'n':
            sidx='\n';
        case 't':
            sidx='\t';
        default:
            sidx='_';
        }
      } else
      if (sidx=='%')
      {
        i++;
        if (udata[i]=='%')
        {
            sidx='%';
        } else
        {
            //Closing the previous chunk
            write_int32_le_buf((*edata)+blockpos,chunk_type+(eidx<<8));
            while ((eidx%4)!=0)
            { (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=0; eidx++; }
            blockpos+=eidx+SIZEOF_STR_ChunkHeader;
            eidx=0;
            // Allocatinm memory for new one, and a little more
            if ((blockpos+SIZEOF_STR_ChunkHeader+eidx+(SIZEOF_STR_ChunkHeader<<2))>(*edata_len))
            {
                (*edata_len)=blockpos+SIZEOF_STR_ChunkHeader+eidx+(SIZEOF_STR_ChunkHeader<<2);
                (*edata)=realloc((*edata),(*edata_len)+1);
                if ((*edata)==NULL)
                {
                    str_error("Can't allocate memory for parameter when encoding STR entry from Unicode");
                    return -1;
                }
            }
            // Param chunk has only header
        //TODO: write the code to handle parameters
            chunk_type=CTSTR_PARAM;
            sidx=0;
            while (isdigit(udata[i]))
            {
                sidx=sidx*10 + (udata[i]-'0');
                i++;
            }
            if (sidx>0) sidx--;
            // Closing the param chunk
            // It always have four bytes, so zero-padding isn't neccessary
            write_int32_le_buf((*edata)+blockpos,chunk_type+(sidx<<8));
            blockpos+=eidx+SIZEOF_STR_ChunkHeader;
            eidx=0;
            chunk_type=CTSTR_STRING;
            i--;
            continue;
        }
      }
      unsigned short chr;
      // Using MBToUni instead of UniToMB, as I have no idea how to handle MBToUni.
      int k;
      chr='_';
      for (k=0;k<mb2uni_count;k++)
      {
          if (mb2uni[k]==sidx)
          {
              chr=k;
              break;
          }
      }
      //printf(" *%04x",k);

//      printf("%c",sidx);
      unsigned short chrlen;
      k=chr;
      chrlen=1;
      while (k>=255)
      {
         chrlen++;
         k-=254;
      }
      if ((blockpos+SIZEOF_STR_ChunkHeader+eidx+chrlen)>(*edata_len))
      {
          (*edata_len)=(blockpos+(SIZEOF_STR_ChunkHeader<<1)+eidx+chrlen);
          (*edata)=realloc((*edata),(*edata_len)+1);
          if ((*edata)==NULL)
          {
            str_error("Can't allocate memory when encoding STR entry from Unicode");
            return -1;
          }
      }
      k=chr;
      while (k>=255)
      {
         (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=(unsigned char)(0xff);
         eidx++;
         k-=254;
      }
      (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=(unsigned char)(k);
      eidx++;
  }
  // Closing previous chunk
  write_int32_le_buf((*edata)+blockpos,chunk_type+(eidx<<8));
  // No zero padding at end of whole entry (just don't ask..)
//TODO: suspicious
//  while ((eidx%4)!=0)
//  { (*edata)[blockpos+SIZEOF_STR_ChunkHeader+eidx]=0; eidx++; }
  blockpos+=eidx+SIZEOF_STR_ChunkHeader;
  eidx=0;
  // Preparing the last chunk
  chunk_type=CTSTR_END;
  if ((blockpos+SIZEOF_STR_ChunkHeader+eidx)!=(*edata_len))
  {
      (*edata_len)=(blockpos+SIZEOF_STR_ChunkHeader+eidx);
      (*edata)=realloc((*edata),(*edata_len)+1);
      if ((*edata)==NULL)
      {
          str_error("Can't allocate memory to end encoding STR entry from Unicode");
          return -1;
      }
  }
  write_int32_le_buf((*edata)+blockpos,chunk_type);
  //printf("Finished\n");
  return ERR_NONE;
}

/**
 * Decodes a single CTSTR_STRING chunk data into unicode.
 * The output udata buffer needs to be preallocated.
 * @return Returns size of the output data, or negative error code.
 */
int str_data_strchunk_decode(unsigned short *udata,
    const unsigned short *mb2uni,const long mb2uni_count,
    const unsigned char *edata,const long edata_len)
{
  if (mb2uni==NULL)
  {
    udata[0]=0;
    return -1;
  }
  int udata_len=0;
  int mbidx=0;
  int i;
  for (i=0;i<edata_len;i++)
  {
      unsigned char chr;
      chr=(unsigned char)edata[i];
      if (chr==0xff)
      {
        mbidx+=254;
        continue;
      }
      unsigned short uchr;
      mbidx+=chr;
      if (mbidx<mb2uni_count)
        uchr=mb2uni[mbidx];
      else
        uchr=(unsigned char)'_';
      if (uchr=='%')
      {
          udata[udata_len]='%';
          udata_len++;
      } else
      if ((uchr=='\\')||(uchr=='\n')||(uchr=='\t'))
      {
          udata[udata_len]='\\';
          udata_len++;
          switch (uchr)
          {
          case '\\':
               break;
          case '\n':
               uchr=(unsigned char)'n';
               break;
          case '\t':
               uchr=(unsigned char)'t';
               break;
          }
      }
      udata[udata_len]=uchr;
      mbidx=0;
      udata_len++;
  }
  udata[udata_len]=0;
  return udata_len;
}

/**
 * Decodes STR file entry into Unicode string and returns it.
 * @return Returns ERR_NONE on success.
 */
short str_data_decode(unsigned short **udata,long *udata_len,
    const unsigned short *mb2uni,const long mb2uni_count,
    const unsigned char *edata,const long edata_len)
{
  //printf("Decoding entry...\n");
  int uidx,eidx;
  (*udata_len)=(edata_len<<1);
  (*udata)=malloc(((*udata_len)+1)*sizeof(unsigned short));
  if ((*udata)==NULL)
  {
    str_error("Can't allocate memory to start decoding STR entry to Unicode");
    return -1;
  }
  uidx=0;
  eidx=0;
  unsigned int chunk_type=CTSTR_END;
  unsigned int chunk_len=0;
  do {
    if (eidx+SIZEOF_STR_ChunkHeader>edata_len)
    {
      str_ferror("Data too short for next chunk header");
      str_ferror("Last type %d, size %d, total %d",chunk_type,chunk_len,edata_len);
      (*udata)[uidx]=0;
      return -1;
    }
    chunk_type=read_int32_le_buf(edata+eidx);
    eidx += 4;
    chunk_len = (chunk_type>>8);
    chunk_type &= 0xff;
    //printf("type=%02x, len=%06x\n",chunk_type,chunk_len);
    switch (chunk_type)
    {
    case CTSTR_END:
        if (chunk_len!=0)
        {
            str_ferror("Entry end chunk has nonzero size");
            (*udata)[uidx]=0;
            return -1;
        }
        break;
    case CTSTR_PARAM:
        {
            //printf("param, uidx=%d\n",uidx);
            if ((uidx+8)>(*udata_len))
            {
                //printf("realloc! %d to %d\n",(*udata_len),(uidx+8));
                (*udata_len)=(uidx+8);
                (*udata)=realloc((*udata),((*udata_len)+1)*sizeof(unsigned short));
                if ((*udata)==NULL)
                {
                    str_error("Can't allocate memory when decoding STR chunk to Unicode");
                    return -1;
                }
            }
            (*udata)[uidx]='%';
            uidx++;
            int val;
            val=(chunk_len+1)/10;
            if (val>0)
            {
                (*udata)[uidx]='0'+val;
                uidx++;
            }
            val=(chunk_len+1)%10;
            (*udata)[uidx]='0'+val;
            uidx++;
            (*udata)[uidx]=0;
        }
        break;
    case CTSTR_STRING:
        //printf("string, uidx=%d\n",uidx);
        if (eidx+chunk_len>edata_len)
        {
            str_error("Entry length exceeds file size");
            (*udata)[uidx]=0;
            return -1;
        }
        if (chunk_len>0)
        {
            if ((uidx+chunk_len)>(*udata_len))
            {
                //printf("realloc! %d to %d\n",(*udata_len),(uidx+chunk_len));
                (*udata_len)=(uidx+chunk_len);
                (*udata)=realloc((*udata),((*udata_len)+1)*sizeof(unsigned short));
                if ((*udata)==NULL)
                {
                    str_error("Can't allocate memory when decoding STR chunk to Unicode");
                    return -1;
                }
            }
           // decode
            int uchunklen;
            uchunklen=str_data_strchunk_decode((*udata)+uidx,mb2uni,mb2uni_count,
                edata+eidx,chunk_len);
            if (uchunklen>0)
                uidx+=uchunklen;
            (*udata)[uidx]=0;
            eidx+=chunk_len;
        }
        break;
    default:
        {
            str_ferror("Bad STR chunk type %02x",chunk_type);
            (*udata)[uidx]=0;
            return -1;
        }
    }
    if ((eidx%4)!=0) eidx += 4-(eidx%4);
  } while (chunk_type!=CTSTR_END);
  if ((uidx)!=(*udata_len))
  {
      (*udata_len)=(uidx);
      (*udata)=realloc((*udata),((*udata_len)+1)*sizeof(unsigned short));
      if ((*udata)==NULL)
      {
          str_error("Can't allocate memory to end decoding STR entry to Unicode");
          str_ferror("wanted %d bytes",((*udata_len)+1)*sizeof(unsigned short));
          return -1;
      }
  }
  (*udata)[uidx]=0;
  //printf("Entry decoded, %d output characters filled.\n",uidx);
  return ERR_NONE;
}

/**
 * Sets allocated amount of offset entries in STR_Maker.
 * @return Returns ERR_NONE on success.
 */
short strmaker_set_offsalloc(struct STR_Maker *mkstr,unsigned int count)
{
  unsigned int prev_count=mkstr->offs_alloc;
  mkstr->offs_alloc=count;
  mkstr->offsets=realloc(mkstr->offsets,mkstr->offs_alloc*sizeof(long));
  if ((mkstr->offs_alloc!=0)&&(mkstr->offsets==NULL))
      return -1;
  if (prev_count<mkstr->offs_alloc)
  {
    int i;
    for (i=mkstr->offs_alloc-prev_count;i<mkstr->offs_alloc;i++)
        mkstr->offsets[i]=-1;
  }
  return ERR_NONE;
}

/**
 * Sets allocated size of data in STR_Maker.
 * @return Returns ERR_NONE on success.
 */
short strmaker_set_dataalloc(struct STR_Maker *mkstr,unsigned long len)
{
  unsigned int prev_len=mkstr->data_alloc;
  mkstr->data_alloc=len;
  mkstr->data=realloc(mkstr->data,mkstr->data_alloc*sizeof(unsigned char));
  if ((mkstr->data_alloc!=0)&&(mkstr->data==NULL))
      return -1;
  if (prev_len<mkstr->data_alloc)
  {
    int i;
    for (i=prev_len;i<mkstr->data_alloc;i++)
        mkstr->data[i]=0;
  }
  return ERR_NONE;
}

/**
 * Adds encoded STR entry into STR_Maker.
 * The block at given pointer is copied into STR_Maker data block,
 * and its offset is added to STR_Maker's offsets list.
 * The original edata block may be freed after calling this function.
 * @return Returns ERR_NONE on success.
 */
short strmaker_add_entry(struct STR_Maker *mkstr,unsigned char *edata,unsigned long len)
{
  short result=ERR_NONE;
  if ((result==ERR_NONE)&&(mkstr->data_len+len+6>mkstr->data_alloc))
      result=strmaker_set_dataalloc(mkstr,mkstr->data_len+len+32);
  if ((result==ERR_NONE)&&(mkstr->offs_count+1>mkstr->offs_alloc))
      result=strmaker_set_offsalloc(mkstr,mkstr->offs_count+4);
  if (result!=ERR_NONE)
      return result;
  while ((mkstr->data_len%4)>0)
  {
      mkstr->data[mkstr->data_len]=0;
      mkstr->data_len++;
  }
  memcpy(mkstr->data+mkstr->data_len,edata,len);
  mkstr->offsets[mkstr->offs_count]=mkstr->data_len;
  mkstr->data_len+=len;
  mkstr->offs_count++;
  while ((mkstr->data_len%4)>0)
  {
      mkstr->data[mkstr->data_len]=0;
      mkstr->data_len++;
  }
  return ERR_NONE;
}

/**
 * Encodes given Unicode text entry and places it in STR_Maker structure.
 * @return Returns ERR_NONE on success.
 */
short strmaker_add_unicode_entry(struct STR_Maker *mkstr,unsigned short *udata,short flags)
{
  short result;
  long udata_len=unicode_strlen(udata);
  unsigned char *edata;
  long edata_len;
  //result=str_data_encode(&edata,&edata_len,mkstr->uni2mb,mkstr->uni2mb_count,udata,udata_len);
  result=str_data_encode_r(&edata,&edata_len,mkstr->mb2uni,mkstr->mb2uni_count,
      udata,udata_len);
  //printf("Have: ");int i;
  //for (i=0;i<edata_len;i++) printf("%02x ",edata[i]);
  //printf("\n");
  if (result!=ERR_NONE)
  {
      if (flags&STRFLAG_VERBOSE)
        str_error("Error on unicode string encoding");
      return result;
  }
  result=strmaker_add_entry(mkstr,edata,edata_len);
  free(edata);
  if (result!=ERR_NONE)
  {
      if (flags&STRFLAG_VERBOSE)
        str_error("Error on adding STR_Maker entry");
      return result;
  }
  if (flags&STRFLAG_DEBUG)
      printf("Unicode entry added\n");
  return ERR_NONE;
}

/**
 * Decodes STR entry of given index and returns it in udata pointer.
 * @return Returns size of the entry, or negative error code.
 */
short strmaker_get_unicode_entry(const struct STR_Maker *mkstr,
    unsigned short **udata,int index,short flags)
{
    char *edata;
    int edata_len;
    // Get the encoded data
    edata_len=strmaker_get_entry(mkstr,&edata,index,flags);
    if (flags&STRFLAG_DEBUG)
        printf("Got entry, size %d\n",edata_len);
    if ((edata_len<=0)||(edata==NULL))
    {
      (*udata)=malloc(2*sizeof(unsigned short));
      if ((*udata)!=NULL) (*udata)[0]=0;
      if (edata_len==0) return 0;
      if (edata_len>0) edata_len=-1;
      if (flags&STRFLAG_VERBOSE)
        str_error("Error in STR structure");
      return edata_len;
    }
    // Decode it
    short result;
    long udata_len;
    result=str_data_decode(udata,&udata_len,mkstr->mb2uni,mkstr->mb2uni_count,
        edata,edata_len);
    if (result!=ERR_NONE)
        return result;
    return udata_len;
}

/**
 * Clears the STR_Maker structure, drops any pointers.
 * @return Returns ERR_NONE on success.
 */
short strmaker_clear(struct STR_Maker *mkstr)
{
  int i;
  for (i=0;i<4;i++)
    mkstr->magic[i]='\0';
  mkstr->offsets=NULL;
  mkstr->offs_alloc=0;
  mkstr->offs_count=0;
  mkstr->data=NULL;
  mkstr->data_alloc=0;
  mkstr->data_len=0;
  mkstr->mb2uni=NULL;
  mkstr->uni2mb=NULL;
  mkstr->mb2uni_count=0;
  mkstr->uni2mb_count=0;
  mkstr->file_id=0;
  mkstr->iosize=0;
  return ERR_NONE;
}

/**
 * Frees the STR_Maker structure and all sub-structures.
 * @return Returns ERR_NONE on success.
 */
short strmaker_free(struct STR_Maker *mkstr)
{
  free(mkstr->offsets);
  free(mkstr->data);
  free(mkstr->mb2uni);
  free(mkstr->uni2mb);
  free(mkstr);
  return ERR_NONE;
}

/**
 * Reads MbToUni file into STR_Maker structure.
 * @return Returns ERR_NONE on success.
 */
short str_mb2uni_fread(struct STR_Maker *mkstr,FILE *fp,short flags)
{
  long nread=0;
  int i;
  char magic[5];
  nread+=fread(magic,1,4,fp);
  magic[4]=0;
  if ((nread!=4)||(memcmp(magic,mb2uni_magic,4)!=0))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("File is not Mb2Uni - bad magic value");
      return -1;
  }
  unsigned short val1=read_int16_le_file(fp); // I have no idea what it is
  long dlen=file_length_opened(fp);
  if ((dlen<8)||(dlen>65538))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Bad Mb2Uni file - wrong size");
      return -1;
  }
  dlen-=6;
  mkstr->mb2uni=malloc(dlen);
  if (mkstr->mb2uni==NULL)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Can't malloc codepage conversion array");
      return -1;
  }
  nread=fread(mkstr->mb2uni,1,dlen,fp);
  mkstr->mb2uni_count = (dlen>>1);
  if (nread!=dlen)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("%s when reading Mb2Uni file",strerror(errno));
      return -1;
  }
  return ERR_NONE;
}

/**
 * Reads UniToMb file into STR_Maker structure.
 * @return Returns ERR_NONE on success.
 */
short str_uni2mb_fread(struct STR_Maker *mkstr,FILE *fp,short flags)
{
  long nread=0;
  int i;
  char magic[5];
  nread+=fread(magic,1,4,fp);
  magic[4]=0;
  if ((nread!=4)||(memcmp(magic,uni2mb_magic,4)!=0))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("File is not Uni2Mb - bad magic value");
      return -1;
  }
  unsigned short val1=read_int16_le_file(fp); // I have no idea what it is
  long dlen=file_length_opened(fp);
  if ((dlen<8)||(dlen>65538))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Bad Uni2Mb file - wrong size");
      return -1;
  }
  dlen-=6;
  mkstr->uni2mb=malloc(dlen);
  if (mkstr->uni2mb==NULL)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Can't malloc codepage conversion array");
      return -1;
  }
  nread=fread(mkstr->uni2mb,1,dlen,fp);
  mkstr->uni2mb_count = (dlen>>1);
  if (nread!=dlen)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("%s when reading Uni2Mb file",strerror(errno));
      return -1;
  }
  return ERR_NONE;
}

/**
 * Loads STR maker from current position of disk file.
 * Requires the STR_Maker to be allocated before.
 * @return Returns ERR_NONE on success.
 */
short strmaker_fread(struct STR_Maker *mkstr,FILE *fp,short flags)
{
  long nread=0;
  int i;
  short result;
  nread += fread(mkstr->magic,1,4,fp);
  if ((nread!=4)||(memcmp(mkstr->magic,str_magic,4)!=0))
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("File is not STR - bad magic value");
      return -1;
  }
  mkstr->file_id=read_int32_le_file(fp);
  nread += 4;
  int offs_num;
  offs_num=read_int32_le_file(fp);
  result=strmaker_set_offsalloc(mkstr,offs_num+2);
  nread += 4;
  if (result!=ERR_NONE)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Cannot allocate memory for offsets array");
      return result;
  }
  int offs_delta;
  offs_delta = (offs_num<<2);
  for (i=0;i<offs_num;i++)
  {
      mkstr->offsets[mkstr->offs_count]=read_int32_le_file(fp)-offs_delta;
      mkstr->offs_count++;
      nread += 4;
  }
  mkstr->disksize=file_length_opened(fp);
  long length=mkstr->disksize-SIZEOF_STR_Header-offs_delta;
  if (length<1)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("STR file too small");
      return -1;
  }
  result=strmaker_set_dataalloc(mkstr,length+16);
  if (result!=ERR_NONE)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("Can't malloc data block");
      return -1;
  }
  mkstr->data_len=length;
  nread=fread(mkstr->data,1,length,fp);
  if (nread!=length)
  {
      if (flags&STRFLAG_VERBOSE)
      {
        if (!feof(fp))
          str_ferror("%s when reading STR file",strerror(errno));
        else
          str_ferror("%s when reading STR file","Sudden EOF");
      }
      return -1;
  }
  return ERR_NONE;
}

/**
 * Writes STR maker into disk file.
 * @return Returns ERR_NONE on success.
 */
short strmaker_fwrite(struct STR_Maker *mkstr,FILE *fp,short flags)
{
  fwrite(str_magic,1,4,fp);
  write_int32_le_file (fp,mkstr->file_id);
  write_int32_le_file (fp,mkstr->offs_count);
  int offs_delta;
  offs_delta = (mkstr->offs_count<<2);
  int k;
  for (k=0;k<mkstr->offs_count;k++)
      write_int32_le_file(fp,mkstr->offsets[k]+offs_delta);
  k=fwrite(mkstr->data,1,mkstr->data_len,fp);
  if (k!=mkstr->data_len)
  {
      if (flags&STRFLAG_VERBOSE)
        str_ferror("%s when writing STR file",strerror(errno));
      return -1;
  }
  return ERR_NONE;
}

/**
 * Gives a specific entry from STR_Maker structure.
 * The entry is not copied nor decoded, just returned directly in edata pointer.
 * @return Returns size of the entry, and its pointer in edata.
 */
int strmaker_get_entry(const struct STR_Maker *mkstr,char **edata,unsigned int entryidx,short flags)
{
  if (entryidx>=mkstr->offs_count)
  {
      (*edata)=NULL;
      return 0;
  }
  long start=mkstr->offsets[entryidx];
  if (start<0)
  {
      (*edata)=NULL;
      return 0;
  }
  (*edata)=mkstr->data+start;
  entryidx++;
  if (entryidx>=mkstr->offs_count)
      return (mkstr->data_len - start);
  long end=mkstr->offsets[entryidx];
  if (end<start)
      return (mkstr->data_len - start);
  return (end - start);
}
