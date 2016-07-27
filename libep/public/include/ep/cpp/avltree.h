#pragma once
#if !defined(_EPAVLTREE_HPP)
#define _EPAVLTREE_HPP

#include "ep/cpp/platform.h"
#include "ep/cpp/keyvaluepair.h"
#include "ep/cpp/range.h"

namespace ep {

namespace internal {
template<typename T> const T& epMax(const T &a, const T &b) { return a > b ? a : b; }
template<typename T> T& epMax(T &a, T &b) { return a > b ? a : b; }
}

template<typename T>
struct Compare
{
  epforceinline ptrdiff_t operator()(const T &a, const T &b)
  {
    return a < b ? -1 : (a > b ? 1 : 0);
  }
};

//! \cond
template<>
struct Compare<String>
{
  epforceinline ptrdiff_t operator()(String a, String b)
  {
    return a.cmp(b);
  }
};
template<>
struct Compare<SharedString>
{
  epforceinline ptrdiff_t operator()(String a, String b)
  {
    return a.cmp(b);
  }
};
//! \endcond

template <typename K, typename V>
struct AVLTreeNode;

template <typename Node>
struct AVLTreeAllocator;

template<typename K, typename V, typename PredFunctor = Compare<K>, typename Allocator = AVLTreeAllocator<AVLTreeNode<K, V>>>
struct AVLTree
{
public:
  using KeyType = K;
  using ValueType = V;
  using KeyValuePair = KVP<K, V>;

  AVLTree() {}
  AVLTree(nullptr_t) {}
  AVLTree(const AVLTree &rval)
    : numNodes(rval.numNodes), pRoot(clone(rval.pRoot))
  {
  }

  AVLTree(AVLTree &&rval)
    : numNodes(rval.numNodes), pRoot(rval.pRoot)
  {
    rval.pRoot = nullptr;
  }

  AVLTree(Slice<const KeyValuePair> arr)
  {
    for (auto &kvp : arr)
      insert(kvp.key, kvp.value);
  }
  AVLTree(std::initializer_list<KeyValuePair> init)
    : AVLTree(Slice<const KeyValuePair>(init.begin(), init.size())) {}

  ~AVLTree()
  {
    clear();
  }

  size_t size() const { return numNodes; }
  bool empty() const { return numNodes == 0; }

  void clear()
  {
    destroy(pRoot);
    pRoot = nullptr;
  }

