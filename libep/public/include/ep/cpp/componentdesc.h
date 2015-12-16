#if !defined(_EP_COMPONENTDESC_HPP)
#define _EP_COMPONENTDESC_HPP

#include "ep/c/componentdesc.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/event.h"

// HACK: TODO: this shouldn't be here
namespace kernel {
  struct ComponentDesc;
  class Kernel;
}

namespace ep {

typedef Variant(Getter)(const Component *pBaseComponent, const void *pDerivedInstance);
typedef void(Setter)(Component *pBaseComponent, void *pDerivedInstance, const Variant *pValue);
typedef Variant(MethodCall)(const Component *pBaseComponent, const void *pDerivedInstance, const Variant *pArgs, size_t numArgs);
typedef Variant(Subscribe)(const Component *pBaseComponent, const void *pDerivedInstance, Variant::VarDelegate *pDelegate);

typedef Variant(StaticCall)(Slice<const Variant> args);

struct PropertyInfo
{
  SharedString id;
  SharedString displayName;
  SharedString description;

  SharedString displayType;
  uint32_t flags;

  void *pGetterMethod;
  void *pSetterMethod;
};
struct MethodInfo
{
  SharedString id;
  SharedString description;

  void *pMethod;
};
struct EventInfo
{
  SharedString id;
  SharedString displayName;
  SharedString description;

  void *pSubscribe;
};
struct StaticFuncInfo
{
  SharedString id;
  SharedString description;

  StaticCall *pCall;
};

struct ComponentInfo
{
  int epVersion;
  int pluginVersion;

  SharedString id;          // an id for this component
  SharedString displayName; // display name
  SharedString description; // description

  // icon image...?

  // TODO: add flags ('Abstract' (can't create) flag)
};

struct ComponentDesc
{
  typedef epResult(InitComponent)(Kernel*);
  typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  typedef void *(CreateImplCallback)(Component *pInstance, Variant::VarMap initParams);

  ComponentInfo info;

  SharedString baseClass;

  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;
  CreateImplCallback *pCreateImpl;

  Array<const PropertyInfo> properties;
  Array<const MethodInfo> methods;
  Array<const EventInfo> events;
  Array<const StaticFuncInfo> staticFuncs;

  // TODO: move this back into kernel, it shouldn't be here...
  const ComponentDesc *pSuperDesc;
};


// base class for pImpl types
template <typename C, typename I>
class BaseImpl : public I
{
public:
  using Component = C;
  using Interface = I;
  using Super = BaseImpl<C, I>;

  const kernel::ComponentDesc* GetDescriptor() const { return (const kernel::ComponentDesc*)pInstance->GetDescriptor(); }
  kernel::Kernel* GetKernel() const { return (kernel::Kernel*)&pInstance->GetKernel(); }

protected:
  template<typename T, bool b> friend struct internal::Destroy;

  BaseImpl(C *pInstance)
    : pInstance(pInstance)
  {}
  virtual ~BaseImpl() {}

