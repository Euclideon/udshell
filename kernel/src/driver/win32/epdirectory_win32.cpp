#include "hal/driver.h"

#if EPSYSTEM_DRIVER == EPDRIVER_WIN32

#include "hal/haldirectory.h"

bool HalDirectory_FindFirst(EPFind *pFind, const char *pFolderPath, EPFindData *pFindData)
{
  WIN32_FIND_DATA fd;
  HANDLE hFind;

  char path[260];
  snprintf(path, sizeof(path), "%s%s*", pFolderPath, (pFolderPath[strlen(pFolderPath - 1)] != '/' ? "/" : ""));

  wchar_t wpath[260];
  int len = MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, sizeof(wpath)/sizeof(wpath[0]));
  if (len == 0)
    return false;

  hFind = FindFirstFileW(wpath, &fd);
  if (hFind == INVALID_HANDLE_VALUE)
    return false;

  BOOL more = TRUE;
  while (!wcscmp(fd.cFileName, L".") || !wcscmp(fd.cFileName, L"..") && more)
    more = FindNextFile(hFind, &fd);
  if (!more)
  {
    FindClose(hFind);
    return false;
  }

  snprintf(pFindData->pSystemPath, sizeof(pFindData->pSystemPath), "%s", pFolderPath);

  char *pLast = strrchr(pFindData->pSystemPath, '/');
  if (pLast)
    pLast[1] = 0;
  else
    pFindData->pSystemPath[0] = NULL;

  pFind->pFilesystemData = (void*)hFind;

  pFind->searchPattern[0] = '\0';
  strcpy_s(pFind->searchPattern, sizeof(pFind->searchPattern), pFindData->pSystemPath);

  pFindData->attributes = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? EPFA_Directory : 0) |
    ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? EPFA_Hidden : 0) |
    ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? EPFA_ReadOnly : 0);
  pFindData->fileSize = (uint64_t)fd.nFileSizeLow | (((uint64_t)fd.nFileSizeHigh) << 32);
  pFindData->writeTime.ticks = (uint64_t)fd.ftLastWriteTime.dwHighDateTime << 32 | (uint64_t)fd.ftLastWriteTime.dwLowDateTime;
  pFindData->accessTime.ticks = (uint64_t)fd.ftLastAccessTime.dwHighDateTime << 32 | (uint64_t)fd.ftLastAccessTime.dwLowDateTime;
  WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, pFindData->pFilename, sizeof(pFindData->pFilename), NULL, NULL);

  return true;
}

bool HalDirectory_FindNext(EPFind *pFind, EPFindData *pFindData)
{
  WIN32_FIND_DATA fd;

  BOOL more = FindNextFile((HANDLE)pFind->pFilesystemData, &fd);

  if (!more)
    return false;

  snprintf(pFindData->pSystemPath, sizeof(pFindData->pSystemPath), "%s", pFind->searchPattern);

  char *pLast = strrchr(pFindData->pSystemPath, '/');
  if (pLast)
    pLast[1] = 0;
  else
    pFindData->pSystemPath[0] = 0;

  pFindData->attributes = ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? EPFA_Directory : 0) |
    ((fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? EPFA_Hidden : 0) |
    ((fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? EPFA_ReadOnly : 0);
  pFindData->writeTime.ticks = (uint64_t)fd.ftLastWriteTime.dwHighDateTime << 32 | (uint64_t)fd.ftLastWriteTime.dwLowDateTime;
  pFindData->accessTime.ticks = (uint64_t)fd.ftLastAccessTime.dwHighDateTime << 32 | (uint64_t)fd.ftLastAccessTime.dwLowDateTime;
  pFindData->fileSize = (uint64_t)fd.nFileSizeLow | (((uint64_t)fd.nFileSizeHigh) << 32);
  WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, pFindData->pFilename, sizeof(pFindData->pFilename), NULL, NULL);

  return true;
}

void HalDirectory_FindClose(EPFind *pFind)
{
  FindClose((HANDLE)pFind->pFilesystemData);
}

#else
EPEMPTYFILE
#endif // EPSYSTEM_DRIVER == EPDRIVER_WIN32
