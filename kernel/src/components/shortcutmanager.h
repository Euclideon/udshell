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
public:
  EP_COMPONENT(ShortcutManager);

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
};

} //namespace kernel

#endif // EP_SHORTCUT_MANAGER_H
