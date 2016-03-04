namespace ep {

// interface for getters, setters, methods, events

// getter glue
class GetterShim
{
public:
  GetterShim(void *pGetter, SharedPtr<const RefCounted> data = nullptr) : pGetter(pGetter), data(data) {}

  explicit operator bool() const { return pGetter != nullptr; }

  Variant get(const Component *pThis) const;

protected:
  void *pGetter;
  SharedPtr<const RefCounted> data;
};

// setter glue
class SetterShim
{
public:
  SetterShim(void *pSetter, SharedPtr<const RefCounted> data = nullptr) : pSetter(pSetter), data(data) {}

  explicit operator bool() const { return pSetter != nullptr; }

  void set(Component *pThis, const Variant &value) const;

protected:
  void *pSetter;
  SharedPtr<const RefCounted> data;
};

// method glue
class MethodShim
{
public:
  MethodShim(void *pMethod, SharedPtr<const RefCounted> data = nullptr) : pMethod(pMethod), data(data) {}

  Variant call(Component *pThis, Slice<const Variant> args) const;

protected:
  void *pMethod;
  SharedPtr<const RefCounted> data;
};

// static funcion glue
class StaticFuncShim
{
public:
  StaticFuncShim(void *pFunc, SharedPtr<const RefCounted> data = nullptr) : pFunc(pFunc), data(data) {}

  Variant call(Slice<const Variant> args) const;

protected:
  void *pFunc;
  SharedPtr<const RefCounted> data;
};

// event glue
class EventShim
{
public:
  EventShim(void *pSubscribe, SharedPtr<const RefCounted> data = nullptr) : pSubscribe(pSubscribe), data(data) {}

  void subscribe(Component *pThis, const Variant::VarDelegate &d) const;

protected:
  void *pSubscribe;
  SharedPtr<const RefCounted> data;
};

struct PropertyDesc : public PropertyInfo
{
  PropertyDesc(const PropertyInfo &info, const GetterShim &getter, const SetterShim &setter)
    : PropertyInfo(info), getter(getter), setter(setter)
  {}

  GetterShim getter;
  SetterShim setter;
};

struct MethodDesc : public MethodInfo
{
  MethodDesc(const MethodInfo &desc, const MethodShim &method)
    : MethodInfo(desc), method(method)
  {}

  MethodShim method;
};

struct StaticFuncDesc : public StaticFuncInfo
{
  StaticFuncDesc(const StaticFuncInfo &desc, const StaticFuncShim &staticFunc)
    : StaticFuncInfo(desc), staticFunc(staticFunc)
  {}

  StaticFuncShim staticFunc;
};

struct EventDesc : public EventInfo
{
  EventDesc(const EventInfo &desc, const EventShim &ev)
    : EventInfo(desc), ev(ev)
  {}

  EventShim ev;
};

// functions
inline Variant GetterShim::get(const ep::Component *pThis) const
{
  // hack to force construct a delegate
  if (data)
  {
    // indirect call with metadata
    FastDelegate<Variant(const RefCounted &)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pGetter;
    return d(*data);
  }
  else
  {
    // direct call
    FastDelegate<Variant()> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pGetter;
    return d();
  }
}

inline void SetterShim::set(ep::Component *pThis, const Variant &value) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<void(const RefCounted &, const Variant &value)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSetter;
    d(*data, value);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<void(const Variant &value)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSetter;
    d(value);
  }
}

inline Variant MethodShim::call(ep::Component *pThis, Slice<const Variant> args) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<Variant(const RefCounted &, Slice<const Variant>)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pMethod;
    return d(*data, args);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<Variant(Slice<const Variant>)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pMethod;
    return d(args);
  }
}

inline Variant StaticFuncShim::call(Slice<const Variant> args) const
{
  if (data)
  {
    // indirect call with metadata
    auto pF = (Variant(*)(const RefCounted &, Slice<const Variant>))pFunc;
    return pF(*data, args);
  }
  else
  {
    auto pF = (Variant(*)(Slice<const Variant>))pFunc;
    return pF(args);
  }
}

inline void EventShim::subscribe(ep::Component *pThis, const Variant::VarDelegate &handler) const
{
  if (data)
  {
    // indirect call with metadata
    FastDelegate<void(const RefCounted &, const Variant::VarDelegate&)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSubscribe;
    return d(*data, handler);
  }
  else
  {
    // hack to force construct a delegate
    FastDelegate<void(const Variant::VarDelegate&)> d;
    const void **pD = (const void**)&d;
    pD[0] = pThis;
    pD[1] = pSubscribe;
    return d(handler);
  }
}

class Kernel;

// Internal Component Descriptor struct
// This supplies the Kernel with all the internal Component Descriptor meta information and callback magic
// Language binding specific descriptors should derive from this struct
struct ComponentDescInl : public ComponentDesc
{
  typedef void(InitComponent)(ep::Kernel*);
  typedef Component *(CreateInstanceCallback)(const ComponentDesc *pType, ep::Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  typedef void *(CreateImplCallback)(Component *pInstance, Variant::VarMap initParams);

  // Helper function to populate the Component Descriptor from another Descriptor - usually used to populate from a super class
  void PopulateFromDesc(const ComponentDescInl *pDesc)
  {
    for (auto p : pDesc->propertyTree)
      if (!propertyTree.Get(p.key))
        propertyTree.Insert(p.key, p.value);

    for (auto m : pDesc->methodTree)
      if (!methodTree.Get(m.key))
        methodTree.Insert(m.key, m.value);

    for (auto e : pDesc->eventTree)
      if (!eventTree.Get(e.key))
        eventTree.Insert(e.key, e.value);

    for (auto f : pDesc->staticFuncTree)
      if (!staticFuncTree.Get(f.key))
        staticFuncTree.Insert(f.key, f.value);
  }

  SharedString baseClass;   // The base/super class type id

  // Callbacks
  InitComponent *pInit;
  CreateInstanceCallback *pCreateInstance;
  CreateImplCallback *pCreateImpl;

  // Meta data trees
  AVLTree<String, PropertyDesc> propertyTree;
  AVLTree<String, MethodDesc> methodTree;
  AVLTree<String, EventDesc> eventTree;
  AVLTree<String, StaticFuncDesc> staticFuncTree;
};

} // ep
