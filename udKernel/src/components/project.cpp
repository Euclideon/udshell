#include "components/project.h"
#include "components/file.h"
#include "components/resourcemanager.h"
#include "kernel.h"
#include "rapidxml.hpp"

namespace ud
{

ComponentDesc Project::descriptor =
{
  &Component::descriptor, // pSuperDesc

  UDSHELL_APIVERSION, // udVersion
  UDSHELL_PLUGINVERSION, // pluginVersion

  "project", // id
  "Project", // displayName
  "Contains the state of a project", // description

  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

Project::Project(const ComponentDesc *pType, Kernel *pKernel, udSharedString uid, udInitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  const udVariant &src = initParams["src"];
  StreamRef spSrc = nullptr;
  ResourceManagerRef spResourceManager = pKernel->GetResourceManager();

  if (src.is(udVariant::Type::String))
  {
    // path or url?
    spSrc = pKernel->CreateComponent<File>({ { "path", src }, { "flags", FileOpenFlags::Read } });
    if (!spSrc)
    {
      LogWarning(5, "\"src\" file path not found: {0}", src);
      throw udR_File_OpenFailure;
    }
  }
  else
  {
    LogError("Missing or invalid parameter \"src\" when creating Project");
    throw udR_InvalidParameter_;
  }

  int64_t len = spSrc->Length();
  char *pBuffer = (char *)udAlloc(len + 1);
  spSrc->Read(pBuffer, len);
  pBuffer[len] = '\0';

  using namespace rapidxml;

  try
  {
    xml_document<> doc;
    doc.parse<0>(pBuffer);

    xml_node<> *nProject = doc.first_node("project");
    xml_node<> *nResources = nProject->first_node("resources");
    for (xml_node<> *nResource = nResources->first_node("resource"); nResource; nResource = nResource->next_sibling("resource"))
    {
      udFixedSlice<const udKeyValuePair, 256> params;
      for (xml_attribute<> *attr = nResource->first_attribute(); attr; attr = attr->next_attribute())
      {
        params.concat(udKeyValuePair(attr->name(), attr->value()));
      }
      udInitParams resourceParams = udInitParams(params);

      spResourceManager->LoadResourcesFromFile({ {"src", resourceParams["src"]} });
    }
  }
  catch (parse_error e)
  {
    LogError("Unable to parse project file: {0} at byte {1} : {2}", src.asString(), (size_t)(e.where<char>() - pBuffer), e.what());
  }

  udFree(pBuffer);
}

} // namespace ud
