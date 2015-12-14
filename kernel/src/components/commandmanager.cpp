#include "components/commandmanager.h"
#include "components/project.h"
#include "components/activities/activity.h"
#include "kernel.h"

namespace ep {

CommandManager::CommandManager(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{}

SharedString CommandManager::GetShortcut(String id) const
{
  const Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
    return nullptr;
  return
    pCommand->shortcut;
}

bool CommandManager::SetShortcut(String id, SharedString shortcut)
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

bool CommandManager::RegisterCommand(String id, Delegate<void(ActivityRef)> func, String script, String activityTypeID, String shortcut)
{
  if (commandRegistry.Get(id))
  {
    LogWarning(5, "Command registration failed - \"{0}\" already exists", id);
    return false;
  }

  MutableString<256> mShortcut(Reserve, shortcut.length);
  mShortcut.length = shortcut.length;
  StripWhitespace(mShortcut, shortcut);

  if (!shortcut.empty())
  {
    for (auto kvp : commandRegistry)
    {
      Command &comm = kvp.value;

      if (!shortcut.cmpIC(comm.shortcut) && id.cmp(comm.id)
        && (activityTypeID.empty() || comm.activityType.empty() || activityTypeID.eq(comm.activityType)))
      {
        LogWarning(2, "Can't bind shortcut \"{0}\" to command \"{1}\". Already bound to \"{2}\"", shortcut, id, comm.id);
        return false;
      }
    }
  }

  commandRegistry.Insert(id, Command(id, func, script, activityTypeID, mShortcut));

  return true;
}

// TODO: remove this; slice/array should have filtering functions instead
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
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject = pKernel->FindComponent("project");
  if (spProject)
    spActiveActivity = spProject->GetActiveActivity();

  for (auto kvp : commandRegistry)
  {
    if (!id.cmpIC(kvp.key))
    {
      Command &comm = kvp.value;

      if (!comm.activityType.empty())
      {
        if (spActiveActivity && comm.activityType.eq(spActiveActivity->GetType()))
        {
          if (comm.func)
            comm.func(spActiveActivity);
          else if (!comm.script.empty())
            pKernel->Exec(comm.script);
          return true;
        }
      }
      else
      {
        if (comm.func)
          comm.func(nullptr);
        else if (!comm.script.empty())
          pKernel->Exec(comm.script);
        return true;
      }
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
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject = pKernel->FindComponent("project");
  if (spProject)
    spActiveActivity = spProject->GetActiveActivity();

  for (auto kvp : commandRegistry)
  {
    Command &comm = kvp.value;

    if (!shortcut.cmpIC(comm.shortcut))
    {
      if (!comm.activityType.empty())
      {
        if (spActiveActivity && comm.activityType.eq(spActiveActivity->GetType()))
        {
          if (comm.func)
            comm.func(spActiveActivity);
          else if (!comm.script.empty())
            pKernel->Exec(comm.script);
          return true;
        }
      }
      else
      {
        if (comm.func)
          comm.func(nullptr);
        else if (!comm.script.empty())
          pKernel->Exec(comm.script);
        return true;
      }
    }
  }

  return false;
}

bool CommandManager::SetFunction(String id, Delegate<void(ActivityRef)> func)
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

String CommandManager::GetActivityType(String commandID) const
{
  const Command *pCommand = commandRegistry.Get(commandID);
  if (!pCommand)
    return nullptr;

  return pCommand->activityType;
}

bool CommandManager::SetActivityType(String commandID, String activityTypeID)
{
  Command *pCommand = commandRegistry.Get(commandID);
  if (!pCommand)
  {
    LogWarning(2, "Can't bind activity type to command \"{0}\", command doesn't exist", commandID);
    return false;
  }

  pCommand->activityType = activityTypeID;

  return true;
}

} // namespace ep
