#pragma once
#ifndef EP_COMMAND_MANAGER_H
#define EP_COMMAND_MANAGER_H

#include "ep/cpp/component.h"
#include "ep/cpp/delegate.h"

namespace ep {

SHARED_CLASS(CommandManager);
SHARED_CLASS(Activity);

class CommandManager : public Component
{
  EP_DECLARE_COMPONENT(CommandManager, Component, EPKERNEL_PLUGINVERSION, "Registers commands accessed by string id and associated with a function or script and an optional shortcut")
public:

  bool RegisterCommand(String id, Delegate<void(ActivityRef)> func, String script, String activityTypeID, String shortcut = nullptr);
  void UnregisterCommand(String id);
  bool HandleShortcutEvent(String shortcut);
  bool RunCommand(String id);
  bool SetFunction(String id, Delegate<void(ActivityRef)> func);
  bool SetScript(String id, String script);
  SharedString GetShortcut(String id) const;
  bool SetShortcut(String id, SharedString shortcut);
  String GetActivityType(String commandID) const;
  bool SetActivityType(String commandID, String activityTypeID);

protected:
  CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams);
  String StripWhitespace(Slice<char> output, String input);

  struct Command
  {
    Command(SharedString id, Delegate<void(ActivityRef)> func, SharedString script, SharedString activityType, SharedString shortcut) : id(id), activityType(activityType), shortcut(shortcut), func(func), script(script) {}

    SharedString id;
    SharedString shortcut;
    Delegate<void(ActivityRef)> func;
    SharedString script;
    SharedString activityType;
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
      EP_MAKE_METHOD(SetActivityType, "Set the activity type associated with the given command"),
      EP_MAKE_METHOD(GetActivityType, "Get the activity type associated with the given command"),
    };
  }
};

} //namespace ep

#endif // EP_COMMAND_MANAGER_H
