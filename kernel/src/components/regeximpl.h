#pragma once
#ifndef EPREGEXIMPL_H
#define EPREGEXIMPL_H

#include "ep/cpp/component/regex.h"
#include "ep/cpp/internal/i/iregex.h"

#include "ep/cpp/component/component.h"

namespace ep {

SHARED_CLASS(Regex);

class RegexImpl : public BaseImpl<Regex, IRegex>
{
public:
  RegexImpl(Component *pInstance, Variant::VarMap initParams);
  ~RegexImpl();

  virtual void CompilePattern(String pattern) override final;
  virtual Array<String> Match(String text) override final;

protected:
  void *pCode = nullptr;
  void *pExtra = nullptr;
};

}

#endif // EPREGEXIMPL_H
