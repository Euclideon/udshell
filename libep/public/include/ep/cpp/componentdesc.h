#if !defined(_EP_COMPONENTDESC_HPP)
#define _EP_COMPONENTDESC_HPP

#include "ep/c/componentdesc.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/event.h"

#include <tuple>

namespace ep {

class Kernel;
struct ComponentDescInl;

typedef Variant(Getter)(const Component *pBaseComponent, const void *pDerivedInstance);
typedef void(Setter)(Component *pBaseComponent, void *pDerivedInstance, const Variant *pValue);
typedef Variant(MethodCall)(const Component *pBaseComponent, const void *pDerivedInstance, const Variant *pArgs, size_t numArgs);
typedef Variant(Subscribe)(const Component *pBaseComponent, const void *pDerivedInstance, VarDelegate *pDelegate);

typedef Variant(StaticCall)(Slice<const Variant> args);

// property description
enum PropertyFlags : uint32_t
{
  epPF_Immutable = 1 << 0 // must be initialised during construction
};

struct PropertyInfo
{
  SharedString id;
  SharedString displayName;
  SharedString description;

  SharedString uiType;
  uint32_t flags;

  VarMethod pGetterMethod;
  VarMethod pSetterMethod;
};
struct MethodInfo
{
  SharedString id;
  SharedString description;

  VarMethod pMethod;
};
struct EventInfo
{
  SharedString id;
  SharedString displayName;
  SharedString description;

  VarMethod pSubscribe;
};
struct StaticFuncInfo
{
  SharedString id;
  SharedString description;

  StaticCall *pCall;
};

EP_BITFIELD(ComponentInfoFlags,
  Unpopulated,
  Unregistered,
  Abstract
);

struct ComponentInfo
{
  int epVersion;
  int pluginVersion;

  SharedString id;          // an id for this component
  SharedString displayName; // display name
  SharedString description; // description

  // icon image...?

  ComponentInfoFlags flags;
};

struct ComponentDesc
{
  virtual ~ComponentDesc() = 0;

  ComponentInfo info;
  const ComponentDesc *pSuperDesc;
};

// We still need to define the destructor even though it's pure virtual
inline ComponentDesc::~ComponentDesc()
{
}

// base class for pImpl types
template <typename C, typename I>
class BaseImpl : public I
{
  friend C;

public:
  using Instance = C;
  using Interface = I;
  using ImplSuper = BaseImpl<C, I>;

  const ComponentDescInl* GetDescriptor() const { return (const ComponentDescInl*)pInstance->GetDescriptor(); }
  Kernel* GetKernel() const { return &pInstance->GetKernel(); }

  C *pInstance;

protected:
  template<typename T, bool b> friend struct internal::Release;

  BaseImpl(const BaseImpl &) = delete;
  void operator=(const BaseImpl &) = delete;

  BaseImpl(Component *pInstance)
    : pInstance((C*)pInstance)
  {}
  virtual ~BaseImpl() {}
};


// helper to get
template<typename T>
struct function_traits;
template<typename C, typename R, typename ...Args>
struct function_traits<R(C::*)(Args...)>
{
  static const size_t num_args = sizeof...(Args);

  using result_type = R;

  using args = std::tuple<Args...>;

