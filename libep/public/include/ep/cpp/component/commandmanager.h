#pragma once
#ifndef EP_COMMAND_MANAGER_H
#define EP_COMMAND_MANAGER_H

#include "ep/cpp/component/component.h"
#include "ep/cpp/internal/i/icommandmanager.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(CommandManager);
SHARED_CLASS(Activity);

class CommandManager : public Component, public ICommandManager
{
  EP_DECLARE_COMPONENT_WITH_IMPL(CommandManager, ICommandManager, Component, EPKERNEL_PLUGINVERSION, "Registers commands accessed by string id and associated with a function or script and an optional shortcut")

public:
  bool RegisterCommand(String id, Delegate<void(Variant::VarMap)> func, String script, String activityTypeID, String shortcut = nullptr) override final { return pImpl->RegisterCommand(id, func, script, activityTypeID, shortcut); }
  void UnregisterCommand(String id) override final { pImpl->UnregisterCommand(id); }
  bool HandleShortcutEvent(String shortcut) override final { return pImpl->HandleShortcutEvent(shortcut); }
  bool RunCommand(String id, Variant::VarMap params = nullptr) override final { return pImpl->RunCommand(id, params); }
  void DisableShortcut(String commandID) override final { pImpl->DisableShortcut(commandID); }
  void EnableShortcut(String commandID) override final { pImpl->EnableShortcut(commandID); }
  bool SetFunction(String id, Delegate<void(Variant::VarMap)> func) override final { return pImpl->SetFunction(id, func); }
  bool SetScript(String id, String script) override final { return pImpl->SetScript(id, script); }
  SharedString GetShortcut(String id) const override final { return pImpl->GetShortcut(id); }
  bool SetShortcut(String id, SharedString shortcut) override final { return pImpl->SetShortcut(id, shortcut); }
  String GetActivityType(String commandID) const override final { return pImpl->GetActivityType(commandID); }
  bool SetActivityType(String commandID, String activityTypeID) override final { return pImpl->SetActivityType(commandID, activityTypeID); }
  Variant Save() const override final { return pImpl->Save(); }

protected:
  CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Component(pType, pKernel, uid, initParams)
  {
    pImpl = CreateImpl(initParams);
  }

  static Array<const MethodInfo> GetMethods()
  {
    return {
      EP_MAKE_METHOD(SetShortcut, "Set the shortcut for the given command"),
      EP_MAKE_METHOD(GetShortcut, "Get the shortcut for the given command"),
      EP_MAKE_METHOD(RegisterCommand, "register a command and assign it a function, script and/or shortcut"),
      EP_MAKE_METHOD(UnregisterCommand, "remove a command from the command manager"),
      EP_MAKE_METHOD(HandleShortcutEvent, "execute the command attached to the given shortcut"),
      EP_MAKE_METHOD(RunCommand, "call the function or script attached to the given command"),
      EP_MAKE_METHOD(SetFunction, "Assign a callback function to the given command"),
      EP_MAKE_METHOD(SetScript, "Assign a script string to the given command"),
      EP_MAKE_METHOD(SetActivityType, "Set the activity type associated with the given command"),
      EP_MAKE_METHOD(GetActivityType, "Get the activity type associated with the given command"),
      EP_MAKE_METHOD(EnableShortcut, "Enable shortcut for the given command"),
      EP_MAKE_METHOD(DisableShortcut, "Disable shortcut for the given command"),
    };
  }
};

} //namespace ep

#endif // EP_COMMAND_MANAGER_H
