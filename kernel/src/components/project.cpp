#include "components/project.h"
#include "components/file.h"
#include "components/resourcemanager.h"
#include "components/activities/activity.h"
#include "components/resources/text.h"
#include "kernel.h"
#include "rapidxml.hpp"

namespace ep {

Project::Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
  : Component(pType, pKernel, uid, initParams)
{
  spResourceManager = pKernel->GetResourceManager();

  const Variant *pSrc = initParams.Get("src");
  StreamRef spSrc = nullptr;

  if (pSrc && pSrc->is(Variant::Type::String))
  {
    srcString = pSrc->asString();

    // path or url?
    spSrc = pKernel->CreateComponent<File>({ { "path", *pSrc }, { "flags", FileOpenFlags::Read | FileOpenFlags::Text } });
    if (!spSrc)
    {
      LogDebug(2, "Project file \"{0}\" does not exist. Creating new project.", *pSrc);
      return;
    }
  }
  else
  {
    LogDebug(3, "No \"src\" parameter. Creating empty project");
    return;
  }

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
    LogError("Unable to parse project file: {0} on line {1} : {2}", srcString, Text::GetLineNumberFromByteIndex(buffer, (size_t)(e.where<char>() - buffer.ptr)), e.what());
    throw epR_Failure;
  }

  Variant::VarMap projectNode = rootElements.asAssocArray();
  Variant *pName = projectNode.Get("name");
  if (pName && pName->is(Variant::Type::String) && pName->asString().eq("project")) // TODO: I think we can make these comparisons better than this
    ParseProject(projectNode);
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

  Variant::VarMap projectNode;
  Array<Variant> children;

  projectNode.Insert("name", "project");
  children.pushBack(SaveActivities());

  projectNode.Insert("children", children);

  spXMLBuffer->FormatXml(projectNode);

  Slice<const void> buffer = spXMLBuffer->MapForRead();
  if (buffer.empty())
  {
    LogDebug(1, "SaveProject() -- Can't Map XML buffer for reading");
    return;
  }

  if (srcString.empty())
  {
    LogWarning(1, "Failed to Save Project. No project open");
    return;
  }

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
  Variant::VarMap activitiesNode;
  Array<Variant> children;

  for (ActivityRef activity : activities)
  {
    Variant::VarMap node = Text::MapToXMLNode(activity->Save()).asAssocArray();
    node.Insert("name", activity->GetType());
    children.pushBack(node);
  }

  activitiesNode.Insert("name", "activities");
  activitiesNode.Insert("children", children);

  return activitiesNode;
}

void Project::ParseProject(Variant node)
{
  Variant::VarMap projectNode = node.asAssocArray();
  Variant *pChildren = projectNode.Get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Array<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::Type::AssocArray))
      {
        Variant *pName = child.asAssocArray().Get("name");
        if(pName && pName->is(Variant::Type::String) && pName->asString().eq("activities"))
          ParseActivities(child);
      }
    }
  }
}

void Project::ParseActivities(Variant node)
{
  Variant::VarMap projectNode = node.asAssocArray();
  Variant *pChildren = projectNode.Get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Array<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::Type::AssocArray))
        ParseActivity(child);
    }
  }
}

void Project::ParseActivity(Variant node)
{
  Variant::VarMap initParams;
  Variant::VarMap activityNode = node.asAssocArray();

  Variant *pName = activityNode.Get("name");
  if (!pName || !pName->is(Variant::Type::String))
    return;

  Variant vParams = Text::XMLNodeToMap(node);
  if (vParams.is(Variant::Type::AssocArray))
    initParams = vParams.asAssocArray();

  ep::ComponentRef c = nullptr;
  epResult r = pKernel->CreateComponent(pName->asString(), initParams, &c);
  if (r != epR_Success)
  {
    LogWarning(1, "Unable to load Activity from project file \"{0}\" -- Activity component \"{1}\" does not exist", srcString, pName->asString());
    return;
  }
  activities.pushBack(shared_pointer_cast<Activity>(c));
}

} // namespace ep
