#pragma once
#ifndef _EP_METADATA_H
#define _EP_METADATA_H

#include "ep/cpp/internal/i/imetadata.h"
#include "ep/cpp/component/resource/kvpstore.h"

namespace ep {

SHARED_CLASS(Metadata);

class Metadata : public KVPStore
{
  EP_DECLARE_COMPONENT_WITH_IMPL(ep, Metadata, IMetadata, KVPStore, EPKERNEL_PLUGINVERSION, "Metadata resource", 0)

public:
  virtual size_t numRecords() const { return pImpl->NumRecords(); }

  virtual void insert(Variant &&key, Variant &&value) { pImpl->InsertRR(std::move(key), std::move(value)); }
  virtual void insert(const Variant &key, Variant &&value) { pImpl->InsertLR(key, std::move(value)); }
  virtual void insert(Variant &&key, const Variant &value) { pImpl->InsertRL(std::move(key), value); }
  virtual void insert(const Variant &key, const Variant &value) { pImpl->InsertLL(key, value); }

  virtual void remove(const Variant &key) { pImpl->Remove(key); }
  virtual bool exists(const Variant &key) const { return pImpl->Exists(key); }
  virtual Variant get(const Variant &key) const { return pImpl->Get(key); }

protected:
  Metadata(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : KVPStore(pType, pKernel, uid, initParams)
  {
    pImpl = createImpl(initParams);
  }

  Array<const PropertyInfo> getProperties() const;
  Array<const MethodInfo> getMethods() const;

private:
  void insertMethod(const Variant &key, const Variant &value) { insert(key, value); }
};

}

#endif // _EP_METADATA_H
