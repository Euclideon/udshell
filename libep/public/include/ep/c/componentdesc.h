#if !defined(_EP_COMPONENTDESC_H)
#define _EP_COMPONENTDESC_H

#include "ep/c/variant.h"
#include "ep/c/delegate.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epComponent;

typedef epVariant(epGetter)(const epComponent *pBaseComponent, const void *pDerivedInstance);
typedef void(epSetter)(epComponent *pBaseComponent, void *pDerivedInstance, const epVariant *pValue);
typedef epVariant(epMethodCall)(const epComponent *pBaseComponent, const void *pDerivedInstance, const epVariant *pArgs, size_t numArgs);
typedef epVariant(epStaticCall)(const epVariant *pValue, size_t numArgs);
typedef epVariant(epSubscribe)(const epComponent *pBaseComponent, const void *pDerivedInstance, epVarDelegate *pDelegate);

struct epPropertyDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  const char *displayType;
  uint32_t flags;

  epGetter *pGetter;
  epSetter *pSetter;

  void *pGetterMethod;
  void *pSetterMethod;
};
struct epMethodDesc
{
  const char *id;
  const char *description;

  epMethodCall *pCall;

  void *pMethod;
};
struct epEventDesc
{
  const char *id;
  const char *displayName;
  const char *description;

  epSubscribe *pSubscribe;
};
struct epStaticFuncDesc
{
  const char *id;
  const char *description;

  epStaticCall *pCall;
};

struct epComponentInfo
{
  int pluginVersion;

  const char *id;          // an id for this component
  const char *displayName; // display name
  const char *description; // description
};

struct epComponentDesc
{
  epComponentInfo info;

  const char *baseClass;
  const struct epComponentOverrides *pOverrides;

  size_t numProperties;
  const epPropertyDesc *pProperties;
  size_t numMethods;
  const epMethodDesc *pMethods;
  size_t numEvents;
  const epEventDesc *pEvents;
  size_t numStaticFuncs;
  const epStaticFuncDesc *pStaticFuncs;
};

#if defined(__cplusplus)
}
#endif


#if defined(EP_CPP11)

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
  static epComponentInfo GetDescriptor()                                                 \
  {                                                                                      \
    static char id[sizeof(#Name)];                                                       \
    for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(#Name[i]);             \
    return { Version, id, #Name, Description };                                          \
  }                                                                                      \
private:


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
    auto d = fastdelegate::MakeDelegate((Shim*)nullptr, &Shim::get);                     \
    return ((void**)&d)[1];                                                              \
  }()                                                                                    \

#define EP_MAKE_SETTER(Setter)                                                           \
  []() -> void* {                                                                        \
    struct Shim                                                                          \
    {                                                                                    \
      void set(Variant v)                                                                \
      {                                                                                  \
        using PT = function_traits<decltype(&This::Setter)>::template arg<0>::type;      \
        ((This*)this)->Setter(v.as<PT>());                                               \
      }                                                                                  \
    };                                                                                   \
    auto d = fastdelegate::MakeDelegate((Shim*)nullptr, &Shim::set);                     \
    return ((void**)&d)[1];                                                              \
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
([]() -> epPropertyDesc {                                                                \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Name, Description, DisplayType, Flags,                                           \
    nullptr, nullptr,                                                                    \
    Getter,                                                                              \
    Setter                                                                               \
  };                                                                                     \
}())


// make method
#define EP_MAKE_METHOD(Name, Description) \
  EP_MAKE_METHOD_EXPLICIT(#Name, Name, Description)

// make method explicit
#define EP_MAKE_METHOD_EXPLICIT(Name, Method, Description)                               \
([]() -> epMethodDesc {                                                                  \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Description,                                                                     \
    nullptr,                                                                             \
    []() -> void* {                                                                      \
      struct Shim                                                                        \
      {                                                                                  \
        Variant call(Slice<Variant> args)                                                \
        {                                                                                \
          return VarCall(fastdelegate::MakeDelegate((This*)this, &This::Method), args);  \
        }                                                                                \
      };                                                                                 \
      auto d = fastdelegate::MakeDelegate((Shim*)nullptr, &Shim::call);                  \
      return ((void**)&d)[1];                                                            \
    }()                                                                                  \
  };                                                                                     \
}())


/* TODO: finish this when events are using weak-ref's correctly
// make event
#define EP_MAKE_EVENT(Name, Description) \
  EP_MAKE_EVENT_EXPLICIT(#Name, Name, Description)

// make event explicit
#define EP_MAKE_EVENT_EXPLICIT(Name, Event, Description)                                 \
([]() -> epEventDesc {                                                                   \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Name, Description,                                                               \
    [](epComponent *pBaseComponent, void *pDerivedInstance, epVarDelegate *pDelegate) -> epVariant { \
      if(pDerivedInstance)                                                               \
        ((This*)pDerivedInstance)->Event.Subscribe((Variant::VarDelegate&)pDelegate);    \
      else                                                                               \
        ((This*)(Component*)pBaseComponent)->Event.Subscribe((Variant::VarDelegate&)pDelegate);      \
      return Variant();                                                                  \
    }()                                                                                  \
  };                                                                                     \
}())
*/


// make static function
#define EP_MAKE_STATICFUNC(Name, Description) \
  EP_MAKE_STATICFUNC_EXPLICIT(#Name, Name, Description)

// make static function explicit
#define EP_MAKE_STATICFUNC_EXPLICIT(Name, Function, Description)                         \
([]() -> epStaticFuncDesc {                                                              \
  static char id[sizeof(Name)];                                                          \
  for (size_t i = 0; i < sizeof(id); ++i) id[i] = (char)tolower(Name[i]);                \
  return{                                                                                \
    id, Description,                                                                     \
    [](const epVariant *pValue, size_t numArgs) -> epVariant {                           \
      epVariant r;                                                                       \
      new(&r) Variant(ep::VarCall(&This::Function, Slice<const Variant>((const Variant*)pValue, numArgs))); \
      return r;                                                                          \
    }                                                                                    \
  };                                                                                     \
}())


#endif // EP_CPP11

#endif
