#pragma once
#if !defined(_EPAVLTREE_HPP)
#define _EPAVLTREE_HPP

#include "ep/cpp/platform.h"

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

template<typename K, typename V, typename PredFunctor = Compare<K>>
class AVLTree
{
public:
  using KeyType = K;
  using ValueType = V;
  using KeyValuePair = KVP<K, V>;

  AVLTree() {}
  AVLTree(nullptr_t) {}
  AVLTree(AVLTree &&rval)
    : size(size), root(root)
  {
    rval.root = nullptr;
  }

  ~AVLTree()
  {
    Destroy(root);
    root = nullptr;
  }

  size_t Size() const { return size; }
  bool Empty() const { return size == 0; }

  void Insert(K &&key, V &&rval)
  {
    Node* node = (Node*)epAlloc(sizeof(Node));
    new(&node->k) K(std::move(key));
    new(&node->v) V(std::move(rval));
    node->left = node->right = nullptr;
    node->height = 1;
    root = insert(root, node);
  }
  void Insert(const K &key, V &&rval)
  {
    Node* node = (Node*)epAlloc(sizeof(Node));
    new(&node->k) K(key);
    new(&node->v) V(std::move(rval));
    node->left = node->right = nullptr;
    node->height = 1;
    root = insert(root, node);
  }
  void Insert(K &&key, const V &v)
  {
    Node* node = (Node*)epAlloc(sizeof(Node));
    new(&node->k) K(std::move(key));
    new(&node->v) V(v);
    node->left = node->right = nullptr;
    node->height = 1;
    root = insert(root, node);
  }
  void Insert(const K &key, const V &v)
  {
    Node* node = (Node*)epAlloc(sizeof(Node));
    new(&node->k) K(key);
    new(&node->v) V(v);
    node->left = node->right = nullptr;
    node->height = 1;
    root = insert(root, node);
  }

  void Remove(const K &key)
  {
    root = deleteNode(root, key);
  }

  const V* Get(const K &key) const
  {
    const Node *n = find(root, key);
    return n ? &n->v : nullptr;
  }
  V* Get(const K &key)
  {
    Node *n = const_cast<Node*>(find(root, key));
    return n ? &n->v : nullptr;
  }

  class Iterator;
  Iterator begin() const { return Iterator(root); }
  static Iterator end() { return Iterator(nullptr); }

private:
  struct Node
  {
    V v;
    K k;
    Node *left, *right;
    int height;

    Node() = delete;
  };

  size_t size = 0;
  Node *root = nullptr;

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

  void Destroy(Node *n)
  {
    if (!n)
      return;

    Destroy(n->left);
    Destroy(n->right);

    n->~Node();
    epFree(n);
  }

  Node* insert(Node *n, Node *newnode)
  {
    // 1.  Perform the normal BST rotation
    if (n == nullptr)
    {
      ++size;
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
      epFree(n);

      return newnode;
    }

    // 2. Update height of this ancestor Node
    n->height = maxHeight(n) + 1;

    // 3. Get the balance factor of this ancestor Node to check whether
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

  Node* deleteNode(Node *pRoot, const K &key)
  {
    // STEP 1: PERFORM STANDARD BST DELETE

    if (pRoot == nullptr)
      return pRoot;

    ptrdiff_t c = PredFunctor()(key, pRoot->k);

    // If the key to be deleted is smaller than the pRoot's key,
    // then it lies in left subtree
    if (c < 0)
      pRoot->left = deleteNode(pRoot->left, key);

    // If the key to be deleted is greater than the pRoot's key,
    // then it lies in right subtree
    else if (c > 0)
      pRoot->right = deleteNode(pRoot->right, key);

    // if key is same as pRoot's key, then this is the Node
    // to be deleted
    else
    {
      // Node with only one child or no child
      if ((pRoot->left == nullptr) || (pRoot->right == nullptr))
      {
        Node *temp = pRoot->left ? pRoot->left : pRoot->right;

        // No child case
        if (temp == nullptr)
        {
          temp = pRoot;
          pRoot = nullptr;
        }
        else // One child case
        {
          // TODO: FIX THIS!!
          // this is copying the child node into the parent node because there is no parent pointer
          // DO: add parent pointer, then fix up the parent's child pointer to the child, and do away with this pointless copy!
          *pRoot = *temp; // Copy the contents of the non-empty child
        }

        temp->~Node();
        epFree(temp);

        --size;
      }
      else
      {
        // Node with two children: Get the inorder successor (smallest
        // in the right subtree)
        struct Node *temp = minValueNode(pRoot->right);

        // Copy the inorder successor's data to this Node
        pRoot->k = temp->k;

        // Delete the inorder successor
        pRoot->right = deleteNode(pRoot->right, temp->k);
      }
    }

    // If the tree had only one Node then return
    if (pRoot == nullptr)
      return pRoot;

    // STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
    pRoot->height = internal::epMax(height(pRoot->left), height(pRoot->right)) + 1;

    // STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to check whether
    //  this Node became unbalanced)
    int balance = getBalance(pRoot);

    // If this Node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && getBalance(pRoot->left) >= 0)
      return rightRotate(pRoot);

    // Left Right Case
    if (balance > 1 && getBalance(pRoot->left) < 0)
    {
      pRoot->left = leftRotate(pRoot->left);
      return rightRotate(pRoot);
    }

    // Right Right Case
    if (balance < -1 && getBalance(pRoot->right) <= 0)
      return leftRotate(pRoot);

    // Right Left Case
    if (balance < -1 && getBalance(pRoot->right) > 0)
    {
      pRoot->right = rightRotate(pRoot->right);
      return leftRotate(pRoot);
    }

    return pRoot;
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

    Iterator &operator++()
    {
      IterateNext(pRoot, nullptr, 0);
      return *this;
    }

    bool operator!=(Iterator rhs) { return pRoot != rhs.pRoot || data != rhs.data; }

    const KVP operator*() const
    {
      auto *node = const_cast<Node*>(GetNode(stack, depth));
      const KVP r = KVP(node->k, node->v);
      return r;
    }
    KVP operator*()
    {
      auto *node = const_cast<Node*>(GetNode(stack, depth));
      return KVP(node->k, node->v);
    }

    const Node *GetNode(uint64_t s, uint64_t d) const
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
    bool IterateNext(Node *pNode, Node *pParent, uint64_t d)
    {
      if (d < depth)
      {
        Node *pNext = (stack & (1LL << d)) ? pNode->left : pNode->right;
        if (!IterateNext(pNext, pNode, d + 1))
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

} // namespace ep

#endif // _EPAVLTREE_HPP
