#include "ep/cpp/variant.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/plugin.h"

extern "C" {

void epVariant_Release(epVariant v)
{
  // Note: Variant's destructor will clean our instance up
  Variant t;
  (epVariant&)t = v;
}

epVariant epVariant_CreateVoid()
{
  epVariant v;
  new(&v) Variant(Variant::Type::Void);
  return v;
}
epVariant epVariant_CreateNull()
{
  epVariant v;
  new(&v) Variant(nullptr);
  return v;
}
epVariant epVariant_CreateBool(char b)
{
  epVariant v;
  new(&v) Variant(b ? true : false);
  return v;
}
epVariant epVariant_CreateInt(int64_t i)
{
  epVariant v;
  new(&v) Variant(i);
  return v;
}
epVariant epVariant_CreateFloat(double f)
{
  epVariant v;
  new(&v) Variant(f);
  return v;
}
epVariant epVariant_CreateComponent(epComponent *pComponent)
{
  epVariant v;
  new(&v) Variant((ComponentRef&)pComponent);
  return v;
}
//inline epVariant epVariant_CreateDelegate() {}
epVariant epVariant_CreateCString(const char *pString)
{
  epVariant v;
  new(&v) Variant(String(pString));
  return v;
}
epVariant epVariant_CreateString(epString string)
{
  epVariant v;
  new(&v) Variant((String&)string);
  return v;
}

epVariantType epVariant_GetType(epVariant v)
{
  return (epVariantType)v.t;
}

int epVariant_IsVoid(epVariant v)
{
  return v.t == epVT_Void;
}
int epVariant_IsNull(epVariant v)
{
  return v.t == epVT_Null || (v.t == epVT_String && v.length == 0) || (v.t == epVT_SharedPtr && v.p == NULL);
}
char epVariant_AsBool(epVariant v)
{
  return (char)((Variant&)v).asBool();
}
int64_t epVariant_AsInt(epVariant v)
{
  return ((Variant&)v).asInt();
}
double epVariant_AsFloat(epVariant v)
{
  return ((Variant&)v).asFloat();
}
epComponent* epVariant_AsComponent(epVariant v)
{
  epComponent *pC;
  new(&pC) ComponentRef(((Variant&)v).asComponent());
  return pC;
}
//char epVariant_GetDelegate(epVariantHandle v) {}
epString epVariant_AsString(epVariant v)
{
  epString r;
  new(&r) String(((Variant&)v).asString());
  return r;
}

const epVariant* epVariant_AsArray(epVariant v, size_t *pLength)
{
  Slice<Variant> arr = ((Variant&)v).asArray();
  *pLength = arr.length;
  return (epVariant*)arr.ptr;
}
const epVarMap* epVariant_AsAssocArray(epVariant v)
{
  return (epVarMap*)v.p;
}

} // extern "C"


ptrdiff_t epStringifyVariant(Slice<char> buffer, String format, const Variant &v, const epVarArg *pArgs)
{
  switch (v.type())
  {
    case Variant::Type::Void:
      return epStringifyTemplate(buffer, format, String("void"), pArgs);
    case Variant::Type::Null:
      return epStringifyTemplate(buffer, format, nullptr, pArgs);
    case Variant::Type::Bool:
      return epStringifyTemplate(buffer, format, v.asBool(), pArgs);
    case Variant::Type::Int:
      return epStringifyTemplate(buffer, format, v.asInt(), pArgs);
    case Variant::Type::Float:
      return epStringifyTemplate(buffer, format, v.asFloat(), pArgs);
    case Variant::Type::Enum:
    case Variant::Type::Bitfield:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
    case Variant::Type::String:
    case Variant::Type::SmallString:
      return epStringifyTemplate(buffer, format, v.asString(), pArgs);
    case Variant::Type::SharedPtr:
      switch (v.spType())
      {
        case Variant::SharedPtrType::Component:
          return epStringifyTemplate(buffer, format, v.asComponent(), pArgs);
        case Variant::SharedPtrType::Delegate:
          return epStringifyTemplate(buffer, format, v.asDelegate(), pArgs);
        case Variant::SharedPtrType::AssocArray:
          EPASSERT(false, "TODO! Please write me!");
          return 0;
        default:
          EPASSERT(false, "TODO! Please write me!");
          return 0;
      }
    case Variant::Type::Array:
      return epStringifyTemplate(buffer, format, v.asArray(), pArgs);
    case Variant::Type::Error:
      EPASSERT(false, "TODO! Please write me!");
      return 0;
  }
  return 0;
}

