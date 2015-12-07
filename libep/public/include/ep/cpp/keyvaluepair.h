#pragma once
#if !defined(_EPKEYVALUEPAIR_HPP)
#define _EPKEYVALUEPAIR_HPP

namespace ep {

template <typename K, typename V>
struct KVPRef
{
  KVPRef(const K &key, V &value) : key(key), value(value) {}

  const K &key;
  V &value;
};

template <typename K, typename V>
struct KVP
{
  KVP() {}
  KVP(KVP<K, V> &&val) : key(std::move(val.key)), value(std::move(val.value)) {}
  KVP(const KVP<K, V> &val) : key(val.key), value(val.value) {}

  KVP(const K &key, const V &value) : key(key), value(value) {}
  KVP(const K &key, V &&value) : key(key), value(std::move(value)) {}
  KVP(K &&key, const V &value) : key(std::move(key)), value(value) {}
  KVP(K &&key, V &&value) : key(std::move(key)), value(std::move(value)) {}

  KVP(const KVPRef<K, V> &val) : key(val.key), value(val.value) {}

  // TODO: consider, should value be first? it is more likely to have alignment requirements.
  //       conversely, key is more frequently accessed, so should be in the first cache line...
  K key;
  V value;
};

} // namespace ep

#endif // _EPKEYVALUEPAIR_HPP
