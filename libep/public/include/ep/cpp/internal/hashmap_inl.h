namespace ep {

template <typename V, typename K, typename HashPred>
HashMap<V, K, HashPred>::HashMap(size_t tableSize, size_t itemBucketSize)
  : tableSizeMask(tableSize - 1), pool(itemBucketSize)
{
  EPASSERT(tableSize > 0 && (tableSize & (tableSize-1)) == 0, "tableSize must be power-of-2!");
  ppTable = epAllocType(Node*, tableSize, epAF_Zero);
  EPTHROW_IF_NULL(ppTable, epR_AllocFailure, "Allocation failed");
}
template <typename V, typename K, typename HashPred>
HashMap<V, K, HashPred>::HashMap(HashMap &&rval)
  : ppTable(rval.ppTable), tableSizeMask(rval.tableSizeMask), pool(std::move(rval.pool))
{
  rval.ppTable = nullptr;
}

template <typename V, typename K, typename HashPred>
HashMap<V, K, HashPred>::~HashMap()
{
  if (ppTable)
  {
    size_t tableSize = tableSizeMask + 1;
    for (size_t i = 0; i < tableSize; ++i)
    {
      while (ppTable[i])
      {
        Node *pNext = ppTable[i]->pNext;
        pool.Delete(ppTable[i]);
        ppTable[i] = pNext;
      }
    }
    epFree(ppTable);
  }
}

template <typename V, typename K, typename HashPred>
size_t HashMap<V, K, HashPred>::Size() const
{
  return pool.Size();
}
template <typename V, typename K, typename HashPred>
bool HashMap<V, K, HashPred>::Empty() const
{
  return pool.Size() == 0;
}

template <typename V, typename K, typename HashPred>
template <typename Key, typename Val>
V* HashMap<V, K, HashPred>::Insert(Key&& key, Val&& val)
{
  Node **ppBucket = GetBucket(key);
  V *pVal = GetValue(*ppBucket, key);
  if (pVal)
    return pVal;
  Node *pNode = pool.New(std::forward<Key>(key), std::forward<Val>(val));
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return &pNode->data.value;
}
template <typename V, typename K, typename HashPred>
V* HashMap<V, K, HashPred>::Insert(KVP<K, V> &&v)
{
  return Insert(std::move(v.key), std::move(v.value));
}
template <typename V, typename K, typename HashPred>
V* HashMap<V, K, HashPred>::Insert(const KVP<K, V> &v)
{
  return Insert(v.key, v.value);
}

template <typename V, typename K, typename HashPred>
template <typename Key>
V* HashMap<V, K, HashPred>::InsertLazy(Key&& key, std::function<V()> lazy)
{
  Node **ppBucket = GetBucket(key);
  V *pVal = GetValue(*ppBucket, key);
  if (pVal)
    return pVal;
  Node *pNode = pool.New(std::forward<Key>(key), lazy());
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return &pNode->data.value;
}

//  template <typename... Args>
//  V* Replace(Args&&... args)
//  {
//  }

template <typename V, typename K, typename HashPred>
void HashMap<V, K, HashPred>::Remove(const K &key)
{
  Node **ppBucket = GetBucket(key);
  while (*ppBucket)
  {
    if (HashPred::eq((*ppBucket)->data.key, key))
    {
      Node *pNext = (*ppBucket)->pNext;
      pool.Delete(*ppBucket);
      *ppBucket = pNext;
    }
    else
      ppBucket = &(*ppBucket)->pNext;
  }
}

template <typename V, typename K, typename HashPred>
const V* HashMap<V, K, HashPred>::Get(const K &key) const
{
  return GetValue(*GetBucket(key), key);
}
template <typename V, typename K, typename HashPred>
V* HashMap<V, K, HashPred>::Get(const K &key)
{
  return GetValue(*GetBucket(key), key);
}

template <typename V, typename K, typename HashPred>
const V& HashMap<V, K, HashPred>::operator[](const K &key) const
{
  const V *pV = Get(key);
  EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
  return *pV;
}
template <typename V, typename K, typename HashPred>
V& HashMap<V, K, HashPred>::operator[](const K &key)
{
  V *pV = Get(key);
  EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
  return *pV;
}

template <typename V, typename K, typename HashPred>
auto HashMap<V, K, HashPred>::begin() const -> typename HashMap<V, K, HashPred>::Iterator
{
  return Iterator(ppTable, ppTable + tableSizeMask + 1);
}
template <typename V, typename K, typename HashPred>
auto HashMap<V, K, HashPred>::end() const -> typename HashMap<V, K, HashPred>::Iterator
{
  return Iterator(ppTable + tableSizeMask + 1, ppTable + tableSizeMask + 1);
}

template <typename V, typename K, typename HashPred>
auto HashMap<V, K, HashPred>::GetBucket(const K &key) const -> typename HashMap<V, K, HashPred>::Node**
{
  auto h = HashPred::hash(key);
  size_t bucket = h & tableSizeMask;
  return &ppTable[bucket];
}

template <typename V, typename K, typename HashPred>
V* HashMap<V, K, HashPred>::GetValue(Node *pBucket, const K &key) const
{
  while (pBucket)
  {
    if (HashPred::eq(pBucket->data.key, key))
      return &pBucket->data.value;
    pBucket = pBucket->pNext;
  }
  return nullptr;
}


// iterator methods

template <typename V, typename K, typename HashPred>
HashMap<V, K, HashPred>::Iterator::Iterator(Node **ppStart, Node **ppEnd)
  : ppEnd(ppEnd)
{
  while (ppStart != ppEnd && !*ppStart)
    ++ppStart;
  if (ppStart != ppEnd)
    pItem = *ppStart;
  else
    pItem = nullptr;
  this->ppStart = ppStart;
}

template <typename V, typename K, typename HashPred>
auto HashMap<V, K, HashPred>::Iterator::operator++() -> typename HashMap<V, K, HashPred>::Iterator&
{
  if (!pItem)
    return *this;
  pItem = pItem->pNext;
  if (!pItem)
  {
    do {
      ++ppStart;
    } while (ppStart != ppEnd && !*ppStart);
    if (ppStart != ppEnd)
      pItem = *ppStart;
    else
      pItem = nullptr;
  }
  return *this;
}

template <typename V, typename K, typename HashPred>
bool HashMap<V, K, HashPred>::Iterator::operator!=(Iterator rhs)
{
  return pItem != rhs.pItem;
}

template <typename V, typename K, typename HashPred>
const KVPRef<K, V> HashMap<V, K, HashPred>::Iterator::operator*() const
{
  const KVPRef<K, V> r = KVPRef<K, V>(pItem->data.key, pItem->data.value);
  return std::move(r);
}
template <typename V, typename K, typename HashPred>
KVPRef<K, V> HashMap<V, K, HashPred>::Iterator::operator*()
{
  return KVPRef<K, V>(pItem->data.key, pItem->data.value);
}

} // namespace ep
