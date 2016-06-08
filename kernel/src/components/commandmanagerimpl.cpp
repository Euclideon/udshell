#include "components/commandmanagerimpl.h"
#include "ep/cpp/component/project.h"
#include "ep/cpp/component/activity.h"
#include "ep/cpp/kernel.h"

namespace ep {

Array<const MethodInfo> CommandManager::GetMethods() const
{
  return{
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

SharedString CommandManagerImpl::GetShortcut(String id) const
{
  const Command *pCommand = commandRegistry.get(id);
  if (!pCommand)
    return nullptr;
  return
    pCommand->shortcut;
}

bool CommandManagerImpl::SetShortcut(String id, SharedString shortcut)
{
  Command *pCommand = commandRegistry.get(id);
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
  if (commandRegistry.get(id))
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

  commandRegistry.insert(id, Command(id, func, script, activityTypeID, mShortcut));

  return true;
}

// TODO: remove this; slice/array should have filtering functions instead
String CommandManagerImpl::StripWhitespace(Slice<char> output, String input)
{
  size_t len = 0;
  for (size_t i = 0; i < input.length; i++)
  {
    if (!epIsWhitespace(input[i]))
      output[len++] = input[i];
  }
  return output.slice(0, len);
}

void CommandManagerImpl::SetShortcutEnabled(String commandID, bool bEnabled)
{
  for (auto kvp : commandRegistry)
  {
    if (!commandID.cmpIC(kvp.key))
    {
      Command &comm = kvp.value;
      comm.shortcutEnabled = bEnabled;
      break;
    }
  }
}

bool CommandManagerImpl::RunCommand(String id, Variant::VarMap params)
{
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject;

  ComponentRef spComp = GetKernel()->FindComponent("project");
  if (spComp)
  {
    spProject = component_cast<Project>(spComp);
    spActiveActivity = spProject->GetActiveActivity();
  }

  for (auto kvp : commandRegistry)
  {
    if (!id.cmpIC(kvp.key))
    {
      Command &comm = kvp.value;

      if (!comm.activityType.empty())
      {
        if (spActiveActivity && comm.activityType.eq(spActiveActivity->GetType()))
        {
          params.insert("activity", spActiveActivity);

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
  commandRegistry.remove(id);
}

bool CommandManagerImpl::HandleShortcutEvent(String shortcut)
{
  ActivityRef spActiveActivity = nullptr;
  ProjectRef spProject;

  ComponentRef spComp = GetKernel()->FindComponent("project");
  if (spComp)
  {
    spProject = component_cast<Project>(spComp);
    spActiveActivity = spProject->GetActiveActivity();
  }

  Variant::VarMap params;

  for (auto kvp : commandRegistry)
  {
    Command &comm = kvp.value;

    if (!shortcut.cmpIC(comm.shortcut) && comm.shortcutEnabled)
    {
      if (!comm.activityType.empty())
      {
        if (spActiveActivity && comm.activityType.eq(spActiveActivity->GetType()))
        {
          params.insert("activity", spActiveActivity);

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
  Command *pCommand = commandRegistry.get(id);
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
  Command *pCommand = commandRegistry.get(id);
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
  const Command *pCommand = commandRegistry.get(commandID);
  if (!pCommand)
    return nullptr;

  return pCommand->activityType;
}

bool CommandManagerImpl::SetActivityType(String commandID, String activityTypeID)
{
  Command *pCommand = commandRegistry.get(commandID);
  if (!pCommand)
  {
    LogWarning(2, "Can't bind activity type to command \"{0}\", command doesn't exist", commandID);
    return false;
  }

  pCommand->activityType = activityTypeID;

  return true;
}

} // namespace ep
