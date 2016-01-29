#include "components/commandmanagerimpl.h"
#include "components/project.h"
#include "ep/cpp/component/activity.h"
#include "kernel.h"

namespace ep {

SharedString CommandManagerImpl::GetShortcut(String id) const
{
  const Command *pCommand = commandRegistry.Get(id);
  if (!pCommand)
    return nullptr;
  return
    pCommand->shortcut;
}

bool CommandManagerImpl::SetShortcut(String id, SharedString shortcut)
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

bool CommandManagerImpl::RegisterCommand(String id, Delegate<void(Variant::VarMap)> func, String script, String activityTypeID, String shortcut)
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
String CommandManagerImpl::StripWhitespace(Slice<char> output, String input)
{
  size_t len = 0;
  for (size_t i = 0; i < input.length; i++)
  {
    if (!isspace(input[i]))
      output[len++] = input[i];
  }
  return output.slice(0, len);
}

bool CommandManagerImpl::RunCommand(String id, Variant::VarMap params)
{
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject = GetKernel()->FindComponent("project");
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
          params.Insert("activity", spActiveActivity);

          if (comm.func)
            comm.func(params);
          else if (!comm.script.empty())
            GetKernel()->Exec(comm.script);
          return true;
        }
      }
      else
      {
        if (comm.func)
          comm.func(params);
        else if (!comm.script.empty())
          GetKernel()->Exec(comm.script);
        return true;
      }
    }
  }

  return false;
}

void CommandManagerImpl::UnregisterCommand(String id)
{
  commandRegistry.Remove(id);
}

bool CommandManagerImpl::HandleShortcutEvent(String shortcut)
{
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject = GetKernel()->FindComponent("project");
  if (spProject)
    spActiveActivity = spProject->GetActiveActivity();

  Variant::VarMap params;

  for (auto kvp : commandRegistry)
  {
    Command &comm = kvp.value;

    if (!shortcut.cmpIC(comm.shortcut))
    {
      if (!comm.activityType.empty())
      {
        if (spActiveActivity && comm.activityType.eq(spActiveActivity->GetType()))
        {
          params.Insert("activity", spActiveActivity);

          if (comm.func)
            comm.func(params);
          else if (!comm.script.empty())
            GetKernel()->Exec(comm.script);
          return true;
        }
      }
      else
      {
        if (comm.func)
          comm.func(params);
        else if (!comm.script.empty())
          GetKernel()->Exec(comm.script);
        return true;
      }
    }
  }

  return false;
}

bool CommandManagerImpl::SetFunction(String id, Delegate<void(Variant::VarMap)> func)
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

bool CommandManagerImpl::SetScript(String id, String script)
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

String CommandManagerImpl::GetActivityType(String commandID) const
{
  const Command *pCommand = commandRegistry.Get(commandID);
  if (!pCommand)
    return nullptr;

  return pCommand->activityType;
}

bool CommandManagerImpl::SetActivityType(String commandID, String activityTypeID)
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
