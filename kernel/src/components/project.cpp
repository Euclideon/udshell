#include "components/project.h"
#include "components/file.h"
#include "components/resourcemanager.h"
#include "kernel.h"
#include "rapidxml.hpp"

namespace ep
{

ComponentDesc Project::descriptor =
{
  &Component::descriptor, // pSuperDesc

  EPSHELL_APIVERSION, // epVersion
  EPSHELL_PLUGINVERSION, // pluginVersion

  "project", // id
  "Project", // displayName
  "Contains the state of a project", // description

  nullptr, // properties
  nullptr, // methods
  nullptr, // events
};

Project::Project(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, InitParams initParams)
  : Component(pType, pKernel, uid, initParams)
{
  const Variant &src = initParams["src"];
  StreamRef spSrc = nullptr;
  ResourceManagerRef spResourceManager = pKernel->GetResourceManager();

  if (src.is(Variant::Type::String))
  {
    // path or url?
    spSrc = pKernel->CreateComponent<File>({ { "path", src }, { "flags", FileOpenFlags::Read | FileOpenFlags::Text } });
    if (!spSrc)
    {
      LogWarning(5, "\"src\" file path not found: {0}", src);
      throw epR_File_OpenFailure;
    }
  }
  else
  {
    LogError("Missing or invalid parameter \"src\" when creating Project");
    throw epR_InvalidParameter_;
  }

  size_t len = (size_t)spSrc->Length();
  char *pBuffer = (char *)epAlloc(len + 1);
  spSrc->Read(Slice<void>(pBuffer, len));
  pBuffer[len] = '\0';

  using namespace rapidxml;

  try
  {
    xml_document<> doc;
    doc.parse<0>(pBuffer);

    xml_node<> *nProject = doc.first_node("project");
    xml_node<> *nDataSources = nProject->first_node("datasources");
    for (xml_node<> *nDataSource = nDataSources->first_node("datasource"); nDataSource; nDataSource = nDataSource->next_sibling("datasource"))
    {
      Array<const KeyValuePair, 256> dsParams;
      for (xml_attribute<> *attr = nDataSource->first_attribute(); attr; attr = attr->next_attribute())
      {
        dsParams.concat(KeyValuePair(attr->name(), attr->value()));
      }
      spResourceManager->LoadResourcesFromFile(InitParams(dsParams));
    }
  }
  catch (parse_error e)
  {
    LogError("Unable to parse project file: {0} at byte {1} : {2}", src.asString(), (size_t)(e.where<char>() - pBuffer), e.what());
  }

  epFree(pBuffer);
}

} // namespace ep
