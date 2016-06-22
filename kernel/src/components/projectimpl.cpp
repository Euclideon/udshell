#include "components/projectimpl.h"
#include "ep/cpp/component/file.h"
#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/component/activity.h"
#include "ep/cpp/component/resource/text.h"
#include "rapidxml.hpp"

namespace ep {

Array<const PropertyInfo> Project::GetProperties() const
{
  return{
    EP_MAKE_PROPERTY(ActiveActivity, "The currently active activity", nullptr, 0),
    EP_MAKE_PROPERTY_RO(Activities, "The activities contained in the project", nullptr, 0),
    EP_MAKE_PROPERTY(Src, "The URL for the Project file", nullptr, 0)
  };
}

Array<const MethodInfo> Project::GetMethods() const
{
  return{
    EP_MAKE_METHOD(SaveProject, "Save Project to an XML file"),
    EP_MAKE_METHOD(AddActivity, "Add an Activity to the Project"),
    EP_MAKE_METHOD(RemoveActivity, "Remove an Activity from the Project"),
  };
}

ProjectImpl::ProjectImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  spResourceManager = GetKernel()->GetResourceManager();

  const Variant *pSrc = initParams.get("src");
  StreamRef spSrc = nullptr;

  if (pSrc && pSrc->is(Variant::Type::String))
  {
    spSrc = GetKernel()->CreateComponent<File>({ { "path", *pSrc },{ "flags", FileOpenFlags::Read | FileOpenFlags::Text } });
  }
  else
  {
    LogDebug(3, "No \"src\" parameter. Creating empty project");
    return;
  }

  TextRef spXMLBuffer = spSrc->LoadText();

  using namespace rapidxml;
  Variant rootElements;

  try
  {
    rootElements = spXMLBuffer->ParseXml();
  }
  catch (parse_error &e)
  {
    auto xmlBuff = spXMLBuffer->MapForRead();
    epscope(exit) { spXMLBuffer->Unmap(); };
    EPTHROW_ERROR(Result::Failure, "Unable to parse project file: {0} on line {1} : {2}", srcString, Text::GetLineNumberFromByteIndex(xmlBuff, (size_t)(e.where<char>() - xmlBuff.ptr)), e.what());
  }

  Variant::VarMap projectNode = rootElements.asAssocArray();
  Variant *pName = projectNode.get("name");
  if (pName && pName->is(Variant::Type::String) && pName->asString().eq("project")) // TODO: I think we can make these comparisons better than this
    ParseProject(projectNode);
  else
    EPTHROW_ERROR(Result::Failure, "Invalid project file \"{0}\" -- Missing <project> element", srcString);
}

void ProjectImpl::SaveProject()
{
  auto spXMLBuffer = GetKernel()->CreateComponent<Text>();
  spXMLBuffer->Reserve(10240);

  Variant::VarMap projectNode;
  Array<Variant> children;

  projectNode.insert("name", "project");
  children.pushBack(SaveActivities());

  projectNode.insert("children", children);

  spXMLBuffer->FormatXml(projectNode);

  Slice<const void> buffer = spXMLBuffer->MapForRead();
  epscope(exit) { spXMLBuffer->Unmap(); };
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

  try
  {
    StreamRef spFile = GetKernel()->CreateComponent<File>({ { "path", String(srcString) },{ "flags", FileOpenFlags::Create | FileOpenFlags::Write | FileOpenFlags::Text } });
    spFile->Write(buffer);
  }
  catch (EPException &)
  {
    LogWarning(1, "Failed to open Project file for writing: \"{0}\"", srcString);
    return;
  }
}

Variant ProjectImpl::SaveActivities()
{
  Variant::VarMap activitiesNode;
  Array<Variant> children;

  for (ActivityRef activity : activities)
  {
    Variant::VarMap node = Text::ComponentParamsToXMLMap(activity->Save()).asAssocArray();
    node.insert("name", activity->GetType());
    children.pushBack(node);
  }

  activitiesNode.insert("name", "activities");
  activitiesNode.insert("children", children);

  return activitiesNode;
}

void ProjectImpl::ParseProject(Variant node)
{
  Variant::VarMap projectNode = node.asAssocArray();
  Variant *pChildren = projectNode.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Array<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
      {
        Variant *pName = child.getItem("name");
        if(pName && pName->is(Variant::Type::String) && pName->asString().eq("activities"))
          ParseActivities(child);
      }
    }
  }
}

void ProjectImpl::ParseActivities(Variant node)
{
  Variant::VarMap projectNode = node.asAssocArray();
  Variant *pChildren = projectNode.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Array<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
        ParseActivity(child);
    }
  }
}

void ProjectImpl::ParseActivity(Variant node)
{
  try
  {
    Variant::VarMap initParams;
    Variant vParams = Text::XMLMapToComponentParams(node);
    if (vParams.is(Variant::SharedPtrType::AssocArray))
      initParams = vParams.asAssocArray();

    activities.pushBack(component_cast<Activity>(GetKernel()->CreateComponent(node["name"].asString(), initParams)));
  }
  catch (EPException &)
  {
    LogError("Unable to load Activity from project file \"{0}\"", srcString);
  }
}

} // namespace ep
