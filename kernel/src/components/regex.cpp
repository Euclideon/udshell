#include "components/regex.h"

#define PCRE_STATIC
#define PCRE_CODE_UNIT_WIDTH 8
#include <pcre.h>

namespace ep {

Regex::Regex(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  Variant *pV = initParams.get("pattern");
  if (pV)
    CompilePattern(pV->asString());
}
Regex::~Regex()
{
  if (pExtra)
    pcre_free((pcre_extra*)pExtra);
  if (pCode)
    pcre_free((pcre*)pCode);
}

void Regex::CompilePattern(String pattern)
{
  if (pExtra)
  {
    pcre_free((pcre_extra*)pExtra);
    pExtra = nullptr;
  }
  if (pCode)
    pcre_free((pcre*)pCode);

  const char *pError;
  int errorcode, erroffset;
  pCode = pcre_compile2(pattern.toStringz(), PCRE_UTF8, &errorcode, &pError, &erroffset, nullptr);
  if (!pCode)
    EPTHROW_ERROR(Result::Failure, "Regex compile failed: {0}", pError);
  epscope(fail) { pcre_free((pcre*)pCode); pCode = nullptr; };

  pExtra = pcre_study((pcre*)pCode, 0, &pError);
  if (!pExtra)
    EPTHROW_ERROR(Result::Failure, "Optimising regex failed: {0}", pError);
}

Array<String> Regex::Match(String text)
{
  EPTHROW_IF(!pCode || !pExtra, Result::InvalidCall, "Call to match requires a valid regex pattern!");

  int subStrVec[30]; // TODO; how many??
  int matches = pcre_exec((pcre*)pCode, (pcre_extra*)pExtra, text.ptr, (int)text.length, 0, 0, subStrVec, sizeof(subStrVec)/sizeof(subStrVec[0]));
  if (matches < 0)
  {
    switch (matches)
    {
      case PCRE_ERROR_NOMATCH: // no match
        return nullptr;
      case PCRE_ERROR_NULL:         EPTHROW_ERROR(Result::InvalidArgument, "Something was null\n");       break;
      case PCRE_ERROR_BADOPTION:    EPTHROW_ERROR(Result::InvalidArgument, "A bad option was passed\n");  break;
      case PCRE_ERROR_BADMAGIC:
      case PCRE_ERROR_UNKNOWN_NODE: EPTHROW_ERROR(Result::Failure, "Error in compiled regex");            break;
      case PCRE_ERROR_NOMEMORY:     EPTHROW_ERROR(Result::AllocFailure, "Ran out of memory");             break;
      default:                      EPTHROW_ERROR(Result::Failure, "Unknown error");                      break;
    }
  }
  if (matches == 0)
  {
    EPASSERT(false, "TODO: Check me!");
    // not enough results??
    return nullptr;
  }

  Array<String, 0> r(Reserve, matches);
  for (int i = 0; i < matches; i++)
    r.pushBack(text.slice(subStrVec[2*i], subStrVec[2*i + 1]));
  return r;
}

} // namespace ep
