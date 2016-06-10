#pragma once
#if !defined(_EP_IMETADATA_HPP)
#define _EP_IMETADATA_HPP

#include "ep/cpp/component/component.h"

namespace ep {

class IMetadata
{
public:
  virtual size_t NumRecords() const = 0;
  virtual void InsertRR(Variant &&key, Variant &&value) = 0;
  virtual void InsertLR(const Variant &key, Variant &&value) = 0;
  virtual void InsertRL(Variant &&key, const Variant &value) = 0;
  virtual void InsertLL(const Variant &key, const Variant &value) = 0;

  virtual void Remove(const Variant &key) = 0;
  virtual bool Exists(const Variant &key) const = 0;
  virtual Variant Get(const Variant &key) const = 0;
};

}

#endif // _EP_IMETADATA_HPP
