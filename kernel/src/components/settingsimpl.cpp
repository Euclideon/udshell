#include "components/settingsimpl.h"
#include "components/fileimpl.h"
#include "rapidxml.hpp"
#include "ep/cpp/component/resource/text.h"

namespace ep {

Array<const MethodInfo> Settings::getMethods() const
{
  return{
    EP_MAKE_METHOD(getValue, "Get a setting"),
    EP_MAKE_METHOD(setValue, "Set a setting"),
    EP_MAKE_METHOD(saveSettings, "Save Settings to an XML file"),
  };
}

SettingsImpl::SettingsImpl(Component *pInstance, Variant::VarMap initParams)
  : ImplSuper(pInstance)
{
  const Variant *pSrc = initParams.get("src");
  StreamRef spSrc = nullptr;

  if (pSrc && pSrc->is(Variant::Type::String))
  {
    srcString = pSrc->asString();

    try {
      spSrc = getKernel()->createComponent<File>({ { "path", *pSrc },{ "flags", FileOpenFlags::Read | FileOpenFlags::Text } });
    } catch (EPException &) {
      logDebug(2, "Settings file \"{0}\" does not exist. Create new settings.", *pSrc);
      return;
    }
  }
  else
  {
    logDebug(3, "No \"src\" parameter. Create empty settings");
    return;
  }

  using namespace rapidxml;
  Variant rootElements;
  TextRef spXMLBuffer = spSrc->loadText();

  try { rootElements = spXMLBuffer->parseXml(); }
  catch (parse_error &e)
  {
    auto xmlBuff = spXMLBuffer->mapForRead();
    epscope(exit) { spXMLBuffer->unmap(); };
    EPTHROW_ERROR(Result::Failure, "Unable to parse settings file: {0} on line {1} : {2}", srcString, Text::getLineNumberFromByteIndex(xmlBuff, (size_t)(e.where<char>() - xmlBuff.ptr)), e.what());
  }

  Variant::VarMap settingsNode = rootElements.asAssocArray();
  Variant *pName = settingsNode.get("name");
  if (pName && pName->is(Variant::Type::String) && pName->asString().eq("settings")) // TODO: I think we can make these comparisons better than this
    ParseSettings(settingsNode);
}

void SettingsImpl::SaveSettings()
{
  Variant::VarMap settingsNode;
  Array<Variant> children;

  settingsNode.insert("name", "settings");
  for (auto setting : settings)
  {
    Variant::VarMap node = Text::componentParamsToXmlMap(setting.value).asAssocArray();
    node.insert("name", setting.key);
    children.pushBack(node);
  }

  settingsNode.insert("children", children);

  auto spXMLBuffer = getKernel()->createComponent<Text>();
  spXMLBuffer->reserve(10240); // TODO: this is not okay...
  spXMLBuffer->formatXml(settingsNode);

  try {
    StreamRef spFile = getKernel()->createComponent<File>({ { "path", String(srcString) },{ "flags", FileOpenFlags::Create | FileOpenFlags::Write | FileOpenFlags::Text } });
    spFile->save(spXMLBuffer);
  } catch (EPException &) {
    logWarning(1, "Failed to open Settings file for writing: \"{0}\"", srcString);
    return;
  }
}

void SettingsImpl::ParseSettings(Variant node)
{
  Variant::VarMap settingsNode = node.asAssocArray();
  Variant *pChildren = settingsNode.get("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    Array<Variant> children = pChildren->asArray();
    for (auto child : children)
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
      {
        Variant *pName = child.getItem("name");
        if (pName && pName->is(Variant::Type::String))
        {
          Variant *pValue = child.getItem("text");
          //if it's a node
          if (!pValue)
          {
            ParsePluginSettings(child);
          }
          //else it's a leaf
          else
          {
            settings.insert(pName->asString(), *pValue);
          }
        }
      }
    }
  }
}

void SettingsImpl::ParsePluginSettings(Variant node)
{
  Variant *pNodeName = node.getItem("name");
  Variant *pChildren = node.getItem("children");
  if (pChildren && pChildren->is(Variant::Type::Array))
  {
    for (auto child : pChildren->asArray())
    {
      if (child.is(Variant::SharedPtrType::AssocArray))
      {
        Variant *pName = child.getItem("name");
        if (pName && pName->is(Variant::Type::String))
        {
          Variant *pValue = child.getItem("text");
          if (pValue)
            SetValue(pNodeName->asString(), pName->asString(), *pValue);
        }
      }
    }
  }
}

void SettingsImpl::SetValue(SharedString pluginkey, SharedString key, Variant value)
{
  Variant *pSetting = settings.get(pluginkey);
  if (pSetting != nullptr)
  {
    //test if node is an AVLTree<SharedString, Variant>
    if (pSetting->is(Variant::SharedPtrType::AssocArray))
    {
      pSetting->asAssocArray().replace(key, value);
    }
    //if not an AVLTree return pluginkey already use for a global setting
    else
      logWarning(0, "\"{0}\" is already used for a global setting", pluginkey);
  }
  else
  {
    settings.insert(pluginkey, AVLTree<SharedString, Variant> { { key, value} });
  }
}

Variant SettingsImpl::GetValue(SharedString pluginkey, SharedString key)
{
  Variant *pSetting = settings.get(pluginkey);
  if (pSetting == nullptr)
  {
    return Variant();
  }
  else
  {
    Variant *pValue = pSetting->asAssocArray().get(key);
    if (pValue == nullptr)
    {
      return Variant();
    }
    return *pValue;
  }
}

} // namespace ep
