#if !defined(_EP_COMPONENTDESC_HPP)
#define _EP_COMPONENTDESC_HPP

#include "ep/cpp/variant.h"
#include "ep/cpp/delegate.h"
#include "ep/cpp/event.h"

#include <tuple>

namespace ep {

namespace internal {
  template <typename T, typename U>
  struct CreateHelper;
}
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

//  SharedString displayName; // TODO: this requires human-entry in our meta registration macros...
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

//  SharedString displayName; // TODO: this requires human-entry in our meta registration macros...
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

//  SharedString displayName; // display name  // TODO: this requires human-entry in our meta registration macros...
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


// TODO: find alternative solution for this block of SFINAE madness!
#define EP_SFINAE_META_GETTERS                                                                                                                                                       \
  template <typename T> static inline auto tryCallGetProperties(const T* t) -> decltype(t->getProperties(), ep::Array<const ep::PropertyInfo>()) { return t->getProperties(); }      \
  static inline ep::Array<const ep::PropertyInfo> tryCallGetProperties(...) { return nullptr; }                                                                                      \
  static inline ep::Array<const ep::PropertyInfo> getPropertiesImpl() { return tryCallGetProperties((This*)nullptr); }                                                               \
  template <typename T> static inline auto tryCallGetMethods(const T* t) -> decltype(t->getMethods(), ep::Array<const ep::MethodInfo>()) { return t->getMethods(); }                 \
  static inline ep::Array<const ep::MethodInfo> tryCallGetMethods(...) { return nullptr; }                                                                                           \
  static inline ep::Array<const ep::MethodInfo> getMethodsImpl() { return tryCallGetMethods((This*)nullptr); }                                                                       \
  template <typename T> static inline auto tryCallGetEvents(const T* t) -> decltype(t->getEvents(), ep::Array<const ep::EventInfo>()) { return t->getEvents(); }                     \
  static inline ep::Array<const ep::EventInfo> tryCallGetEvents(...) { return nullptr; }                                                                                             \
  static inline ep::Array<const ep::EventInfo> getEventsImpl() { return tryCallGetEvents((This*)nullptr); }                                                                          \
  template <typename T> static inline auto tryCallGetStaticFuncs(const T* t) -> decltype(t->getStaticFuncs(), ep::Array<const ep::StaticFuncInfo>()) { return t->getStaticFuncs(); } \
  static inline ep::Array<const ep::StaticFuncInfo> tryCallGetStaticFuncs(...) { return nullptr; }                                                                                   \
  static inline ep::Array<const ep::StaticFuncInfo> getStaticFuncsImpl() { return tryCallGetStaticFuncs((This*)nullptr); }


// declare magic for a C++ component
#define EP_DECLARE_COMPONENT(Namespace, Name, SuperType, Version, Description, Flags)    \
public:                                                                                  \
  friend class ::ep::Kernel;                                                             \
  template <typename T, typename U> friend struct ::ep::internal::CreateHelper;          \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = ep::SharedPtr<This>;                                                       \
  using Impl = void;                                                                     \
  static ep::SharedString componentID() { return componentInfo().identifier; }           \
  static const ep::ComponentInfo& componentInfo()                                        \
  {                                                                                      \
    static const ep::ComponentInfo info                                                  \
    {                                                                                    \
      EP_APIVERSION, Version,                                                            \
      #Namespace, #Name,                                                                 \
      ep::MutableString<0>(ep::Concat, #Namespace, '.', #Name),                          \
      Description,                                                                       \
      Flags                                                                              \
    };                                                                                   \
    return info;                                                                         \
  }                                                                                      \
private:                                                                                 \
  EP_SFINAE_META_GETTERS

// declare magic for a C++ component with a pImpl interface
#define EP_DECLARE_COMPONENT_WITH_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags) \
  friend class ::ep::Kernel;                                                                               \
  template <typename T, typename U> friend struct ::ep::internal::CreateHelper;                            \
  __EP_DECLARE_COMPONENT_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)

#define __EP_DECLARE_COMPONENT_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)    \
public:                                                                                  \
  friend class Name##Impl;                                                               \
  using Super = SuperType;                                                               \
  using This = Name;                                                                     \
  using Ref = ep::SharedPtr<This>;                                                       \
  using Impl = ep::BaseImpl<Name, Interface>;                                            \
  static ep::SharedString componentID() { return componentInfo().identifier; }           \
  static const ep::ComponentInfo& componentInfo()                                        \
  {                                                                                      \
    static const ep::ComponentInfo info                                                  \
    {                                                                                    \
      EP_APIVERSION, Version,                                                            \
      #Namespace, #Name,                                                                 \
      ep::MutableString<0>(ep::Concat, #Namespace, '.', #Name),                          \
      Description,                                                                       \
      Flags                                                                              \
    };                                                                                   \
    return info;                                                                         \
  }                                                                                      \
  template <typename T>                                                                  \
  T* getImpl() const { return static_cast<T*>(pImpl.get()); }                            \
private:                                                                                 \
  ep::UniquePtr<Impl> pImpl = nullptr;                                                   \
  ep::UniquePtr<Impl> createImpl(ep::Variant::VarMap initParams)                         \
  {                                                                                      \
    using namespace ep;                                                                  \
    return UniquePtr<Impl>((Impl*)createImplInternal(This::componentID(), initParams));  \
  }                                                                                      \
  EP_SFINAE_META_GETTERS

#define EP_DECLARE_COMPONENT_WITH_STATIC_IMPL(Namespace, Name, Interface, StaticInterface, SuperType, Version, Description, Flags)  \
  friend class ::ep::Kernel;                                                                                                        \
  template <typename T, typename U> friend struct ::ep::internal::CreateHelper;                                                     \
  __EP_DECLARE_COMPONENT_STATIC_IMPL(Namespace, Name, Interface, StaticInterface, SuperType, Version, Description, Flags)


#define __EP_DECLARE_COMPONENT_STATIC_IMPL(Namespace, Name, Interface, StaticInterface, SuperType, Version, Description, Flags)     \
  __EP_DECLARE_COMPONENT_IMPL(Namespace, Name, Interface, SuperType, Version, Description, Flags)                                   \
public:                                                                                                                             \
  static StaticInterface* getStaticImpl()                                                                                           \
  {                                                                                                                                 \
    static BaseStaticImpl<StaticInterface> *pStaticImpl = nullptr;                                                                  \
    if(!pStaticImpl)                                                                                                                \
      pStaticImpl = static_cast<BaseStaticImpl<StaticInterface>*>(internal::getStaticImpl(componentID()));                          \
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
        using PT = internal::function_traits<decltype(&This::Setter)>::template arg<0>::type; \
        try {                                                                            \
          ((This*)(Component*)this)->Setter(args[0].as<std::remove_reference<PT>::type>()); \
          return Variant();                                                              \
        } catch (EPException &e) {                                                       \
          return Variant(e.claim());                                                     \
        } catch (std::exception &e) {                                                    \
          return Variant(allocError(Result::CppException, e.what()));                    \
        } catch (...) {                                                                  \
          return Variant(allocError(Result::CppException, "C++ exception"));             \
        }                                                                                \
      }                                                                                  \
    };                                                                                   \
    return VarMethod(&Shim::set);                                                        \
  }()


