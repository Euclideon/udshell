#pragma once
#ifndef EP_COMMAND_MANAGER_H
#define EP_COMMAND_MANAGER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/icommandmanager.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(CommandManager);
SHARED_CLASS(Activity);

class CommandManager : public Component
{
  EP_DECLARE_COMPONENT_WITH_IMPL(CommandManager, ICommandManager, Component, EPKERNEL_PLUGINVERSION, "Registers commands accessed by string id and associated with a function or script and an optional shortcut", 0)

public:
  bool RegisterCommand(String id, Delegate<void(Variant::VarMap)> func, String script, String activityTypeID, String shortcut = nullptr) { return pImpl->RegisterCommand(id, func, script, activityTypeID, shortcut); }
  void UnregisterCommand(String id) { pImpl->UnregisterCommand(id); }
  bool HandleShortcutEvent(String shortcut) { return pImpl->HandleShortcutEvent(shortcut); }
  bool RunCommand(String id, Variant::VarMap params = nullptr) { return pImpl->RunCommand(id, params); }
  void DisableShortcut(String commandID) { pImpl->DisableShortcut(commandID); }
  void EnableShortcut(String commandID) { pImpl->EnableShortcut(commandID); }
  bool SetFunction(String id, Delegate<void(Variant::VarMap)> func) { return pImpl->SetFunction(id, func); }
  bool SetScript(String id, String script) { return pImpl->SetScript(id, script); }
  SharedString GetShortcut(String id) const { return pImpl->GetShortcut(id); }
  bool SetShortcut(String id, SharedString shortcut) { return pImpl->SetShortcut(id, shortcut); }
  String GetActivityType(String commandID) const { return pImpl->GetActivityType(commandID); }
  bool SetActivityType(String commandID, String activityTypeID) { return pImpl->SetActivityType(commandID, activityTypeID); }

  Variant Save() const override { return pImpl->Save(); }

protected:
  CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

private:
  Array<const MethodInfo> GetMethods() const;
};

} //namespace ep

#endif // EP_COMMAND_MANAGER_H
