#if defined(__cplusplus)
extern "C" {
#endif

inline epVariant epVariant_CreateNull()
{
  epVariant v = { epVT_Null, 0, 0 };
  return v;
}
inline epVariant epVariant_CreateBool(char b)
{
  epVariant v = { epVT_Bool, 0, 0 };
  v.b = b;
  return v;
}
inline epVariant epVariant_CreateInt(int64_t i)
{
  epVariant v = { epVT_Int, 0, 0 };
  v.i = i;
  return v;
}
inline epVariant epVariant_CreateFloat(double f)
{
  epVariant v = { epVT_Float, 0, 0 };
  v.f = f;
  return v;
}
inline epVariant epVariant_CreateComponent(epComponent * epUnusedParam(pComponent))
{
}
//inline epVariant epVariant_CreateDelegate() {}
inline epVariant epVariant_CreateCString(const char *epUnusedParam(pString))
{
}
inline epVariant epVariant_CreateString(epString epUnusedParam(string))
{
}

inline epVariantType epVariant_GetType(epVariant v)
{
  return (epVariantType)v.t;
}

#if defined(__cplusplus)
}
#endif