  C *pInstance;
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
#define EP_DECLARE_COMPONENT(Name, SuperType, Version, Description)                      \
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
        for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = (char)tolower(#Name[i]);       \
      id = SharedString(buf, sizeof(buf)-1);                                             \
    }                                                                                    \
    return id;                                                                           \
  }                                                                                      \
  static ComponentInfo MakeDescriptor()                                                  \
  {                                                                                      \
    return { EP_APIVERSION, Version, ComponentID(), #Name, Description };                \
  }                                                                                      \
private:

// declare magic for a C++ component with a pImpl interface
#define EP_DECLARE_COMPONENT_WITH_IMPL(Name, Interface, SuperType, Version, Description) \
public:                                                                                  \
  friend class ::ep::Kernel;                                                             \
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
        for (size_t i = 0; i < sizeof(buf); ++i) buf[i] = (char)tolower(#Name[i]);       \
      id = SharedString(buf, sizeof(buf)-1);                                             \
    }                                                                                    \
    return id;                                                                           \
  }                                                                                      \
  static ComponentInfo MakeDescriptor()                                                  \
  {                                                                                      \
    return { EP_APIVERSION, Version, ComponentID(), #Name, Description };                \
  }                                                                                      \
private:                                                                                 \
  UniquePtr<Impl> pImpl = nullptr;                                                       \
  UniquePtr<Impl> CreateImpl(Variant::VarMap initParams)                                 \
  {                                                                                      \
    return UniquePtr<Impl>((Impl*)CreateImplInternal(This::ComponentID(), initParams));  \
  }


// emit getter and setter magic
#define EP_MAKE_GETTER(Getter)                                                           \
  []() -> void* {                                                                        \
    struct Shim                                                                          \
    {                                                                                    \
      Variant get()                                                                      \
      {                                                                                  \
        return Variant(((const This*)this)->Getter());                                   \
      }                                                                                  \
    };                                                                                   \
    auto d = &Shim::get;                                                                 \
    return *(void**)&d;                                                                  \
  }()                                                                                    \

#define EP_MAKE_SETTER(Setter)                                                           \
  []() -> void* {                                                                        \
    struct Shim                                                                          \
    {                                                                                    \
      void set(Variant v)                                                                \
      {                                                                                  \
        using PT = function_traits<decltype(&This::Setter)>::template arg<0>::type;      \
        ((This*)this)->Setter(v.as<std::remove_reference<PT>::type>());                  \
      }                                                                                  \
    };                                                                                   \
    auto d = &Shim::set;                                                                 \
    return *(void**)&d;                                                                  \
  }()                                                                                    \


// make property with getter and setter
#define EP_MAKE_PROPERTY(Name, Description, DisplayType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, EP_MAKE_GETTER(Get##Name), EP_MAKE_SETTER(Set##Name), DisplayType, Flags)

// make property with getter only (read only)
#define EP_MAKE_PROPERTY_RO(Name, Description, DisplayType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, EP_MAKE_GETTER(Get##Name), nullptr, DisplayType, Flags)

// make property with setter only (write only)
#define EP_MAKE_PROPERTY_WO(Name, Description, DisplayType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(#Name, Description, nullptr, EP_MAKE_SETTER(Set##Name), DisplayType, Flags)

// make property with explicit getter and setter
#define EP_MAKE_PROPERTY_EXPLICIT(Name, Description, Getter, Setter, DisplayType, Flags) \
([]() -> PropertyInfo {                                                                  \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Name, Description, DisplayType, Flags,                                           \
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
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Description,                                                                     \
    []() -> void* {                                                                      \
      struct Shim                                                                        \
      {                                                                                  \
        Variant call(Slice<Variant> args)                                                \
        {                                                                                \
          return VarCall(fastdelegate::MakeDelegate((This*)this, &This::Method), args);  \
        }                                                                                \
      };                                                                                 \
      auto d = &Shim::call;                                                              \
      return *(void**)&d;                                                                \
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
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Name, Description,                                                               \
    []() -> void* {                                                                      \
      struct Shim                                                                        \
      {                                                                                  \
        void subscribe(const Variant::VarDelegate &handler)                              \
        {                                                                                \
          Variant v(handler);                                                            \
          ((This*)this)->Event.Subscribe(v.as<decltype(This::Event)::EvDelegate>());     \
        }                                                                                \
      };                                                                                 \
      auto d = &Shim::subscribe;                                                         \
      return *(void**)&d;                                                                \
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
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Description,                                                                     \
    [](Slice<const Variant> args) -> Variant {                                           \
      Variant r;                                                                         \
      new(&r) Variant(ep::VarCall(&This::Function, args));                               \
      return r;                                                                          \
    }                                                                                    \
  };                                                                                     \
}())

} // ep

#include "ep/cpp/internal/componentdesc_inl.h"

#endif
