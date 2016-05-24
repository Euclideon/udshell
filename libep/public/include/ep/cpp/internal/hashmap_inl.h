namespace ep {

template <typename K, typename V, typename HashPred>
HashMap<K, V, HashPred>::HashMap(size_t tableSize, size_t itemBucketSize)
  : tableSizeMask(tableSize - 1), pool(itemBucketSize)
{
  EPASSERT(tableSize > 0 && (tableSize & (tableSize-1)) == 0, "tableSize must be power-of-2!");
  ppTable = epAllocType(Node*, tableSize, epAF_Zero);
  EPTHROW_IF_NULL(ppTable, Result::AllocFailure, "Allocation failed");
}
template <typename K, typename V, typename HashPred>
HashMap<K, V, HashPred>::HashMap(HashMap &&rval)
  : ppTable(rval.ppTable), tableSizeMask(rval.tableSizeMask), pool(std::move(rval.pool))
{
  rval.ppTable = nullptr;
}

template <typename K, typename V, typename HashPred>
HashMap<K, V, HashPred>::~HashMap()
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

template <typename K, typename V, typename HashPred>
size_t HashMap<K, V, HashPred>::size() const
{
  return pool.size();
}
template <typename K, typename V, typename HashPred>
bool HashMap<K, V, HashPred>::empty() const
{
  return pool.size() == 0;
}

template <typename K, typename V, typename HashPred>
void HashMap<K, V, HashPred>::clear()
{
  for (size_t i = 0; (i < tableSizeMask + 1) && pool.size(); ++i)
  {
    while (ppTable[i])
    {
      Node *pNext = ppTable[i]->pNext;
      pool.Delete(ppTable[i]);
      ppTable[i] = pNext;
    }
  }
}

template <typename K, typename V, typename HashPred>
template <typename Key, typename Val>
V& HashMap<K, V, HashPred>::insert(Key&& key, Val&& val)
{
  Node **ppBucket = getBucket(key);
  V *pVal = getValue(*ppBucket, key);
  if (pVal)
    EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
  Node *pNode = pool.New(std::forward<Key>(key), std::forward<Val>(val));
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return pNode->data.value;
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::insert(KVP<K, V> &&v)
{
  return insert(std::move(v.key), std::move(v.value));
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::insert(const KVP<K, V> &v)
{
  return insert(v.key, v.value);
}

template <typename K, typename V, typename HashPred>
template <typename Key>
V& HashMap<K, V, HashPred>::tryInsert(Key&& key, V&& val)
{
  Node **ppBucket = getBucket(key);
  V *pVal = getValue(*ppBucket, key);
  if (pVal)
    return *pVal;
  Node *pNode = pool.New(std::forward<Key>(key), std::move(val));
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return pNode->data.value;
}
template <typename K, typename V, typename HashPred>
template <typename Key>
V& HashMap<K, V, HashPred>::tryInsert(Key&& key, const V& val)
{
  Node **ppBucket = getBucket(key);
  V *pVal = getValue(*ppBucket, key);
  if (pVal)
    return *pVal;
  Node *pNode = pool.New(std::forward<Key>(key), val);
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return pNode->data.value;
}
template <typename K, typename V, typename HashPred>
template <typename Key>
V& HashMap<K, V, HashPred>::tryInsert(Key&& key, std::function<V()> lazy)
{
  Node **ppBucket = getBucket(key);
  V *pVal = getValue(*ppBucket, key);
  if (pVal)
    return *pVal;
  Node *pNode = pool.New(std::forward<Key>(key), lazy());
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return pNode->data.value;
}

template <typename K, typename V, typename HashPred>
template <typename Key, typename Val>
V& HashMap<K, V, HashPred>::replace(Key&& key, Val&& val)
{
  Node **ppBucket = getBucket(key);
  V *pVal = getValue(*ppBucket, key);
  if (pVal)
  {
    *pVal = std::forward<Val>(val);
    return *pVal;
  }
  Node *pNode = pool.New(std::forward<Key>(key), std::forward<Val>(val));
  pNode->pNext = *ppBucket;
  *ppBucket = pNode;
  return pNode->data.value;
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::replace(KVP<K, V> &&v)
{
  return replace(std::move(v.key), std::move(v.value));
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::replace(const KVP<K, V> &v)
{
  return replace(v.key, v.value);
}

template <typename K, typename V, typename HashPred>
void HashMap<K, V, HashPred>::remove(const K &key)
{
  Node **ppBucket = getBucket(key);
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

template <typename K, typename V, typename HashPred>
const V* HashMap<K, V, HashPred>::get(const K &key) const
{
  return getValue(*getBucket(key), key);
}
template <typename K, typename V, typename HashPred>
V* HashMap<K, V, HashPred>::get(const K &key)
{
  return getValue(*getBucket(key), key);
}

template <typename K, typename V, typename HashPred>
const V& HashMap<K, V, HashPred>::operator[](const K &key) const
{
  const V *pV = get(key);
  EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
  return *pV;
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::operator[](const K &key)
{
  V *pV = get(key);
  EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
  return *pV;
}

template <typename K, typename V, typename HashPred>
bool HashMap<K, V, HashPred>::exists(const K &key)
{
  return get(key) != nullptr;
}

template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::begin() const -> typename HashMap<K, V, HashPred>::Iterator
{
  return Iterator(ppTable, ppTable + tableSizeMask + 1);
}
template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::end() const -> typename HashMap<K, V, HashPred>::Iterator
{
  return Iterator(ppTable + tableSizeMask + 1, ppTable + tableSizeMask + 1);
}

template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::find(const K &key) -> typename HashMap<K, V, HashPred>::Iterator
{
  Node **ppBucket = getBucket(key);
  Node *pItem = *ppBucket;
  while (pItem)
  {
    if (HashPred::eq(pItem->data.key, key))
    {
      Iterator it(ppBucket, ppTable + tableSizeMask + 1);
      it.pItem = pItem;
      return it;
    }
    pItem = pItem->pNext;
  }
  return end();
}

template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::erase(Iterator it) -> typename HashMap<K, V, HashPred>::Iterator
{
  EPASSERT_THROW(it.pItem, Result::InvalidArgument, "Attempting to erase null iterator");
  if (*it.ppStart == it.pItem)
    *it.ppStart = it.pItem->pNext;
  else
  {
    Node **ppPrev = it.ppStart;
    while (*ppPrev && (*ppPrev)->pNext != it.pItem)
      ppPrev = &(*ppPrev)->pNext;
    EPASSERT_THROW(*ppPrev, Result::InvalidArgument, "Attempting to erase with invalid iterator");
    (*ppPrev)->pNext = it.pItem->pNext;
  }

  Node *pItem = it.pItem;
  pool.Delete(pItem);
  return ++it;
}

template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::getBucket(const K &key) const -> typename HashMap<K, V, HashPred>::Node**
{
  auto h = HashPred::hash(key);
  size_t bucket = h & tableSizeMask;
  return &ppTable[bucket];
}

template <typename K, typename V, typename HashPred>
V* HashMap<K, V, HashPred>::getValue(Node *pBucket, const K &key) const
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

template <typename K, typename V, typename HashPred>
HashMap<K, V, HashPred>::Iterator::Iterator(Node **ppStart, Node **ppEnd)
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

template <typename K, typename V, typename HashPred>
auto HashMap<K, V, HashPred>::Iterator::operator++() -> typename HashMap<K, V, HashPred>::Iterator&
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

template <typename K, typename V, typename HashPred>
bool HashMap<K, V, HashPred>::Iterator::operator!=(Iterator rhs) const
{
  return pItem != rhs.pItem;
}

template <typename K, typename V, typename HashPred>
bool HashMap<K, V, HashPred>::Iterator::operator==(Iterator rhs) const
{
  return pItem == rhs.pItem;
}

template <typename K, typename V, typename HashPred>
const K& HashMap<K, V, HashPred>::Iterator::key() const
{
  return pItem->data.key;
}
template <typename K, typename V, typename HashPred>
const V& HashMap<K, V, HashPred>::Iterator::value() const
{
  return pItem->data.value;
}
template <typename K, typename V, typename HashPred>
V& HashMap<K, V, HashPred>::Iterator::value()
{
  return pItem->data.value;
}

template <typename K, typename V, typename HashPred>
KVPRef<const K, const V> HashMap<K, V, HashPred>::Iterator::operator*() const
{
  return KVPRef<const K, const V>(pItem->data.key, pItem->data.value);
}
template <typename K, typename V, typename HashPred>
KVPRef<const K, V> HashMap<K, V, HashPred>::Iterator::operator*()
{
  return KVPRef<const K, V>(pItem->data.key, pItem->data.value);
}

template <typename K, typename V, typename HashPred>
const V* HashMap<K, V, HashPred>::Iterator::operator->() const
{
  return &pItem->data.value;
}

template <typename K, typename V, typename HashPred>
V* HashMap<K, V, HashPred>::Iterator::operator->()
{
  return &pItem->data.value;
}

} // namespace ep