  V& insert(K &&key, V &&val)
  {
    if (get(key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(std::move(key), std::move(val));
  }
  V& insert(const K &key, V &&val)
  {
    if (get(key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(key, std::move(val));
  }
  V& insert(K &&key, const V &val)
  {
    if (get(key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(std::move(key), val);
  }
  V& insert(const K &key, const V &val)
  {
    if (get(key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(key, val);
  }

  V& insert(KVP<K, V> &&kvp)
  {
    if (get(kvp.key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(std::move(kvp));
  }
  V& insert(const KVP<K, V> &kvp)
  {
    if (get(kvp.key))
      EPTHROW_ERROR(Result::AlreadyExists, "Key already exists");
    return replace(kvp);
  }

  template <typename Key>
  V& tryInsert(Key&& key, V&& val)
  {
    V *v = get(key);
    if (v)
      return *v;
    return replace(std::forward<Key>(key), std::move(val));
  }
  template <typename Key>
  V& tryInsert(Key&& key, const V& val)
  {
    V *v = get(key);
    if (v)
      return *v;
    return replace(std::forward<Key>(key), val);
  }
  template <typename Key>
  V& tryInsert(Key&& key, std::function<V()> lazy)
  {
    V *v = get(key);
    if (v)
      return *v;
    return replace(std::forward<Key>(key), lazy());
  }

  V& replace(K &&key, V &&val)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(std::move(key), std::move(val));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }
  V& replace(const K &key, V &&val)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(key, std::move(val));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }
  V& replace(K &&key, const V &val)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(std::move(key), val);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }
  V& replace(const K &key, const V &val)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(key, val);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }

  V& replace(KVP<K, V> &&kvp)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(std::move(kvp));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }
  V& replace(const KVP<K, V> &kvp)
  {
    Node *node = Allocator::get()._alloc();
    epscope(fail) { Allocator::get()._free(node); };
    epConstruct(&node->kvp) KVP<K, V>(kvp);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->kvp.value;
  }

  void remove(const K &key)
  {
    pRoot = deleteNode(pRoot, key);
  }

  const V* get(const K &key) const
  {
    const Node *n = find(pRoot, key);
    return n ? &n->kvp.value : nullptr;
  }
  V* get(const K &key)
  {
    Node *n = const_cast<Node*>(find(pRoot, key));
    return n ? &n->kvp.value : nullptr;
  }

  const V& operator[](const K &key) const
  {
    const V *pV = get(key);
    EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }
  V& operator[](const K &key)
  {
    V *pV = get(key);
    EPASSERT_THROW(pV, Result::OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }

  AVLTree<K, V>& operator =(const AVLTree<K, V> &rh)
  {
    if (this != &rh)
    {
      this->~AVLTree();
      epConstruct(this) AVLTree<K, V>(rh);
    }
    return *this;
  }

  AVLTree<K, V>& operator =(AVLTree<K, V> &&rval)
  {
    if (this != &rval)
    {
      this->~AVLTree();
      epConstruct(this) AVLTree<K, V>(std::move(rval));
    }
    return *this;
  }

  class Iterator;
  Iterator begin() const { return Iterator(pRoot); }
  static Iterator end() { return Iterator(nullptr); }

private:
  using Node = AVLTreeNode<K, V>;

  size_t numNodes = 0;
  Node *pRoot = nullptr;

  static int height(const Node *n)
  {
    return n ? n->height : 0;
  }
  static int maxHeight(const Node *n)
  {
    if (!n)
      return 0;
    if (n->left)
    {
      if (n->right)
        return internal::epMax(n->left->height, n->right->height);
      else
        return n->left->height;
    }
    if (n->right)
      return n->right->height;
    return 0;
  }
  static int getBalance(Node *n)
  {
    return n ? height(n->left) - height(n->right) : 0;
  }

  static Node *rightRotate(Node *y)
  {
    Node *x = y->left;
    Node *T2 = x->right;

    // Perform rotation
    x->right = y;
    y->left = T2;

    // Update heights
    y->height = maxHeight(y) + 1;
    x->height = maxHeight(x) + 1;

    // Return new root
    return x;
  }
  static Node *leftRotate(Node *x)
  {
    Node *y = x->right;
    Node *T2 = y->left;

    // Perform rotation
    y->left = x;
    x->right = T2;

    //  Update heights
    x->height = maxHeight(x) + 1;
    y->height = maxHeight(y) + 1;

    // Return new root
    return y;
  }

  static const Node* find(const Node *n, const K &key)
  {
    if (!n)
      return nullptr;
    ptrdiff_t c = PredFunctor()(key, n->kvp.key);
    if (c < 0)
      return find(n->left, key);
    if (c > 0)
      return find(n->right, key);
    return n;
  }

  void destroy(Node *n)
  {
    if (!n)
      return;

    destroy(n->left);
    destroy(n->right);

    n->~Node();
    Allocator::get()._free(n);
  }

  Node* insert(Node *n, Node *newnode)
  {
    // 1.  Perform the normal BST rotation
    if (n == nullptr)
    {
      ++numNodes;
      return newnode;
    }

    ptrdiff_t c = PredFunctor()(newnode->kvp.key, n->kvp.key);
    if (c < 0)
      n->left = insert(n->left, newnode);
    else if (c > 0)
      n->right = insert(n->right, newnode);
    else
    {
      newnode->left = n->left;
      newnode->right = n->right;
      newnode->height = n->height;

      n->~Node();
      Allocator::get()._free(n);

      return newnode;
    }

    // 2. Update height of this ancestor Node
    n->height = maxHeight(n) + 1;

    // 3. get the balance factor of this ancestor Node to check whether
    //    this Node became unbalanced
    int balance = getBalance(n);

    // If this Node becomes unbalanced, then there are 4 cases

    if (balance > 1)
    {
      ptrdiff_t lc = PredFunctor()(newnode->kvp.key, n->left->kvp.key);
      // Left Left Case
      if (lc < 0)
        return rightRotate(n);

      // Left Right Case
      if (lc > 0)
      {
        n->left = leftRotate(n->left);
        return rightRotate(n);
      }
    }

    if (balance < -1)
    {
      ptrdiff_t rc = PredFunctor()(newnode->kvp.key, n->right->kvp.key);

      // Right Right Case
      if (rc > 0)
        return leftRotate(n);

      // Right Left Case
      if (rc < 0)
      {
        n->right = rightRotate(n->right);
        return leftRotate(n);
      }
    }

    // return the (unchanged) Node pointer
    return n;
  }

  static Node* minValueNode(Node *n)
  {
    Node *current = n;

    // loop down to find the leftmost leaf
    while (current->left != nullptr)
      current = current->left;

    return current;
  }

  Node* deleteNode(Node *_pRoot, const K &key)
  {
    // STEP 1: PERFORM STANDARD BST DELETE

    if (_pRoot == nullptr)
      return _pRoot;

    ptrdiff_t c = PredFunctor()(key, _pRoot->kvp.key);

    // If the key to be deleted is smaller than the _pRoot's key,
    // then it lies in left subtree
    if (c < 0)
      _pRoot->left = deleteNode(_pRoot->left, key);

    // If the key to be deleted is greater than the _pRoot's key,
    // then it lies in right subtree
    else if (c > 0)
      _pRoot->right = deleteNode(_pRoot->right, key);

    // if key is same as _pRoot's key, then this is the Node
    // to be deleted
    else
    {
      // Node with only one child or no child
      if ((_pRoot->left == nullptr) || (_pRoot->right == nullptr))
      {
        Node *temp = _pRoot->left ? _pRoot->left : _pRoot->right;

        // No child case
        if (temp == nullptr)
        {
          temp = _pRoot;
          _pRoot = nullptr;
        }
        else // One child case
        {
          // TODO: FIX THIS!!
          // this is copying the child node into the parent node because there is no parent pointer
          // DO: add parent pointer, then fix up the parent's child pointer to the child, and do away with this pointless copy!
          *_pRoot = std::move(*temp); // Copy the contents of the non-empty child
        }

        temp->~Node();
        Allocator::get()._free(temp);

        --numNodes;
      }
      else
      {
        // Node with two children: get the inorder successor (smallest
        // in the right subtree)
        Node *temp = minValueNode(_pRoot->right);

        // Copy the inorder successor's data to this Node
        _pRoot->kvp.key = temp->kvp.key;

        // Delete the inorder successor
        _pRoot->right = deleteNode(_pRoot->right, temp->kvp.key);
      }
    }

    // If the tree had only one Node then return
    if (_pRoot == nullptr)
      return _pRoot;

    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
    _pRoot->height = internal::epMax(height(_pRoot->left), height(_pRoot->right)) + 1;

    // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to check whether
    //  this Node became unbalanced)
    int balance = getBalance(_pRoot);

    // If this Node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && getBalance(_pRoot->left) >= 0)
      return rightRotate(_pRoot);

    // Left Right Case
    if (balance > 1 && getBalance(_pRoot->left) < 0)
    {
      _pRoot->left = leftRotate(_pRoot->left);
      return rightRotate(_pRoot);
    }

    // Right Right Case
    if (balance < -1 && getBalance(_pRoot->right) <= 0)
      return leftRotate(_pRoot);

    // Right Left Case
    if (balance < -1 && getBalance(_pRoot->right) > 0)
    {
      _pRoot->right = rightRotate(_pRoot->right);
      return leftRotate(_pRoot);
    }

    return _pRoot;
  }

  static Node* clone(Node *pOld)
  {
    if (!pOld)
      return nullptr;

    Node *pNew = Allocator::get()._alloc();
    new(&pNew->kvp) KeyValuePair(pOld->kvp);
    pNew->height = pOld->height;
    pNew->left = clone(pOld->left);
    pNew->right = clone(pOld->right);
    return pNew;
  }

public:
  class Iterator
  {
  public:
    Iterator(Node *pRoot) : depth(0), stack(0), pRoot(pRoot)
    {
      Node *pLeftMost = pRoot;
      while (pLeftMost && pLeftMost->left)
      {
        stack |= 1LL << depth;
        ++depth;
        pLeftMost = pLeftMost->left;
      }
    }

    Iterator(const Iterator &rh) : data(rh.data), pRoot(rh.pRoot) {}
    Iterator(Iterator &&rval) : data(rval.data), pRoot(rval.pRoot)
    {
      rval.data = 0;
      rval.pRoot = nullptr;
    }

    Iterator &operator++()
    {
      iterateNext(pRoot, nullptr, 0);
      return *this;
    }

    bool operator!=(Iterator rhs) { return pRoot != rhs.pRoot || data != rhs.data; }

    const K& key() const
    {
      auto *node = getNode(stack, depth);
      return node->kvp.key;
    }
    const V& value() const
    {
      auto *node = getNode(stack, depth);
      return node->kvp.value;
    }
    V& value()
    {
      auto *node = const_cast<Node*>(getNode(stack, depth));
      return node->kvp.value;
    }

    KVPRef<const K, const V> operator*() const
    {
      return KVPRef<const K, const V>(key(), value());
    }
    KVPRef<const K, V> operator*()
    {
      return KVPRef<const K, V>(key(), value());
    }
    const V* operator->() const
    {
      return &value();
    }
    V* operator->()
    {
      return &value();
    }

    const Node *getNode(uint64_t s, uint64_t d) const
    {
      const Node *pNode = pRoot;
      for (uint64_t i = 0; i < d; ++i)
      {
        if (s & (1LL << i))
          pNode = pNode->left;
        else
          pNode = pNode->right;
      }
      return pNode;
    }

  private:
    bool iterateNext(Node *pNode, Node *pParent, uint64_t d)
    {
      if (d < depth)
      {
        Node *pNext = (stack & (1LL << d)) ? pNode->left : pNode->right;
        if (!iterateNext(pNext, pNode, d + 1))
          return false;
      }
      else
      {
        if (pNode->right) // Left Most
        {
          ++depth;
          const Node *pLeftMost = pNode->right;
          while (pLeftMost->left)
          {
            stack |= 1LL << depth;
            ++depth;
            pLeftMost = pLeftMost->left;
          }
          return false;
        }
      }

      if (depth == 0)
      {
        pRoot = 0;
        data = 0;
        return false;
      }

      --depth;
      stack &= ~(1 << depth);
      if (pParent->right == pNode)
        return true;

      return false;
    }

    union
    {
      struct
      {
        uint64_t depth : 8;
        uint64_t stack : 56;
      };
      uint64_t data;
    };
    Node *pRoot;
  };
};


template <typename K, typename V>
struct AVLTreeNode
{
  AVLTreeNode *left, *right;
  KVP<K, V> kvp;
  int height;

  AVLTreeNode() = delete;
  AVLTreeNode(const AVLTreeNode &rh)
    : left(rh.left), right(rh.right), kvp(rh.kvp), height(rh.height) {}
  AVLTreeNode(AVLTreeNode &&rh)
    : left(rh.left), right(rh.right), kvp(std::move(rh.kvp)), height(rh.height) {}
  AVLTreeNode& operator = (const AVLTreeNode &rh)
  {
    this->~AVLTreeNode();
    new(this) AVLTreeNode(rh);
    return *this;
  }
  AVLTreeNode& operator = (AVLTreeNode &&rh)
  {
    this->~AVLTreeNode();
    new(this) AVLTreeNode(std::move(rh));
    return *this;
  }
};

template <typename Node>
struct AVLTreeAllocator
{
  Node *_alloc()
  {
    void *pMem = epAlloc(sizeof(Node));
    EPTHROW_IF_NULL(pMem, Result::AllocFailure, "AVLTreeAllocator failed");
    return (Node*)pMem;
  }

  void _free(Node *pMem)
  {
    epFree(pMem);
  }

  static AVLTreeAllocator& get()
  {
    static AVLTreeAllocator<Node> allocator;
    return allocator;
  }
  // TODO: Add API for memory usage statistics
};


template<typename K, typename V, typename PredFunctor, typename Allocator>
ptrdiff_t epStringify(Slice<char> buffer, String epUnusedParam(format), const AVLTree<K, V, PredFunctor, Allocator> &tree, const VarArg *epUnusedParam(pArgs))
{
  size_t offset = 0;
  if (buffer)
    offset += String("{ ").copyTo(buffer);
  else
    offset += String("{ ").length;

  bool bFirst = true;
  for (auto &&kvp : tree)
  {
    if (!bFirst)
    {
      if (buffer)
        offset += String(", ").copyTo(buffer.strip(offset));
      else
        offset += String(", ").length;
    }
    else
      bFirst = false;

    if (buffer)
      offset += epStringify(buffer.strip(offset), nullptr, kvp, nullptr);
    else
      offset += epStringify(nullptr, nullptr, kvp, nullptr);
  }

  if (buffer)
    offset += String(" }").copyTo(buffer.strip(offset));
  else
    offset += String(" }").length;

  return offset;
}

} // namespace ep

#endif // _EPAVLTREE_HPP
