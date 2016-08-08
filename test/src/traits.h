#pragma once
#if !defined(_EP_TEST_TRAITS_H)
#define _EP_TEST_TRAITS_H

#include "ep/cpp/traits.h"
#include "ep/cpp/string.h"
#include "ep/cpp/keyvaluepair.h"


// ------------------------------- Helpers ----------------------------------------------------

#define DEFINE_TEST_CONTAINER(Container)                                \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = Container<int>;                                 \
    using HoldsInt = Container<int>;                                    \
    using HoldsFloat = Container<float>;                                \
    using HoldsDouble = Container<double>;                              \
    using HoldsBool = Container<bool>;                                  \
    using HoldsChar = Container<char>;                                  \
    using HoldsString = Container<ep::SharedString>;                    \
  };                                                                    \
  }

#define DEFINE_TEST_CONTAINER_KEYED(Container, KeyType)                 \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = Container<KeyType, int>;                        \
    using HoldsInt = Container<KeyType, int>;                           \
    using HoldsFloat = Container<KeyType, float>;                       \
    using HoldsDouble = Container<KeyType, double>;                     \
    using HoldsBool = Container<KeyType, bool>;                         \
    using HoldsChar = Container<KeyType, char>;                         \
    using HoldsString = Container<KeyType, ep::SharedString>;           \
  };                                                                    \
  }

#define DEFINE_TEST_CONTAINER_TREE(Container, TreeType, KeyType)        \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = Container<TreeType<KeyType, int>>;              \
    using HoldsInt = Container<TreeType<KeyType, int>>;                 \
    using HoldsFloat = Container<TreeType<KeyType, float>>;             \
    using HoldsDouble = Container<TreeType<KeyType, double>>;           \
    using HoldsBool = Container<TreeType<KeyType, bool>>;               \
    using HoldsChar = Container<TreeType<KeyType, char>>;               \
    using HoldsString = Container<TreeType<KeyType, ep::SharedString>>; \
  };                                                                    \
  }


namespace testTraits {

// Helper to initialise a plain array of elements (that matches the element type of container/range T)
// and then create a new container/range for that type
template <typename T, size_t Size>
struct ValueArray
{
public:
  template <typename Type = T, typename std::enable_if<ep::IsKeyed<Type>::value>::type* = nullptr>
  T create()
  {
    typename T::KeyValuePair pairs[Size];
    for (size_t i = 0; i < Size; ++i)
      pairs[i] = typename T::KeyValuePair(ep::SharedString::format("test_{0}", values[i]), values[i]);

    return T(ep::Slice<const typename T::KeyValuePair>(pairs));
  }
  template <typename Type = T, typename std::enable_if<!ep::IsKeyed<Type>::value>::type* = nullptr>
  T create()
  {
    return T(values, Size);
  }

  ep::ElementType<T> values[Size];
};


// Value ...
template <typename Container, typename Value, typename std::enable_if<ep::IsKeyed<Container>::value>::type* = nullptr>
auto value(Value &&val) -> typename Container::KeyValuePair
{
  return typename Container::KeyValuePair(ep::SharedString::format("test_{0}", val), val);
}

template <typename Container, typename Value, typename std::enable_if<!ep::IsKeyed<Container>::value>::type* = nullptr>
Value value(Value &&val)
{
  return val;
}


// Key ...
template <typename Container, typename Value, typename std::enable_if<ep::IsKeyed<Container>::value>::type* = nullptr>
auto key(Value &&val) -> typename Container::KeyType
{
  return typename Container::KeyType(ep::SharedString::format("test_{0}", val));
}

template <typename Container, typename Value, typename std::enable_if<!ep::IsKeyed<Container>::value>::type* = nullptr>
Value key(Value && epUnusedParam(val))
{
  EPASSERT(sizeof(Container) == 0, "Called testTraits::key(...) on a non keyed container");
}


// Add Item ...
template <typename Container, typename Item, typename std::enable_if<ep::HasBack<Container>::value && ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  container.push_back(std::move(value<Container>(item)));
}

template <typename Container, typename Item, typename std::enable_if<!ep::HasBack<Container>::value && ep::HasFront<Container>::value && ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  container.push_front(std::move(value<Container>(item)));
}

template <typename Container, typename Item, typename std::enable_if<!ep::HasBack<Container>::value && !ep::HasFront<Container>::value && ep::RandomAccessible<Container>::value && ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container &container, Item &&item)
{
  container.insert(std::move(value<Container>(item)));
}

template <typename Container, typename Item, typename std::enable_if<!ep::Growable<Container>::value>::type* = nullptr>
void addItem(Container & epUnusedParam(container), Item && epUnusedParam(item))
{
  EPASSERT(sizeof(Container) == 0, "Called testTraits::addItem(...) on a non growable container");
}

} // namespace testTraits


#endif  // _EP_TEST_TRAITS_H
