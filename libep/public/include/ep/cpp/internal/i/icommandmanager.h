#pragma once
#if !defined(_EP_ICOMMANDMANAGER_HPP)
#define _EP_ICOMMANDMANAGER_HPP

#include "ep/cpp/variant.h"

namespace ep {

class ICommandManager
{
public:
  virtual bool RegisterCommand(String id, Delegate<void(Variant::VarMap)> func, String script, String activityTypeID, String shortcut = nullptr) = 0;
  virtual void UnregisterCommand(String id) = 0;
  virtual bool HandleShortcutEvent(String shortcut) = 0;
  virtual bool RunCommand(String id, Variant::VarMap params = nullptr) = 0;
  virtual bool SetFunction(String id, Delegate<void(Variant::VarMap)> func) = 0;
  virtual bool SetScript(String id, String script) = 0;
  virtual SharedString GetShortcut(String id) const = 0;
  virtual bool SetShortcut(String id, SharedString shortcut) = 0;
  virtual String GetActivityType(String commandID) const = 0;
  virtual bool SetActivityType(String commandID, String activityTypeID) = 0;
  virtual Variant Save() const = 0;
};

} //namespace ep

#endif // EP_ICOMMANDMANAGER_H
