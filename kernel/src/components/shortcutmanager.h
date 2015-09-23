#pragma once
#ifndef UD_SHORTCUT_MANAGER_H
#define UD_SHORTCUT_MANAGER_H

#include "component.h"
#include "ep/epdelegate.h"

namespace ud
{
PROTOTYPE_COMPONENT(ShortcutManager);

class ShortcutManager : public Component
{
public:
  UD_COMPONENT(ShortcutManager);

  bool SetShortcutString(udString id, udString shortcutString);
  udSharedString GetShortcutString(udString id) const;
  bool RegisterShortcut(udString id, udString shortcutString, bool bFailIfExists = false);
  void UnregisterShortcut(udString id);
  bool HandleShortcutEvent(udString shortcutString);
  bool SetShortcutFunction(udString id, udDelegate<void()> func);
  bool SetShortcutScript(udString id, udString script);

protected:
  ShortcutManager(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams);
  udString StripWhitespace(udSlice<char> output, udString input);

  struct Shortcut
  {
    Shortcut(udSharedString id, udSharedString shortcutString) : id(id), shortcutString(shortcutString) {}

    udSharedString id;
    udSharedString shortcutString;
    udSharedString script;
    udDelegate<void()> func;
  };

  udAVLTree<udSharedString, struct Shortcut> shortcutRegistry;
};

} //namespace ud
#endif // UD_SHORTCUT_MANAGER_H

