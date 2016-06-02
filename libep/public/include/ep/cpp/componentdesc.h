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

  SharedArray<SharedString> argTypes;

  VarMethod pMethod;
};
struct EventInfo
{
  SharedString id;
  SharedString displayName;
  SharedString description;

  SharedArray<SharedString> argTypes;

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

  SharedString nameSpace;   // namespace
  SharedString name;        // name
  SharedString identifier;  // identifier

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

// declare magic for a C++ component
#define EP_DECLARE_COMPONENT(Namespace, Name, SuperType, Version, Description, Flags)    \
public:                                                                                  \
  friend class ::ep::Kernel;                                                             \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = ep::SharedPtr<This>;                                                       \
  using Impl = void;                                                                     \
  static ep::SharedString ComponentID() { return ComponentInfo().identifier; }           \
  static const ep::ComponentInfo& ComponentInfo()                                        \
  {                                                                                      \
    static const ep::ComponentInfo info                                                  \
    {                                                                                    \
      EP_APIVERSION, Version,                                                            \
      #Namespace, std::move(ep::MutableString<0>(#Name).toLower()),                      \
      std::move(ep::MutableString<0>(ep::Concat, #Namespace, '.', ep::MutableString<0>(#Name).toLower())), \
      #Name, Description,                                                                \
      Flags                                                                              \
    };                                                                                   \
    return info;                                                                         \
  }                                                                                      \
private:

// declare magic for a C++ component with a pImpl interface
#define EP_DECLARE_COMPONENT_WITH_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags) \
  friend class ::ep::Kernel;                                                                               \
  __EP_DECLARE_COMPONENT_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)

#define __EP_DECLARE_COMPONENT_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)    \
public:                                                                                  \
  friend class Name##Impl;                                                               \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = ep::SharedPtr<This>;                                                       \
  using Impl = ep::BaseImpl<Name, Interface>;                                            \
  static ep::SharedString ComponentID() { return ComponentInfo().identifier; }           \
  static const ep::ComponentInfo& ComponentInfo()                                        \
  {                                                                                      \
    static const ep::ComponentInfo info                                                  \
    {                                                                                    \
      EP_APIVERSION, Version,                                                            \
      #Namespace, ep::MutableString<0>(#Name).toLower(),                                 \
      ep::MutableString<0>(ep::Concat, #Namespace, '.', ep::MutableString<0>(#Name).toLower()), \
      #Name, Description,                                                                \
      Flags                                                                              \
    };                                                                                   \
    return info;                                                                         \
  }                                                                                      \
  template <typename T>                                                                  \
  T* GetImpl() const { return static_cast<T*>(pImpl.ptr()); }                            \
private:                                                                                 \
  ep::UniquePtr<Impl> pImpl = nullptr;                                                   \
  ep::UniquePtr<Impl> CreateImpl(ep::Variant::VarMap initParams)                         \
  {                                                                                      \
    using namespace ep;                                                                  \
    return UniquePtr<Impl>((Impl*)CreateImplInternal(This::ComponentID(), initParams));  \
  }


#define EP_DECLARE_COMPONENT_WITH_STATIC_IMPL(Namespace, Name, Interface, StaticInterface, SuperType, Version, Description, Flags)  \
  EP_DECLARE_COMPONENT_WITH_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)                                \
public:                                                                                                                             \
  static StaticInterface* GetStaticImpl()                                                                                           \
  {                                                                                                                                 \
    static BaseStaticImpl<StaticInterface> *pStaticImpl = nullptr;                                                                  \
    if(!pStaticImpl)                                                                                                                \
      pStaticImpl = static_cast<BaseStaticImpl<StaticInterface>*>(internal::GetStaticImpl(ComponentID()));                          \
    return static_cast<StaticInterface*>(pStaticImpl);                                                                              \
  }                                                                                                                                 \
private:


// emit getter and setter magic
#define EP_MAKE_GETTER(Getter)                                                           \
  []() -> ep::VarMethod {                                                                \
    using namespace ep;                                                                  \
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
  []() -> ep::VarMethod {                                                                \
    using namespace ep;                                                                  \
    struct Shim                                                                          \
    {                                                                                    \
      Variant set(Slice<const Variant> args)                                             \
      {                                                                                  \
        using PT = internal::function_traits<decltype(&This::Setter)>::template arg<0>::type;      \
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
([]() -> ep::PropertyInfo {                                                              \
  using namespace ep;                                                                    \
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
([]() -> ep::MethodInfo {                                                                \
  using namespace ep;                                                                    \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Description, Array<SharedString>(Alloc, internal::function_traits<decltype(&This::Method)>::num_args), \
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
([]() -> ep::EventInfo {                                                                 \
  using namespace ep;                                                                    \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Name, Description, Array<SharedString>(Alloc, decltype(This::Event)::ParamCount), \
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
([]() -> ep::StaticFuncInfo {                                                            \
  using namespace ep;                                                                    \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)epToLower(Name[i]);              \
  return{                                                                                \
    id, Description,                                                                     \
    [](Slice<const Variant> args) -> Variant {                                           \
      return VarCall(&This::Function, args);                                             \
    }                                                                                    \
  };                                                                                     \
}())

#define EP_FRIENDS_WITH_IMPL(component) friend class component; friend class component##Impl

} // ep

#include "ep/cpp/internal/componentdesc_inl.h"

#endif
