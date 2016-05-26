#pragma once
#ifndef EP_HALDIRECTORY_H
#define EP_HALDIRECTORY_H

#include "ep/cpp/platform.h"

/**
* File attributes.
* These are a set of file attributes.
*/
enum EPFileAttributes
{
  EPFA_Directory = 1,	/**< File is a directory */
  EPFA_SymLink = 2,	/**< File is a symbolic link */
  EPFA_Hidden = 4,		/**< File is hidden */
  EPFA_ReadOnly = 8,	/**< File is read only */

  EPFA_ForceInt = 0x7FFFFFFF	/**< Force EPFileAttributes to an int type */
};

struct EPFind
{
  char searchPattern[256];
  void *pFilesystemData;
};

struct EPFileTime
{
  uint64_t ticks;
};

/**
* FileSystem find data.
* Structure used to return information about a file in the filesystem.
*/
struct EPFindData
{
  char pFilename[224];	/**< The files filename */
  char pSystemPath[260];	/**< The system path to the file */
  uint32_t attributes;		/**< The files attributes */
  uint64_t fileSize;		/**< The files size */
  EPFileTime writeTime;	/**< Last time the file was written */
  EPFileTime accessTime;	/**< Last time the file was accessed */
};

bool HalDirectory_FindFirst(EPFind *pFind, const char *pSearchPattern, EPFindData *pFindData);
bool HalDirectory_FindNext(EPFind *pFind, EPFindData *pFindData);
void HalDirectory_FindClose(EPFind *pFind);

#endif // EP_HALDIRECTORY_H