namespace ep {

namespace internal {

const Variant::Type s_typeTranslation[] =
{
  Variant::Type::Void,      // epVT_Void
  Variant::Type::Error,     // epVT_Error

  Variant::Type::Null,      // epVT_Null
  Variant::Type::Bool,      // epVT_Bool
  Variant::Type::Int,       // epVT_Int
  Variant::Type::Float,     // epVT_Float
  Variant::Type::Enum,      // epVT_Enum
  Variant::Type::Bitfield,  // epVT_Bitfield
  Variant::Type::SharedPtr, // epVT_SharedPtr
  Variant::Type::String,    // epVT_String
  Variant::Type::Array,     // epVT_Array

  // these get reinterpreted
  Variant::Type::String     // epVT_SmallString
};

} // namespace internal

// string constructors
Variant::Variant(String s, bool unsafeReference)
{
  static_assert(internal::VariantSmallStringSize <= 15, "Only 4 bits for length!");
  if (unsafeReference)
  {
    t = (size_t)Type::String;
    ownsContent = 0;
    length = s.length;
    this->s = s.ptr;
  }
  else if (s.length <= internal::VariantSmallStringSize)
  {
    // small string optimisation stashes short strings in the variant struct directly
    uint8_t *pArray = (uint8_t*)this;
    *pArray++ = (uint8_t)Type::SmallString | (uint8_t)(s.length << 4);
    memcpy((char*)pArray, s.ptr, s.length);
    if (s.length < internal::VariantSmallStringSize)
      pArray[s.length] = 0;
  }
  else
  {
    t = (size_t)Type::String;
    ownsContent = 1;
    length = s.length;
    char *pS = internal::SliceAlloc<char>(s.length + 1);
    this->s = pS;
    memcpy(pS, s.ptr, s.length);
    pS[s.length] = 0;
  }
}

// array constructors
Variant::Variant(Slice<Variant> a, bool unsafeReference)
{
  if (unsafeReference)
  {
    t = (size_t)Type::Array;
    ownsContent = 0;
    length = a.length;
    this->p = a.ptr;
  }
  else
  {
    t = (size_t)Type::Array;
    ownsContent = 1;
    length = a.length;
    Variant *pA = internal::SliceAlloc<Variant>(a.length);
    this->p = pA;
    for (size_t j = 0; j < length; ++j)
      new(&pA[j]) Variant(a.ptr[j]);
  }
}

// KVP constructors
Variant::Variant(Slice<KeyValuePair> aa)
  : Variant(VarMap(aa))
{}

void Variant::copyContent(const Variant &val)
{
  switch ((Type)t)
  {
    case Type::SharedPtr:
    {
      new((void*)&p) SharedPtr<RefCounted>((const SharedPtr<RefCounted>&)val.sp);
      break;
    }
    case Type::String:
    {
      ++internal::GetSliceHeader(s)->refCount;
      break;
    }
    case Type::Array:
    {
      ++internal::GetSliceHeader(p)->refCount;
      break;
    }
    default:
      break;
  }
}

void Variant::destroy()
{
  switch ((Type)t)
  {
    case Type::SharedPtr:
      ((SharedPtr<RefCounted>&)sp).~SharedPtr();
      break;
    case Type::String:
    {
      internal::SliceHeader *pH = internal::GetSliceHeader(s);
      if (pH->refCount == 1)
        internal::SliceFree(s);
      else
        --pH->refCount;
      break;
    }
    case Type::Array:
    {
      internal::SliceHeader *pH = internal::GetSliceHeader(p);
      if (pH->refCount == 1)
      {
        for (size_t j = 0; j < length; ++j)
          a[j].~Variant();
        internal::SliceFree(p);
      }
      else
        --pH->refCount;
      break;
    }
    case Type::SmallString:
      // SmallString may appear to have the ownsContent bit set, but it's actually a bit of the length ;)
      break;
    default:
    break;
  }
}

bool Variant::isNull() const
{
  switch ((Type)t)
  {
    case Type::Void:
      // TODO: consider, is this correct?
      EPTHROW_ERROR(epR_InvalidType, "Variant is void; has no value");
    case Type::Null:
      return true;
    case Type::String:
    case Type::Array:
      return length == 0;
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::AssocArray)
        return aa ? aa->tree.Empty() : true;
      else
        return sp == nullptr;
    case Type::SmallString:
      return (*(uint8_t*)this >> 4) == 0;
    default:
      return false;
  }
}

