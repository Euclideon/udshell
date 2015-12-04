#if !defined(EPVARIANT_H)
#define EPVARIANT_H

#include "ep/c/platform.h"
#include "ep/epenum.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct epComponent;

enum epVariantType
{
  epVT_Null,
  epVT_Bool,
  epVT_Int,
  epVT_Float,
  epVT_Enum,
  epVT_Bitfield,
  epVT_Component,
  epVT_Delegate,
  epVT_String,
  epVT_Array,
  epVT_AssocArray,
  epVT_Void,
  epVT_SmallString
};

struct epVariant
{
  size_t t : 4;
  size_t ownsContent : 1;
  size_t length : (sizeof(size_t)*8)-5; // NOTE: if you change this, update the shift's in asEnum()!!!
  union
  {
    char b;
    int64_t i;
    double f;
    const char *s;
    void *p;
  };
};

struct epKeyValuePair
{
  epVariant key;
  epVariant value;
};

struct epVarMap;


void epVariant_Release(epVariant v);

epVariant epVariant_CreateVoid();
epVariant epVariant_CreateNull();
epVariant epVariant_CreateBool(char b);
epVariant epVariant_CreateInt(int64_t i);
epVariant epVariant_CreateFloat(double f);
epVariant epVariant_CreateComponent(epComponent *pComponent);
//epVariant epVariant_CreateDelegate();
epVariant epVariant_CreateCString(const char *pString);
epVariant epVariant_CreateString(epString string);

epVariantType epVariant_GetType(epVariant v);

int          epVariant_IsVoid(epVariant v);
int          epVariant_IsNull(epVariant v);
char         epVariant_AsBool(epVariant v);
int64_t      epVariant_AsInt(epVariant v);
double       epVariant_AsFloat(epVariant v);
epComponent* epVariant_AsComponent(epVariant v);
//char       epVariant_AsDelegate(epVariant v);
epString     epVariant_AsString(epVariant v);
const epVariant* epVariant_AsArray(epVariant v, size_t *pLength);
const epVarMap* epVariant_AsAssocArray(epVariant v);

#if defined(__cplusplus)
}
#endif

#endif // EPVARIANT_H
