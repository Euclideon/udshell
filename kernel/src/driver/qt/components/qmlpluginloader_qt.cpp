#include "hal/driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "components/file.h"

#include "driver/qt/epqt.h"

#include "driver/qt/components/qmlpluginloader_qt.h"
#include "driver/qt/epkernel_qt.h"
#include "driver/qt/util/typeconvert_qt.h"

#include <QRegularExpression>

using ep::Slice;
using ep::String;
using ep::Variant;

namespace qt {

Slice<const String> QmlPluginLoader::GetSupportedExtensions() const
{
  static ep::Array<const String> s_ext = { ".qml" };
  return s_ext;
}

bool QmlPluginLoader::LoadPlugin(String filename)
{
  QtKernel *pQtKernel = static_cast<QtKernel*>(pKernel);
  ep::Variant::VarMap descMap;

  try {
    descMap = ParseTypeDescriptor(pQtKernel, filename);
    if (!descMap.empty())
      pQtKernel->RegisterQml(filename, descMap);
  }
  catch (ep::EPException &e) {
    // if ParseTypeDescriptor threw, then we have an invalid type descriptor
    if (descMap.empty())
      LogWarning(2, "Could not register QML file '{0}' as Component: \"{1}\"", filename, e.what());
    // if RegisterQml threw, then it might be due to the super not yet being loaded - hence we should try again
    else
      LogDebug(2, "Failed attempt to register QML file '{0}' as Component: \"{1}\"", filename, e.what());

    ep::ClearError();
    return descMap.empty();
  }

  return true;
}

Variant::VarMap QmlPluginLoader::ParseTypeDescriptor(QtKernel *pQtKernel, ep::String filename)
{
  // Open and read the entire file
  QFile file(epToQString(filename));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    EPTHROW(epR_Failure, "Unable to open file '{0}'", filename);
  QByteArray data = file.readAll();

  // Check if we contain an epTypeDesc property
  QString fileData(data);
  int descPropertyIndex = fileData.indexOf(QRegularExpression("property\\s+var\\s+epTypeDesc:\\s+{", QRegularExpression::CaseInsensitiveOption));
  if (descPropertyIndex != -1)
  {
    // Now attempt to extract the code block that is the value of our javascript map
    int expStart = fileData.indexOf(QChar('{'), descPropertyIndex);
    int expEnd = expStart;

    if (expStart != -1)
    {
      int braceStack = 1;
      do
      {
        ++expEnd;
        if (fileData[expEnd] == QChar('{'))
        {
          ++braceStack;
        }
        else if (fileData[expEnd] == QChar('}'))
        {
          if ((--braceStack) == 0)
            break;
        }
      } while (expEnd != fileData.size());

      // Malformed
      if (braceStack != 0) EPTHROW(epR_Failure, "Cannot Parse Type Descriptor: File contains malformed 'epTypeDesc' property map");

      // Evaluate the value of the property map using the JS engine
      Variant result = epToVariant(pQtKernel->QmlEngine()->evaluate(fileData.mid(expStart, expEnd - expStart + 1)));
      if (result.is(Variant::Type::Error))
        EPTHROW(epR_ScriptException, "Cannot Parse Type Descriptor: 'epTypeDesc' property map does not contain a valid javascript expression");

      Variant::VarMap typeDesc = result.asAssocArray();

      // id and super must be present
      Variant *pId = typeDesc.get("id");
      Variant *pSuper = typeDesc.get("super");
      if (!pId) EPTHROW(epR_Failure, "Invalid Type Descriptor: Does not contain 'id' key");
      if (!pSuper) EPTHROW(epR_Failure, "Invalid Type Descriptor: Does not contain 'super' key");

      if (!typeDesc.get("displayname"))
        typeDesc.insert("displayname", pId->asString());

      // id must be lowercase
      *pId = ep::MutableString128(pId->asString()).toLower();

      if (!typeDesc.get("description"))
        typeDesc.insert("description", ep::SharedString::format("{0} based QML component - {1}", pSuper->asString(), filename));

      if (!typeDesc.get("version"))
        typeDesc.insert("version", (int)EPKERNEL_PLUGINVERSION);

      return typeDesc;
    }
    else
    {
      EPTHROW(epR_Failure, "Cannot Parse Type Descriptor: File contains empty 'epTypeDesc' property");
    }
  }
  return nullptr;
}

} // namespace qt

#else
EPEMPTYFILE
#endif  // EPUI_DRIVER == EPDRIVER_QT
