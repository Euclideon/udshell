namespace ep {

template <typename T>
HashMap<T>::HashMap(size_t _tableSize)
  : itemPool(1024) // TODO: this is not enough, FreeList needs to be enhanced to grow automatically
{
#if !defined(SUPPORT_FLEXIBLE_TABLE_SIZE)
  _tableSize = 256;
#endif
  ppItems = (Item**)epAllocFlags(sizeof(Item*)*_tableSize, epAF_Zero);
  tableSize = _tableSize;
  itemCount = 0;
}

template <typename T>
HashMap<T>::~HashMap()
{
  epFree(ppItems);
  tableSize = 0;
  itemCount = 0;
}

template <typename T>
inline T* HashMap<T>::Get(size_t hash) const
{
  size_t i = GetTableIndex(hash);
  Item *pItem = ppItems[i];

  while (pItem && pItem->hash != hash)
    pItem = pItem->pNext;

  return pItem ? &pItem->item : nullptr;
}

template <typename T>
inline T& HashMap<T>::Create(size_t hash)
{
  Item *pNew = (Item*)itemPool.Alloc();
  new(&pNew->item) T;

  pNew->hash = hash;

  size_t i = GetTableIndex(hash);
  pNew->pNext = ppItems[i];
  ppItems[i] = pNew;

  ++itemCount;

  return pNew->item;
}

template <typename T>
inline T& HashMap<T>::Add(size_t hash, const T& item)
{
  Item *pNew = itemPool.Alloc();
  new(&pNew->item) T(item);

  pNew->hash = hash;

  size_t i = GetTableIndex(hash);
  pNew->pNext = ppItems[i];
  ppItems[i] = pNew;

  ++itemCount;

  return pNew->item;
}

#if EP_CPP11
template <typename T>
inline T& HashMap<T>::Add(size_t hash, T&& item)
{
  Item *pNew = itemPool.Alloc();
  new(&pNew->item) T(std::move(item));

  pNew->hash = hash;

  size_t i = GetTableIndex(hash);
  pNew->pNext = ppItems[i];
  ppItems[i] = pNew;

  ++itemCount;

  return pNew->item;
}
#endif

template <typename T>
inline void HashMap<T>::Destroy(size_t hash)
{
  size_t i = GetTableIndex(hash);

  Item *pTemp = ppItems[i];
  if (!pTemp)
    return;

  Item *pDel = nullptr;
  if (pTemp->hash == hash)
  {
    pDel = pTemp;
    ppItems[i] = pTemp->pNext;
  }
  else
  {
    while (pTemp->pNext && pTemp->hash != hash)
      pTemp = pTemp->pNext;
    if (pTemp->pNext)
    {
      pDel = pTemp->pNext;
      pTemp->pNext = pTemp->pNext->pNext;
    }
  }

  if (pDel)
  {
    pDel->item.~T();
    itemPool.Free(pDel);
    --itemCount;
  }
}

template <typename T>
inline bool HashMap<T>::DestroyItem(const T& item)
{
  if (itemPool.Owns(&item))
  {
    Item &i = (Item&)item;
    Destroy(i.hash);
    return true;
  }

  return false;
}

template <typename T>
inline bool HashMap<T>::Destroy(const T& item)
{
  if (DestroyItem(item))
    return true;

  Item *pDel = nullptr;
  for (int a = 0; a<tableSize; ++a)
  {
    Item *pTemp = ppItems[a];
    if (!pTemp)
      continue;

    if (pTemp->item == item)
    {
      pDel = pTemp;
      ppItems[a] = pTemp->pNext;
      break;
    }
    else
    {
      while (pTemp->pNext && pTemp->pNext->item != item)
        pTemp = pTemp->pNext;
      if (pTemp->pNext)
      {
        pDel = pTemp->pNext;
        pTemp->pNext = pTemp->pNext->pNext;
        break;
      }
    }
  }

  if (pDel)
  {
    pDel->item.~T();
    itemPool.Free(pDel);
    --itemCount;
    return true;
  }

  return false;
}

template <typename T>
inline typename HashMap<T>::Iterator HashMap<T>::First() const
{
  if (!itemCount)
    return Iterator(*this, nullptr);

  for (size_t a = 0; a<tableSize; ++a)
  {
    if (ppItems[a])
      return Iterator(*this, ppItems[a]);
  }
  return Iterator(*this, nullptr);
}

template <typename T>
inline typename HashMap<T>::Iterator HashMap<T>::end() const
{
  return Iterator(*this, nullptr);
}

template <typename T>
inline typename HashMap<T>::Iterator HashMap<T>::Next(Iterator item) const
{
  Item *pI = item.pItem;
  size_t i = GetTableIndex(pI->hash);
  Item *pTemp = ppItems[i];

  while (pTemp && pTemp != item.pItem)
    pTemp = pTemp->pNext;

  if (!pTemp)
    return Iterator(*this, nullptr);
  else if (pTemp->pNext)
    return Iterator(*this, pTemp->pNext);
  else
  {
    for (size_t a = i+1; a<tableSize; ++a)
    {
      if (ppItems[a])
        return Iterator(*this, ppItems[a]);
    }
  }
  return Iterator(*this, nullptr);
}

template <typename T>
inline typename HashMap<T>::Iterator HashMap<T>::NextMatch(Iterator item) const
{
  if (item.pItem == nullptr)
    return Iterator(*this, nullptr);

  Item *pI = item.pItem;
  size_t i = GetTableIndex(pI->hash);
  Item *pTemp = ppItems[i];

  while (pTemp && &pTemp->item != item.pItem)
    pTemp = pTemp->pNext;

  if (pTemp)
  {
    pTemp = pTemp->pNext;
    while (pTemp)
    {
      if (pTemp->hash == pI->hash)
        return Iterator(*this, pTemp);
      pTemp = pTemp->pNext;
    }
  }

  return Iterator(*this, nullptr);
}

} // namespace ep
