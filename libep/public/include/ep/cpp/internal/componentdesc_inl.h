namespace ep {

namespace internal {

RefCounted* getStaticImpl(String name);
void addStaticImpl(SharedString name, UniquePtr<RefCounted> upImpl);

} // namespace internal

class Kernel;


// base class for pImpl types
template <typename C, typename I>
class BaseImpl : public I
{
  friend C;
  template <typename T>
  friend void ::_epDelete(T *pMemory);

public:
  using Instance = C;
  using Interface = I;
  using ImplSuper = BaseImpl<C, I>;

  const ComponentDescInl* getDescriptor() const { return (const ComponentDescInl*)pInstance->getDescriptor(); }
  const ComponentDescInl* getSuperDescriptor() const { return (const ComponentDescInl*)getDescriptor()->pSuperDesc; }
  Kernel* getKernel() const { return &pInstance->getKernel(); }

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

template <typename I>
class BaseStaticImpl : public RefCounted, public I
{};


// interface for getters, setters, methods, events

typedef MethodPointer<Variant(Slice<const Variant>, const RefCounted &)> VarMethodWithData;

// method glue
struct MethodShim
{
public:
  MethodShim(const MethodShim &rh) : method(rh.method), data(rh.data) {}
  MethodShim(MethodShim &&rh) : method(rh.method), data(std::move(rh.data)) {}

  MethodShim(VarMethod method) : method(method) {}
  MethodShim(VarMethodWithData method, SharedPtr<const RefCounted> data)
    : methodWithData(method), data(data) { EPASSERT(data != nullptr, "Invalid; 'data' is nullptr!"); }

  explicit operator bool() const { return (bool)method; }

  Variant get(Component *pThis) const;
  Variant set(Component *pThis, const Variant &value) const;
  Variant call(Component *pThis, Slice<const Variant> args) const;

  VarDelegate getDelegate(Component *pThis) const;

protected:
  // HACK: remove this friendship!
  friend class DynamicComponent;

  union
  {
    VarMethod method;
    VarMethodWithData methodWithData;
  };
  SharedPtr<const RefCounted> data;
};

// static funcion glue
struct StaticFuncShim
{
public:
  StaticFuncShim(void *pFunc, SharedPtr<const RefCounted> data = nullptr) : pFunc(pFunc), data(data) {}

  Variant call(Slice<const Variant> args) const;

protected:
  void *pFunc;
  SharedPtr<const RefCounted> data;
};

// event glue
struct EventShim
{
public:
  EventShim(const EventShim &rh) : subscribeFunc(rh.subscribeFunc), data(rh.data) {}
  EventShim(EventShim &&rh) : subscribeFunc(rh.subscribeFunc), data(std::move(rh.data)) {}

  EventShim(VarMethod subscribe) : subscribeFunc(subscribe) {}
  EventShim(VarMethodWithData subscribe, SharedPtr<const RefCounted> data) : subscribeWithDataFunc(subscribe), data(data) {}

  Variant subscribe(Component *pThis, const VarDelegate &d) const;

protected:
  union
  {
    VarMethod subscribeFunc;
    VarMethodWithData subscribeWithDataFunc;
  };
  SharedPtr<const RefCounted> data;
};

struct PropertyDesc : public PropertyInfo
{
  PropertyDesc(const PropertyInfo &info, const MethodShim &getter, const MethodShim &setter)
    : PropertyInfo(info), getter(getter), setter(setter) {}

  MethodShim getter;
  MethodShim setter;
};

struct MethodDesc : public MethodInfo
{
  MethodDesc(const MethodInfo &desc, const MethodShim &method)
    : MethodInfo(desc), method(method) {}

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
inline Variant MethodShim::get(Component *pThis) const
{
  if (data)
    return methodWithData.call(pThis, nullptr, *data);
  else
    return method.call(pThis, nullptr);
}
inline Variant MethodShim::set(Component *pThis, const Variant &value) const
{
  if (data)
    return methodWithData.call(pThis, Slice<const Variant>(&value, 1), *data);
  else
    return method.call(pThis, Slice<const Variant>(&value, 1));
}
inline Variant MethodShim::call(Component *pThis, Slice<const Variant> args) const
{
  if (data)
    return methodWithData.call(pThis, args, *data);
  else
    return method.call(pThis, args);
}
inline VarDelegate MethodShim::getDelegate(Component *pThis) const
{
  if (data)
  {
    // we need to allocate a delegate that keeps the metadata
    class MethodDelegate : public DelegateMemento
    {
    public:
      Variant call(Slice<const Variant> args) const
      {
        return method(args, *data);
      }
      MethodDelegate(VarMethodWithData::FastDelegateType method, const SharedPtr<const RefCounted> &data)
        : method(method), data(data)
      {
        // set the memento to the lua call shim
        VarDelegate::FastDelegateType shim(this, &MethodDelegate::call);
        m = shim.GetMemento();
      }

      VarMethodWithData::FastDelegateType method;
      SharedPtr<const RefCounted> data;
    };

    // indirect delegate carries metadata
    auto d = methodWithData.getDelegate(pThis);
    return VarDelegate(SharedPtr<MethodDelegate>::create(d, data));
  }
  else
    return method.getDelegate(pThis);
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

inline Variant EventShim::subscribe(Component *pThis, const VarDelegate &handler) const
{
  Variant v(handler);
  if (data)
    return subscribeWithDataFunc.call(pThis, Slice<const Variant>(&v, 1), *data);
  else
    return subscribeFunc.call(pThis, Slice<const Variant>(&v, 1));
}

// Internal Component Descriptor struct
// This supplies the Kernel with all the internal Component Descriptor meta information and callback magic
// Language binding specific descriptors should derive from this struct
struct ComponentDescInl : public ComponentDesc
{
  typedef void(InitComponent)(ep::Kernel*);
  typedef ComponentRef (CreateInstanceCallback)(const ComponentDesc *pType, ep::Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  typedef void *(CreateImplCallback)(Component *pInstance, Variant::VarMap initParams);

  // Helper function to populate the Component Descriptor from another Descriptor - usually used to populate from a super class
  void PopulateFromDesc(const ComponentDescInl *pDesc)
  {
    for (auto p : pDesc->propertyTree)
      propertyTree.tryInsert(p.key, p.value);
    for (auto m : pDesc->methodTree)
      methodTree.tryInsert(m.key, m.value);
    for (auto e : pDesc->eventTree)
      eventTree.tryInsert(e.key, e.value);
    for (auto f : pDesc->staticFuncTree)
      staticFuncTree.tryInsert(f.key, f.value);
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
