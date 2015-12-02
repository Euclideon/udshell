#include "components/project.h"
#include "components/file.h"
#include "components/resourcemanager.h"
#include "components/activities/activity.h"
#include "components/resources/text.h"
#include "kernel.h"
#include "rapidxml.hpp"

namespace kernel {

Project::Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  spResourceManager = pKernel->GetResourceManager();

  const Variant &src = initParams["src"];
  StreamRef spSrc = nullptr;

  if (src.is(Variant::Type::String))
  {
    srcString = src.asString();

    // path or url?
    spSrc = pKernel->CreateComponent<File>({ { "path", src }, { "flags", FileOpenFlags::Read | FileOpenFlags::Text } });
    if (!spSrc)
    {
      LogDebug(2, "Project file \"{0}\" does not exist. Creating new project.", src);
      return;
    }
  }
  else
    return; // Create empty project

  size_t len = (size_t)spSrc->Length() + 1;
  Array<char> buffer(Reserve, len);
  buffer.length = spSrc->Read(buffer.getBuffer()).length;
  buffer.pushBack('\0');

  auto spXMLBuffer = pKernel->CreateComponent<Text>();
  spXMLBuffer->CopyBuffer(buffer);

  using namespace rapidxml;
  Variant rootElements;

  try
  {
    rootElements = spXMLBuffer->ParseXml();
  }
  catch (parse_error e)
  {
    LogError("Unable to parse project file: {0} on line {1} : {2}", srcString, GetLineNumberFromByteIndex(buffer, (size_t)(e.where<char>() - buffer.ptr)), e.what());
    throw epR_Failure;
  }

  Variant::VarMap kvps = rootElements.asAssocArray();
  Variant *pProject = kvps.Get("project");
  if (pProject)
    ParseProject(*pProject);
  else
  {
    LogError("Invalid project file \"{0}\" -- Missing <project> element", srcString);
    throw epR_Failure;
  }
}

void Project::SaveProject()
{
  auto spXMLBuffer = GetKernel().CreateComponent<Text>();
  spXMLBuffer->Reserve(10240);

  Array<KeyValuePair> projectValues;
  projectValues.pushBack(KeyValuePair(String("activities"), SaveActivities()));
  Array<KeyValuePair> rootValues;
  rootValues.pushBack(KeyValuePair(String("project"), projectValues));
  spXMLBuffer->FormatXml(rootValues);

  Slice<const void> buffer = spXMLBuffer->MapForRead();
  if (buffer.empty())
  {
    LogDebug(1, "SaveProject() -- Can't Map XML buffer for reading");
    return;
  }

  // TODO One day this will pop up a dialog requesting a name for the project
  if (srcString.empty())
    srcString = "project0.epproj";

  StreamRef spFile = GetKernel().CreateComponent<File>({ { "path", String(srcString) }, { "flags", FileOpenFlags::Create | FileOpenFlags::Write | FileOpenFlags::Text } });
  if (!spFile)
  {
    LogWarning(1, "Failed to open Project file for writing: \"{0}\"", srcString);
    return;
  }

  spFile->Write(buffer);
}

Variant Project::SaveActivities()
{
  Array<KeyValuePair> values;

  for(ActivityRef activity : activities)
    values.pushBack(KeyValuePair(activity->GetType(), activity->Save()));

  return values;
}

void Project::ParseProject(Variant values)
{
  Variant::VarMap kvps = values.asAssocArray();
  if (kvps.Empty())
    return;

  for (auto kvp : kvps)
  {
    if (kvp.key.asString().eq("activities"))
      ParseActivities(kvp.value);
  }
}

void Project::ParseActivities(Variant values)
{
  Variant::VarMap kvps = values.asAssocArray();
  if (kvps.Empty())
    return;

  for (auto kvp : kvps)
    ParseActivity(kvp.key.asString(), kvp.value);
}

void Project::ParseActivity(String type, Variant values)
{
  Variant::VarMap kvps = values.asAssocArray();

  ep::ComponentRef c = nullptr;
  epResult r = pKernel->CreateComponent(type, Variant::VarMap(kvps), &c);
  if (r != epR_Success)
  {
    LogWarning(1, "Unable to load Activity from project file \"{0}\" -- Activity component \"{1}\" does not exist", srcString, type);
    return;
  }
  activities.pushBack(shared_pointer_cast<Activity>(c));
}


uint32_t Project::GetLineNumberFromByteIndex(Slice<char> buffer, size_t index) const
{
  int lineNumber = 0;

  for (size_t i = 0; i < buffer.length && i <= index; i++)
  {
    if (buffer[i] == '\n')
      lineNumber++;
  }

  return lineNumber;
}

} // namespace kernel
