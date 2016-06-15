#include "../headers/%{ProjectName}.h"

#include "ep/cpp/component/viewport.h"
#include "ep/cpp/component/view.h"
#include "ep/cpp/component/scene.h"
#include "ep/cpp/component/node/simplecamera.h"
#include "ep/cpp/component/node/udnode.h"
#include "ep/cpp/component/resource/udmodel.h"
#include "ep/cpp/component/resource/metadata.h"
#include "ep/cpp/component/datasource/datasource.h"
#include "ep/cpp/component/resourcemanager.h"
#include "ep/cpp/component/commandmanager.h"
#include "ep/cpp/kernel.h"
#include "ep/cpp/component/resource/shader.h"
#include "ep/cpp/component/resource/material.h"
#include "ep/cpp/component/resource/model.h"

namespace ep {

  Array<const PropertyInfo> MyPlugin::GetProperties() const
  {
    return{

    };
  }

  MyPlugin::MyPlugin(const ComponentDesc *pType, Kernel *pKernel, SharedString uid, Variant::VarMap initParams)
    : Activity(pType, pKernel, uid, initParams)
  {

  }

  extern "C" bool epPluginAttach()
  {
    Kernel::GetInstance()->RegisterComponentType<MyPlugin>();

    // TODO: this path should be changed before release
    Kernel::GetInstance()->Call("registerqmlcomponents", "plugin/MyPlugin/qml");

    return true;
  }

} // namespace ep
