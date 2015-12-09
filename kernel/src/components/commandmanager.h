#pragma once
#ifndef EP_COMMAND_MANAGER_H
#define EP_COMMAND_MANAGER_H

#include "component.h"
#include "ep/cpp/delegate.h"

namespace kernel
{

PROTOTYPE_COMPONENT(CommandManager);

class CommandManager : public Component
{
  EP_DECLARE_COMPONENT(CommandManager, Component, EPKERNEL_PLUGINVERSION, "Registers commands accessed by string id and associated with a function or script and an optional shortcut")
public:

  bool RegisterCommand(String id, Delegate<void()> func, SharedString script, SharedString shortcut = nullptr, bool bFailIfExists = false);
  void UnregisterCommand(String id);
  bool HandleShortcutEvent(String shortcut);
  bool RunCommand(String id);
  bool SetFunction(String id, Delegate<void()> func);
  bool SetScript(String id, String script);
  SharedString GetShortcut(String id) const;
  bool SetShortcut(String id, SharedString shortcut);

protected:
  CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  String StripWhitespace(Slice<char> output, String input);

  struct Command
  {
    Command(SharedString id, Delegate<void()> func, SharedString script, SharedString shortcut) : id(id), shortcut(shortcut), func(func), script(script) {}

    SharedString id;
    SharedString shortcut;
    Delegate<void()> func;
    SharedString script;
  };

  AVLTree<SharedString, struct Command> commandRegistry;

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
    };
  }
};

} //namespace kernel

#endif // EP_COMMAND_MANAGER_H
