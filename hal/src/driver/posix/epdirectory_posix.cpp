#include "driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_POSIX

#include "hal/directory.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

bool HalDirectory_FindFirst(EPFind *pFind, const char *pFolderPath, EPFindData *pFindData)
{
  char path[260];

  if (pFolderPath[0] == '\0')
    strcpy(path, "./");
  else
    snprintf(path, sizeof(path), "%s%s", pFolderPath, (pFolderPath[strlen(pFolderPath - 1)] != '/' ? "/" : ""));

  // open the directory
  DIR *pDir = opendir(path);

  if (!pDir)
  {
    epDebugPrintf("Couldnt open directory '%s'\n", path);
    return false;
  }

  pFind->pFilesystemData = (void*)pDir;

  pFind->searchPattern[0] = '\0';
  strncat(pFind->searchPattern, path, sizeof(pFind->searchPattern) - 1);

  bool bFound = HalDirectory_FindNext(pFind, pFindData);
  if (!bFound)
    HalDirectory_FindClose(pFind);
  return bFound;
}

bool HalDirectory_FindNext(EPFind *pFind, EPFindData *pFindData)
{
  dirent *pDir = readdir((DIR*)pFind->pFilesystemData);

  while (pDir && (!strcmp(pDir->d_name, ".") || !strcmp(pDir->d_name, "..")))
    pDir = readdir((DIR*)pFind->pFilesystemData);
  if (!pDir)
    return false;

  pFindData->pSystemPath[0] = '\0';
  strncat(pFindData->pSystemPath, pFind->searchPattern, sizeof(pFindData->pSystemPath) - 1);

  char pFilePath[260];
  snprintf(pFilePath, sizeof(pFilePath), "%s%s", pFindData->pSystemPath, pDir->d_name);

  struct stat statbuf;
  if (stat(pFilePath, &statbuf) < 0)
    return false;

  pFindData->attributes = (S_ISDIR(statbuf.st_mode) ? EPFA_Directory : 0) |
    (S_ISLNK(statbuf.st_mode) ? EPFA_SymLink : 0) |
    (pDir->d_name[0] == '.' ? EPFA_Hidden : 0);
  // TODO: Set EPFA_ReadOnly from write privileges
  pFindData->fileSize = statbuf.st_size;
  pFindData->writeTime.ticks = (uint64_t)statbuf.st_mtime;
  pFindData->accessTime.ticks = (uint64_t)statbuf.st_atime;

  pFindData->pFilename[0] = '\0';
  strncat(pFindData->pFilename, pDir->d_name, sizeof(pFindData->pFilename) - 1);

  return true;
}

void HalDirectory_FindClose(EPFind *pFind)
{
  closedir((DIR*)pFind->pFilesystemData);
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_POSIX
