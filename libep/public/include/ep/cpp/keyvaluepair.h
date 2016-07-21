#pragma once
#if !defined(_EPKEYVALUEPAIR_HPP)
#define _EPKEYVALUEPAIR_HPP

namespace ep {

template <typename K, typename V>
struct KVPRef
{
  KVPRef(const KVPRef<K, V> &kvp) : key(kvp.key), value(kvp.value) {}

  // these are templated for const promotion
  template <typename _K, typename _V> KVPRef(_K &key, _V &value) : key(key), value(value) {}
  template <typename _K, typename _V> KVPRef(const KVPRef<_K, _V> &kvp) : key(kvp.key), value(kvp.value) {}

  K &key;
  V &value;
};

template <typename K, typename V>
struct KVP
{
  KVP() {}
  KVP(KVP<K, V> &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  KVP(const KVP<K, V> &val) : key(val.key), value(val.value) {}

  KVP(K &&key, V &&value) : key(std::move(key)), value(std::move(value)) {}
  KVP(const K &key, V &&value) : key(key), value(std::move(value)) {}
  KVP(K &&key, const V &value) : key(std::move(key)), value(value) {}
  KVP(const K &key, const V &value) : key(key), value(value) {}

  // these are templated for const promotion
  template <typename _K, typename _V> KVP(KVP<_K, _V> &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  template <typename _K, typename _V> KVP(const KVP<_K, _V> &val) : key(val.key), value(val.value) {}
  template <typename _K, typename _V> KVP(const KVPRef<_K, _V> &val) : key(val.key), value(val.value) {}

  KVP<K, V>& operator=(KVP<K, V> &&val) { key = std::move(val.key); value = std::move(val.value); return *this; }
  KVP<K, V>& operator=(const KVP<K, V> &val) { key = val.key; value = val.value; return *this; }

  // TODO: consider, should value be first? it is more likely to have alignment requirements.
  //       conversely, key is more frequently accessed, so should be in the first cache line...
  K key;
  V value;
};


template <typename K, typename V>
ptrdiff_t epStringify(Slice<char> buffer, String epUnusedParam(format), KVPRef<K, V> kvp, const VarArg *epUnusedParam(pArgs))
{
  size_t offset = 0;
  if (buffer)
  {
    offset += epStringifyTemplate(buffer, nullptr, kvp.key, nullptr);
    offset += String(": ").copyTo(buffer.strip(offset));
    offset += epStringifyTemplate(buffer.strip(offset), nullptr, kvp.value, nullptr);
  }
  else
  {
    offset += epStringifyTemplate(nullptr, nullptr, kvp.key, nullptr);
    offset += String(": ").length;
    offset += epStringifyTemplate(nullptr, nullptr, kvp.value, nullptr);
  }

  return offset;
}
template <typename K, typename V>
ptrdiff_t epStringify(Slice<char> buffer, String format, const KVP<K, V> &kvp, const VarArg *pArgs)
{
  return epStringify(buffer, format, KVPRef<K, V>(kvp.key, kvp.value), pArgs);
}

} // namespace ep

#endif // _EPKEYVALUEPAIR_HPP
