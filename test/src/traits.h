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
    using DefaultType = typename Container<int>;                        \
    using HoldsInt = typename Container<int>;                           \
    using HoldsFloat = typename Container<float>;                       \
    using HoldsDouble = typename Container<double>;                     \
    using HoldsBool = typename Container<bool>;                         \
    using HoldsChar = typename Container<char>;                         \
    using HoldsString = typename Container<ep::SharedString>;           \
  };                                                                    \
  }

#define DEFINE_TEST_CONTAINER_KEYED(Container, KeyType)                 \
  namespace testTraits {                                                \
  struct Types {                                                        \
    using DefaultType = typename Container<KeyType, int>;               \
    using HoldsInt = typename Container<KeyType, int>;                  \
    using HoldsFloat = typename Container<KeyType, float>;              \
    using HoldsDouble = typename Container<KeyType, double>;            \
    using HoldsBool = typename Container<KeyType, bool>;                \
    using HoldsChar = typename Container<KeyType, char>;                \
    using HoldsString = typename Container<KeyType, ep::SharedString>;  \
  };                                                                    \
  }

#define DEFINE_TEST_CONTAINER_TREE(Container, TreeType, KeyType)                 \
  namespace testTraits {                                                         \
  struct Types {                                                                 \
    using DefaultType = typename Container<TreeType<KeyType, int>>;              \
    using HoldsInt = typename Container<TreeType<KeyType, int>>;                 \
    using HoldsFloat = typename Container<TreeType<KeyType, float>>;             \
    using HoldsDouble = typename Container<TreeType<KeyType, double>>;           \
    using HoldsBool = typename Container<TreeType<KeyType, bool>>;               \
    using HoldsChar = typename Container<TreeType<KeyType, char>>;               \
    using HoldsString = typename Container<TreeType<KeyType, ep::SharedString>>; \
  };                                                                             \
  }


namespace testTraits {

// Value ...
template <typename Container, typename Value, typename std::enable_if<ep::IsKeyed<Container>::value>::type* = nullptr>
auto value(Value &&val)
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
auto key(Value &&val)
{
  return typename Container::KeyType(ep::SharedString::format("test_{0}", val));
}

template <typename Container, typename Value, typename std::enable_if<!ep::IsKeyed<Container>::value>::type* = nullptr>
Value key(Value &&val)
{
  static_assert(sizeof(Container) != 0, "Called testTraits::key(...) on a non keyed container");
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
void addItem(Container &container, Item &&item)
{
  static_assert(sizeof(Container) != 0, "Called testTraits::addItem(...) on a non growable container");
}

} // namespace traits


#endif  // _EP_TEST_TRAITS_H
