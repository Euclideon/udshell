#pragma once
#ifndef EP_COMMANDMANAGER_IMPL_H
#define EP_COMMANDMANAGER_IMPL_H

#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/internal/i/icommandmanager.h"
#include "ep/cpp/component/component.h"
#include "ep/cpp/delegate.h"

namespace ep {

class CommandManagerImpl : public BaseImpl<CommandManager, ICommandManager>
{
public:
  CommandManagerImpl(Component *pInstance, Variant::VarMap initParams)
    : ImplSuper(pInstance)
  {}

  bool RegisterCommand(String id, Delegate<void(Variant::VarMap)> func, String script, String activityTypeID, String shortcut = nullptr) override final;
  void UnregisterCommand(String id) override final;
  bool HandleShortcutEvent(String shortcut) override final;
  bool RunCommand(String id, Variant::VarMap params = nullptr) override final;
  bool SetFunction(String id, Delegate<void(Variant::VarMap)> func) override final;
  bool SetScript(String id, String script) override final;
  SharedString GetShortcut(String id) const override final;
  bool SetShortcut(String id, SharedString shortcut) override final;
  void DisableShortcut(String commandID) override final { SetShortcutEnabled(commandID, false); }
  void EnableShortcut(String commandID) override final { SetShortcutEnabled(commandID, true); }
  String GetActivityType(String commandID) const override final;
  bool SetActivityType(String commandID, String activityTypeID) override final;
  Variant Save() const override final { return pInstance->Super::save(); }

protected:
  String StripWhitespace(Slice<char> output, String input);
  void SetShortcutEnabled(String commandID, bool bEnabled);

  struct Command
  {
    Command(SharedString id, Delegate<void(Variant::VarMap)> func, SharedString script, SharedString activityType, SharedString shortcut)
      : id(id), shortcut(shortcut), func(func), script(script), activityType(activityType) {}

    SharedString id;
    SharedString shortcut;
    bool shortcutEnabled = true;
    Delegate<void(Variant::VarMap)> func;
    SharedString script;
    SharedString activityType;
  };

  AVLTree<SharedString, struct Command> commandRegistry;
};

} //namespace ep

#endif // EP_COMMANDMANAGER_IMPL_H
