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

/// \cond
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
/// \endcond

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
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
    return replace(std::move(key), std::move(val));
  }
  V& insert(const K &key, V &&val)
  {
    if (get(key))
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
    return replace(key, std::move(val));
  }
  V& insert(K &&key, const V &val)
  {
    if (get(key))
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
    return replace(std::move(key), val);
  }
  V& insert(const K &key, const V &val)
  {
    if (get(key))
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
    return replace(key, val);
  }

  V& insert(KVP<K, V> &&kvp)
  {
    if (get(kvp.key))
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
    return replace(std::move(kvp));
  }
  V& insert(const KVP<K, V> &kvp)
  {
    if (get(kvp.key))
      EPTHROW_ERROR(epR_AlreadyExists, "Key already exists");
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
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(std::move(key));
    epConstruct(&node->v) V(std::move(val));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }
  V& replace(const K &key, V &&val)
  {
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(key);
    epConstruct(&node->v) V(std::move(val));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }
  V& replace(K &&key, const V &val)
  {
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(std::move(key));
    epConstruct(&node->v) V(val);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }
  V& replace(const K &key, const V &val)
  {
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(key);
    epConstruct(&node->v) V(val);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }

  V& replace(KVP<K, V> &&kvp)
  {
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(std::move(kvp.key));
    epConstruct(&node->v) V(std::move(kvp.value));
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }
  V& replace(const KVP<K, V> &kvp)
  {
    Node *node = Allocator::Get().Alloc();
    epConstruct(&node->k) K(kvp.key);
    epConstruct(&node->v) V(kvp.value);
    node->left = node->right = nullptr;
    node->height = 1;
    pRoot = insert(pRoot, node);
    return node->v;
  }

  void remove(const K &key)
  {
    pRoot = deleteNode(pRoot, key);
  }

  const V* get(const K &key) const
  {
    const Node *n = find(pRoot, key);
    return n ? &n->v : nullptr;
  }
  V* get(const K &key)
  {
    Node *n = const_cast<Node*>(find(pRoot, key));
    return n ? &n->v : nullptr;
  }

  const V& operator[](const K &key) const
  {
    const V *pV = get(key);
    EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
    return *pV;
  }
  V& operator[](const K &key)
  {
    V *pV = get(key);
    EPASSERT_THROW(pV, epR_OutOfBounds, "Element not found: {0}", key);
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
    ptrdiff_t c = PredFunctor()(key, n->k);
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
    Allocator::Get().Free(n);
  }

  Node* insert(Node *n, Node *newnode)
  {
    // 1.  Perform the normal BST rotation
    if (n == nullptr)
    {
      ++numNodes;
      return newnode;
    }

    ptrdiff_t c = PredFunctor()(newnode->k, n->k);
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
      Allocator::Get().Free(n);

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
      ptrdiff_t lc = PredFunctor()(newnode->k, n->left->k);
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
      ptrdiff_t rc = PredFunctor()(newnode->k, n->right->k);

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

    ptrdiff_t c = PredFunctor()(key, _pRoot->k);

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
        Allocator::Get().Free(temp);

        --numNodes;
      }
      else
      {
        // Node with two children: get the inorder successor (smallest
        // in the right subtree)
        Node *temp = minValueNode(_pRoot->right);

        // Copy the inorder successor's data to this Node
        _pRoot->k = temp->k;

        // Delete the inorder successor
        _pRoot->right = deleteNode(_pRoot->right, temp->k);
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

public:
  class Iterator
  {
  public:
    using KVP = KVPRef<K, V>;

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

    const KVP operator*() const
    {
      auto *node = const_cast<Node*>(getNode(stack, depth));
      const KVP r = KVP(node->k, node->v);
      return r;
    }
    KVP operator*()
    {
      auto *node = const_cast<Node*>(getNode(stack, depth));
      return KVP(node->k, node->v);
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
  K k;
  AVLTreeNode *left, *right;
  V v;
  int height;

  AVLTreeNode() = delete;
  AVLTreeNode(const AVLTreeNode &rh)
    : k(rh.k), left(rh.left), right(rh.right), v(rh.v), height(rh.height) {}
  AVLTreeNode(AVLTreeNode &&rh)
    : k(std::move(rh.k)), left(rh.left), right(rh.right), v(std::move(rh.v)), height(rh.height) {}
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
  Node *Alloc()
  {
    void *pMem = epAlloc(sizeof(Node));
    EPTHROW_IF_NULL(pMem, epR_AllocFailure, "AVLTreeAllocator failed");
    return (Node*)pMem;
  }

  void Free(Node *pMem)
  {
    epFree(pMem);
  }

  static AVLTreeAllocator& Get()
  {
    static AVLTreeAllocator<Node> allocator;
    return allocator;
  }
  // TODO: Add API for memory usage statistics
};

} // namespace ep

#endif // _EPAVLTREE_HPP