// make property with getter and setter
#define EP_MAKE_PROPERTY(Name, Getter, Setter, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(Name, Description, EP_MAKE_GETTER(Getter), EP_MAKE_SETTER(Setter), UIType, Flags)

// make property with getter only (read only)
#define EP_MAKE_PROPERTY_RO(Name, Getter, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(Name, Description, EP_MAKE_GETTER(Getter), nullptr, UIType, Flags)

// make property with setter only (write only)
#define EP_MAKE_PROPERTY_WO(Name, Setter, Description, UIType, Flags) \
  EP_MAKE_PROPERTY_EXPLICIT(Name, Description, nullptr, EP_MAKE_SETTER(Setter), UIType, Flags)

// make property with explicit getter and setter
#define EP_MAKE_PROPERTY_EXPLICIT(Name, Description, Getter, Setter, UIType, Flags)      \
([]() -> ep::PropertyInfo {                                                              \
  using namespace ep;                                                                    \
  return{                                                                                \
    Name, Description, UIType, Flags,                                                    \
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
  return{                                                                                \
    Name, Description, Array<SharedString>(Alloc, internal::function_traits<decltype(&This::Method)>::num_args), \
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
  return{                                                                                \
    Name, Description, Array<SharedString>(Alloc, decltype(This::Event)::ParamCount),    \
    []() -> VarMethod {                                                                  \
      struct Shim                                                                        \
      {                                                                                  \
        Variant subscribe(Slice<const Variant> args)                                     \
        {                                                                                \
          auto d = args[0].as<decltype(This::Event)::EvDelegate>();                      \
          return ((This*)(Component*)this)->Event.subscribe(d);                          \
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
  return{                                                                                \
    Name, Description,                                                                   \
    [](Slice<const Variant> args) -> Variant {                                           \
      return VarCall(&This::Function, args);                                             \
    }                                                                                    \
  };                                                                                     \
}())

#define EP_FRIENDS_WITH_IMPL(component) friend class component; friend class component##Impl

} // ep

#include "ep/cpp/internal/componentdesc_inl.h"

#endif
