#pragma once
#ifndef EP_SHORTCUT_MANAGER_H
#define EP_SHORTCUT_MANAGER_H

#include "component.h"
#include "ep/cpp/delegate.h"

namespace kernel
{

PROTOTYPE_COMPONENT(ShortcutManager);

class ShortcutManager : public Component
{
  EP_DECLARE_COMPONENT(ShortcutManager, Component, EPKERNEL_PLUGINVERSION, "Registers keyboard shortcuts and handles shortcut events")
public:

  bool SetShortcutString(String id, String shortcutString);
  SharedString GetShortcutString(String id) const;
  bool RegisterShortcut(String id, String shortcutString, bool bFailIfExists = false);
  void UnregisterShortcut(String id);
  bool HandleShortcutEvent(String shortcutString);
  bool SetShortcutFunction(String id, Delegate<void()> func);
  bool SetShortcutScript(String id, String script);

protected:
  ShortcutManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams);
  String StripWhitespace(Slice<char> output, String input);

  struct Shortcut
  {
    Shortcut(SharedString id, SharedString shortcutString) : id(id), shortcutString(shortcutString) {}

    SharedString id;
    SharedString shortcutString;
    SharedString script;
    Delegate<void()> func;
  };

  AVLTree<SharedString, struct Shortcut> shortcutRegistry;

  static Array<const MethodInfo> GetMethods()
  {
    return{
      EP_MAKE_METHOD(SetShortcutString, "Set the shortcut string for the given operation id"),
      EP_MAKE_METHOD(RegisterShortcut, "Register an operation and assign it a shortcut string"),
      EP_MAKE_METHOD(UnregisterShortcut, "Remove an operation from the shortcut manager"),
      EP_MAKE_METHOD(HandleShortcutEvent, "Call the function or script attached to the given shortcut string"),
      EP_MAKE_METHOD(SetShortcutFunction, "Assign a callback function to the given shortcut operation"),
      EP_MAKE_METHOD(SetShortcutScript, "Assign a script string to the given shortcut operation"),
    };
  }
};

} //namespace kernel

#endif // EP_SHORTCUT_MANAGER_H