SharedString Variant::stringify() const
{
  switch ((Type)t)
  {
    case Type::Void:
      return "void";
    case Type::Null:
      return "nil";
    case Type::Bool:
      return b ? "true" : "false";
    case Type::String:
    case Type::SmallString:
      return asString();
    default:
      break;
  }
  return nullptr;
}

ptrdiff_t Variant::compare(const Variant &v) const
{
  Type x = internal::s_typeTranslation[t];
  Type y = internal::s_typeTranslation[v.t];
  if (x != y)
    return (ptrdiff_t)x - (ptrdiff_t)y;

  switch ((Type)t)
  {
    case Type::Void:
    case Type::Null:
      return 0;
    case Type::Bool:
      return (b ? 1 : 0) - (v.b ? 1 : 0);
    case Type::Int:
      return ptrdiff_t(i - v.i);
    case Type::Float:
      return f < v.f ? -1 : (f > v.f ? 1 : 0);
    case Type::String:
    case Type::SmallString:
      return asString().cmp(v.asString());
    case Type::SharedPtr:
    {
      switch ((SharedPtrType)length)
      {
        case SharedPtrType::Component:
          return c->uid.cmp(v.c->uid);
        default:
          return (char*)p - (char*)v.p;
      }
    }
    default:
      return (char*)p - (char*)v.p;
  }
}

bool Variant::asBool() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return false;
    case Type::Bool:
      return b ? true : false;
    case Type::Int:
      return !!i;
    case Type::Float:
      return f != 0;
    case Type::String:
    case Type::SmallString:
    {
      String str = asString();
      if (str.eqIC("true"))
        return true;
      else if (str.eqIC("false"))
        return false;
      return !str.empty();
    }
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
int64_t Variant::asInt() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return 0;
    case Type::Bool:
      return b ? 1 : 0;
    case Type::Int:
      return i;
    case Type::Float:
      return (int64_t)f;
    case Type::String:
      return String(s, length).parseInt();
    case Type::SmallString:
    {
      uint8_t *pBuffer = (uint8_t*)this;
      return String((char*)pBuffer + 1, pBuffer[0] >> 4).parseInt();
    }
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
double Variant::asFloat() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return 0.0;
    case Type::Bool:
      return b ? 1.0 : 0.0;
    case Type::Int:
      return (double)i;
    case Type::Float:
      return f;
    case Type::String:
      return String(s, length).parseFloat();
    case Type::SmallString:
    {
      uint8_t *pBuffer = (uint8_t*)this;
      return String((char*)pBuffer + 1, pBuffer[0] >> 4).parseFloat();
    }
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
const EnumDesc* Variant::asEnum(size_t *pVal) const
{
  if ((Type)t == Type::Void)
    EPTHROW_ERROR(epR_InvalidType, "Variant is void");
  else if ((Type)t == Type::Enum || (Type)t == Type::Bitfield)
  {
    *pVal = (ptrdiff_t)(length << 5) >> 5;
    return (const EnumDesc*)p;
  }
  return nullptr;
}
ComponentRef Variant::asComponent() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return ComponentRef();
    case Type::SharedPtr:
      if((SharedPtrType)length == SharedPtrType::Component)
        return (ComponentRef&)c;
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
Variant::VarDelegate Variant::asDelegate() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return VarDelegate();
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::Delegate)
        return VarDelegate((DelegateMementoRef&)d);
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
SubscriptionRef Variant::asSubscription() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return SubscriptionRef();
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::Subscription)
        return (SubscriptionRef&)d;
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
String Variant::asString() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return String();
    case Type::String:
      return String(s, length);
    case Type::SmallString:
    {
      uint8_t *pBuffer = (uint8_t*)this;
      return String((char*)pBuffer + 1, pBuffer[0] >> 4);
    }
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::Component)
        return c->GetUid();
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
SharedString Variant::asSharedString() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return String();
    case Type::Bool:
      return b ? "true" : "false";
    case Type::Int:
      return SharedString::format("{0}", i);
    case Type::Float:
      return SharedString::format("{0}", f);
    case Type::String:
    {
      String str(s, length);
      if (ownsContent)
        return (SharedString&)str;
      else
        return SharedString(str);
    }
    case Type::SmallString:
    {
      uint8_t *pBuffer = (uint8_t*)this;
      return SharedString((char*)pBuffer + 1, pBuffer[0] >> 4);
    }
    case Type::Array:
      return SharedString::format("{0}", asArray());
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::Component)
        return c->GetUid();
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}

