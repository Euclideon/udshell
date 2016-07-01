#pragma once
#if !defined(_EPFILESYSTEM_HPP)
#define _EPFILESYSTEM_HPP

#include "ep/cpp/variant.h"

namespace ep {

  EP_BITFIELD(FileAttributes,
    Directory,
    SymLink,
    Hidden,
    ReadOnly
  );

  struct FileTime
  {
    uint64_t ticks;
  };

  struct FindData
  {
    SharedString filename;      //!< The files filename
    SharedString path;          //!< The system path to the file
    uint64_t fileSize;          //!< The files size
    FileAttributes attributes;  //!< The files attributes
    FileTime writeTime;         //!< Last time the file was written
    FileTime accessTime;        //!< Last time the file was accessed
  };

  class Directory
  {
    void *pDirectoryHandle;
    FindData fd;

  public:
    Directory(String searchPattern);
    ~Directory();

    bool Empty() const { return pDirectoryHandle == nullptr; }

    FindData GetFront() const { return fd; }
    FindData PopFront();
  };

  inline Variant epToVariant(FileTime time) { return time.ticks; }
  inline void epFromVariant(const Variant &variant, FileTime *pTime) { pTime->ticks = variant.as<uint64_t>(); }

  Variant epToVariant(const FindData& fd);

} // namespace ep

#endif // _EPFILESYSTEM_HPP
