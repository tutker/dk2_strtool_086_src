/******************************************************************************/
/** @file strtool.c
 * Tool for r/w of DK2 STR text strings files.
 * @par Purpose:
 *     Allows reading/writing of DK2 STR (text strings) files.
 * @par Comment:
       Main program - contains the main() function with console interface code.
 * @author   Tomasz Lis
 * @date     29 Jul 2008 - 16 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unitext.h"
#include "strfile.h"

int main(int argc, char *argv[])
{
    printf("\nDungeon Keeper 2 text STR tool %s\n",VER_STRING);
    printf("designed for Polish Dungeon Keeper Team\n");
    printf("-------------------------------\n");
    if ((argc<3)||(strlen(argv[2])!=1))
    {
        printf("Not enought parameters.\n");
        printf("Usage:\n");
        printf("  %s <strfile> <operation>\n","strtool");
        printf("The <strfile> should be given without extension.\n");
        printf("Valid <operations> are:\n");
        printf("  x: eXport entries into text file \n");
        printf("  c: Create the str file using text file\n");
        printf("  d: Dump str file structure data\n");
        printf("\n");
        system("PAUSE");	
    	return 1;
    }
  struct STR_File *strfile;
  short flags = STRFLAG_VERBOSE;
  int fname_len=strlen(argv[1]);
  char *strfname=malloc(fname_len+5);
  char *txtfname=malloc(fname_len+5);
  if ((strfname==NULL)||(txtfname==NULL))
  {
    str_error("Can't allocate memory for file names");
    return 4;
  }
  strfile=NULL;
  sprintf(strfname,"%s.str",argv[1]);
  sprintf(txtfname,"%s.txt",argv[1]);
  char operatn=tolower(argv[2][0]);
  switch (operatn)
  {
  case 'd':
    {
      printf("STR Data Dump mode\n");
      strfile=str_open(strfname,flags|STRFLAG_DEBUG);
      if (strfile==NULL)
      {
        return 2;
      }
      printf("Dump finished.\n");
    };break;
  case 'c':
      printf("Opening Unicode Text file...\n");
      strfile=str_open_unicode(txtfname,flags);
      if (strfile==NULL)
      {
        return 2;
      }
      printf("Writing STR file...\n");
      str_write(strfile,strfname,flags);
      printf("Creation finished.\n");
      break;
  case 'e':
  case 'x':
      printf("Opening STR file...\n");
      strfile=str_open(strfname,flags);
      if (strfile==NULL)
      {
        return 2;
      }
      printf("Wriring Unicode Text file...\n");
      str_write_unicode(strfile,txtfname,flags);
      printf("Extraction finished.\n");
      break;
  default:
      printf("Unknown opertation symbol.\n");
      printf("Exiting without any changes.\n");
      break;
  }
  free(strfname);
  free(txtfname);
  if (!str_close(strfile,flags))
    return 3;
  return 0;
}