Slice<Variant> Variant::asArray() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return Slice<Variant>();
    case Type::Array:
      return Slice<Variant>(a, length);
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
SharedArray<Variant> Variant::asSharedArray() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return SharedArray<Variant>();
    case Type::Array:
    {
      Slice<Variant> r(a, length);
      return (SharedArray<Variant>&)r;
    }
    case Type::SharedPtr:
      if((SharedPtrType)length == SharedPtrType::AssocArray)
      {
        // check the AA has a numeric series
        if(!aa)
          return SharedArray<Variant>();

        // does the series start at 0 or 1?
        size_t j = aa->tree.Get(0) ? 0 : 1;

        // append each item in the series to an array
        Array<Variant, 0> arr;
        do
        {
          Variant *pV = aa->tree.Get(j++);
          if (!pV)
            break;
          arr.pushBack(*pV);
        } while (1);
        return std::move(arr);
      }
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
    case Type::String:
      // TODO: should we parse strings that look like arrays??
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}
Variant::VarMap Variant::asAssocArray() const
{
  switch ((Type)t)
  {
    case Type::Void:
      EPTHROW_ERROR(epR_InvalidType, "Variant is void");
    case Type::Null:
      return VarMap();
    case Type::SharedPtr:
      if ((SharedPtrType)length == SharedPtrType::AssocArray)
        return (VarMap&)aa;
    default:
      EPTHROW_ERROR(epR_InvalidType, "Wrong type!");
  }
}

size_t Variant::arrayLen() const
{
  if (is(Type::Array))
    return length;
  else if (is(SharedPtrType::AssocArray))
    return assocArraySeriesLen();
  return 0;
}
size_t Variant::assocArraySeriesLen() const
{
  if (!is(SharedPtrType::AssocArray))
    return 0;

  // does the series start at 0 or 1?
  size_t j = aa->tree.Get(0) ? 0 : 1;

  size_t len = 0;
  while (aa->tree.Get(j++))
    ++len;
  return len;
}

Variant& Variant::operator[](size_t j) const
{
  if (is(Type::Array))
  {
    EPASSERT_THROW(j < length, epR_OutOfBounds, "Index out of range: {0} in [0 .. {1})", j, length);
    return a[j];
  }
  else if (is(SharedPtrType::AssocArray))
  {
    EPASSERT_THROW(j < length, epR_OutOfBounds, "Element not found: {0}", j);
    return aa->tree[j + (aa->tree.Get(0) ? 0 : 1)];
  }
  EPTHROW_ERROR(epR_InvalidType, "Invalid type!");
}
Variant& Variant::operator[](String key) const
{
  if (is(SharedPtrType::AssocArray))
    return aa->tree[key];
  EPTHROW_ERROR(epR_InvalidType, "Invalid type!");
}

Variant* Variant::getItem(String key) const
{
  if (is(SharedPtrType::AssocArray))
    return aa->tree.Get(key);
  return nullptr;
}

namespace internal
{
  AVLTreeAllocator<VariantAVLNode> &GetAVLTreeAllocator()
  {
    return *(AVLTreeAllocator<VariantAVLNode>*)s_pInstance->TreeAllocator();
  }
}

} // namespace ep
