#include "components/commandmanager.h"
#include "kernel.h"

namespace kernel {

CommandManager::CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{}

String CommandManager::GetShortcut(String id) const
{
  const Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
    return nullptr;
  return
    pCommand->shortcut;
}

bool CommandManager::SetShortcut(String id, String shortcut)
{
  Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
  {
    LogWarning(2, "Can't bind shortcut \"{0}\" to command \"{1}\" - \"{1}\" doesn't exist", shortcut, id);
    return false;
  }

  pCommand->shortcut = shortcut;

  return true;
}

bool CommandManager::RegisterCommand(String id, Delegate<void()> func, String script, String shortcut, bool bFailIfExists)
{
  if (bFailIfExists && commandRegistry.Get(id))
  {
    LogWarning(5, "Command registration failed - \"{0}\" already exists", id);
    return false;
  }

  MutableString<256> mShortcut(Reserve, shortcut.length);
  mShortcut.length = shortcut.length;
  StripWhitespace(mShortcut, shortcut);

  if (!shortcut.empty())
  {
    for (auto comm : commandRegistry)
    {
      if (!shortcut.cmpIC(comm.value.shortcut) && id.cmp(comm.value.id))
      {
        LogWarning(2, "Can't bind shortcut \"{0}\" to command \"{1}\". Already bound to \"{2}\"", shortcut, id, comm.value.id);
        return false;
      }
    }
  }

  commandRegistry.Insert(id, Command(id, func, script, mShortcut));

  return true;
}

String CommandManager::StripWhitespace(Slice<char> output, String input)
{
  size_t len = 0;
  for (size_t i = 0; i < input.length; i++)
  {
    if (!isspace(input[i]))
      output[len++] = input[i];
  }
  return output.slice(0, len);
}

bool CommandManager::RunCommand(String id)
{
  for (Command &comm : commandRegistry)
  {
    if (!id.cmpIC(comm.id))
    {
      if (comm.func)
        comm.func();
      else if (!comm.script.empty())
        pKernel->Exec(comm.script);

      return true;
    }
  }

  return false;
}

void CommandManager::UnregisterCommand(String id)
{
  commandRegistry.Remove(id);
}

bool CommandManager::HandleShortcutEvent(String shortcut)
{
  for (auto comm : commandRegistry)
  {
    if (!shortcut.cmpIC(comm.value.shortcut))
    {
      if (comm.value.func)
        comm.value.func();
      else if (!comm.value.script.empty())
        pKernel->Exec(comm.value.script);

      return true;
    }
  }

  return false;
}

bool CommandManager::SetFunction(String id, Delegate<void()> func)
{
  Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
  {
    LogWarning(2, "Can't bind function to command \"{0}\", command doesn't exist", id);
    return false;
  }

  pCommand->func = func;
  pCommand->script = nullptr;

  return true;
}

bool CommandManager::SetScript(String id, String script)
{
  Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
  {
    LogWarning(2, "Can't bind script to command \"{0}\", command doesn't exist", id);
    return false;
  }

  pCommand->script = script;
  pCommand->func = nullptr;

  return true;
}



} // namespace kernel