  template <size_t i>
  struct arg
  {
    using type = typename std::tuple_element<i, args>::type;
  };
};


// declare magic for a C++ component
#define EP_DECLARE_COMPONENT(Name, SuperType, Version, Description, Flags)               \
public:                                                                                  \
  friend class ::ep::Kernel;                                                             \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = SharedPtr<This>;                                                           \
  using Impl = void;                                                                     \
  static SharedString ComponentID()                                                      \
  {                                                                                      \
    static SharedString id;                                                              \
    if (!id.ptr)                                                                         \
    {                                                                                    \
      char buf[sizeof(#Name)];                                                           \
        for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = (char)epToLower(#Name[i]);     \
      id = SharedString(buf, sizeof(buf)-1);                                             \
    }                                                                                    \
    return id;                                                                           \
  }                                                                                      \
  static ComponentInfo MakeDescriptor()                                                  \
  {                                                                                      \
    return { EP_APIVERSION, Version, ComponentID(), #Name, Description, Flags };         \
  }                                                                                      \
private:

// declare magic for a C++ component with a pImpl interface
#define EP_DECLARE_COMPONENT_WITH_IMPL(Name, Interface, SuperType, Version, Description, Flags) \
  friend class ::ep::Kernel;                                                                    \
  __EP_DECLARE_COMPONENT_IMPL(Name, Interface, SuperType, Version, Description, Flags)

#define __EP_DECLARE_COMPONENT_IMPL(Name, Interface, SuperType, Version, Description, Flags)    \
public:                                                                                  \
  friend class Name##Impl;                                                               \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = SharedPtr<This>;                                                           \
  using Impl = BaseImpl<Name, Interface>;                                                \
  static SharedString ComponentID()                                                      \
  {                                                                                      \
    static SharedString id;                                                              \
    if (!id.ptr)                                                                         \
    {                                                                                    \
      char buf[sizeof(#Name)];                                                           \
        for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = (char)epToLower(#Name[i]);     \
      id = SharedString(buf, sizeof(buf)-1);                                             \
    }                                                                                    \
    return id;                                                                           \
  }                                                                                      \
  static ComponentInfo MakeDescriptor()                                                  \
  {                                                                                      \
    return { EP_APIVERSION, Version, ComponentID(), #Name, Description, Flags };         \
  }                                                                                      \
  template <typename T>                                                                  \
  T* GetImpl() const { return static_cast<T*>(pImpl.ptr()); }                            \
private:                                                                                 \
  UniquePtr<Impl> pImpl = nullptr;                                                       \
  UniquePtr<Impl> CreateImpl(Variant::VarMap initParams)                                 \
  {                                                                                      \
    return UniquePtr<Impl>((Impl*)CreateImplInternal(This::ComponentID(), initParams));  \
  }


// emit getter and setter magic
#define EP_MAKE_GETTER(Getter)                                                           \
  []() -> VarMethod {                                                                    \
    struct Shim                                                                          \
    {                                                                                    \
      Variant get(Slice<const Variant>)                                                  \
      {                                                                                  \
        return Variant(((const This*)(const Component*)this)->Getter());                 \
      }                                                                                  \
    };                                                                                   \
    return VarMethod(&Shim::get);                                                        \
  }()

#define EP_MAKE_SETTER(Setter)                                                           \
  []() -> VarMethod {                                                                    \
    struct Shim                                                                          \
    {                                                                                    \
      Variant set(Slice<const Variant> args)                                             \
      {                                                                                  \
        using PT = function_traits<decltype(&This::Setter)>::template arg<0>::type;      \
        try {                                                                            \
          ((This*)(Component*)this)->Setter(args[0].as<std::remove_reference<PT>::type>()); \
          return Variant();                                                              \
        } catch (EPException &) {                                                        \
          return Variant(GetError()); /* it's already on the stack */                    \
        } catch (std::exception &e) {                                                    \
          return Variant(PushError(epR_CppException, e.what()));                         \
        } catch (...) {                                                                  \
          return Variant(PushError(epR_CppException, "C++ exception"));                  \
        }                                                                                \
      }                                                                                  \
    };                                                                                   \
    return VarMethod(&Shim::set);                                                        \
  }()


// make property with getter and setter
#define EP_MAKE_PROPERTY(Name, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, EP_MAKE_GETTER(Get##Name), EP_MAKE_SETTER(Set##Name), UIType, Flags)

// make property with getter only (read only)
#define EP_MAKE_PROPERTY_RO(Name, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, EP_MAKE_GETTER(Get##Name), nullptr, UIType, Flags)

// make property with setter only (write only)
#define EP_MAKE_PROPERTY_WO(Name, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, nullptr, EP_MAKE_SETTER(Set##Name), UIType, Flags)

// make property with explicit getter and setter
#define EP_MAKE_PROPERTY_EXPLICIT(Name, Description, Getter, Setter, UIType, Flags)      \
([]() -> PropertyInfo {                                                                  \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Name, Description, UIType, Flags,                                                \
    Getter,                                                                              \
    Setter                                                                               \
  };                                                                                     \
}())


// make method
#define EP_MAKE_METHOD(Name, Description) \
  EP_MAKE_METHOD_EXPLICIT(#Name, Name, Description)

// make method explicit
#define EP_MAKE_METHOD_EXPLICIT(Name, Method, Description)                               \
([]() -> MethodInfo {                                                                    \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Description,                                                                     \
    []() -> VarMethod {                                                                  \
      struct Shim                                                                        \
      {                                                                                  \
        Variant call(Slice<const Variant> args)                                          \
        {                                                                                \
          return VarCall(fastdelegate::MakeDelegate((This*)(Component*)this, &This::Method), args); \
        }                                                                                \
      };                                                                                 \
      return VarMethod(&Shim::call);                                                     \
    }()                                                                                  \
  };                                                                                     \
}())


// make event
#define EP_MAKE_EVENT(Name, Description) \
  EP_MAKE_EVENT_EXPLICIT(#Name, Name, Description)

// make event explicit
#define EP_MAKE_EVENT_EXPLICIT(Name, Event, Description)                                 \
([]() -> EventInfo {                                                                     \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Name, Description,                                                               \
    []() -> VarMethod {                                                                  \
      struct Shim                                                                        \
      {                                                                                  \
        Variant subscribe(Slice<const Variant> args)                                     \
        {                                                                                \
          auto d = args[0].as<decltype(This::Event)::EvDelegate>();                      \
          return ((This*)(Component*)this)->Event.Subscribe(d);                          \
        }                                                                                \
      };                                                                                 \
      return VarMethod(&Shim::subscribe);                                                \
    }()                                                                                  \
  };                                                                                     \
}())


// make static function
#define EP_MAKE_STATICFUNC(Name, Description) \
  EP_MAKE_STATICFUNC_EXPLICIT(#Name, Name, Description)

// make static function explicit
#define EP_MAKE_STATICFUNC_EXPLICIT(Name, Function, Description)                         \
([]() -> StaticFuncInfo {                                                                \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Description,                                                                     \
    [](Slice<const Variant> args) -> Variant {                                           \
      return ep::VarCall(&This::Function, args);                                         \
    }                                                                                    \
  };                                                                                     \
}())

#define EP_FRIENDS_WITH_IMPL(component) friend class component; friend class component##Impl

} // ep

#include "ep/cpp/internal/componentdesc_inl.h"

#endif
