#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT
#if defined(realloc)
#undef realloc
#endif
#include <QObject>

#include "ep/cpp/component/window.h"

#include "driver/qt/util/qmlbindings_qt.h"
#include "driver/qt/components/qobjectcomponent_qt.h"

namespace qt {

QObjectComponent::QObjectComponent(const ep::ComponentDesc *pType, ep::Kernel *pKernel, ep::SharedString uid, ep::Variant::VarMap initParams)
  : ep::DynamicComponent(pType, pKernel, uid, initParams)
{
  // NOTE: There are two main types of QObjectComponents that we need to distinguish
  // 1) Public QObjectComponents that wrap externally created QObjects that we just want to pass through our system
  // 2) Private QObjectComponents that we create and store internally in glue extensible components
  // Public components are assumed to have the QObject pointer passed in (as an int) and will not claim ownership
  // - holding a reference to a public component is *NOT* recommended because the underlying QObject may be deleted
  // Private components are assumed to have a QmlComponentData pointer passed in and *will* claim ownership of the created QObject

  ep::Variant *pV = initParams.get("object");
  if (pV)
  {
    pQObject = (QObject*)(size_t)pV->asInt();
  }
  else
  {
    hasOwnership = true;
    pQmlComponentData = (QmlComponentData*)(size_t)initParams["qmlcomponentdata"].asInt();
  }
}

QObjectComponent::~QObjectComponent()
{
  if (hasOwnership)
    delete pQObject;
}

// PRIVATE QOBJECTCOMPONENTS ONLY -----------------------------------------------------------------

// Called by the Kernel following construction of the enclosing Glue Component
void QObjectComponent::AttachToGlue(Component *pGlue, ep::Variant::VarMap initParams)
{
  using namespace ep;
  Super::AttachToGlue(pGlue, initParams);

  // Create a new QObject instance from the stored QmlComponentData
  EPASSERT(pQmlComponentData, "Attempting to attach a public/instantiated QObjectComponent to a Glue Component");
  pQObject = pQmlComponentData->CreateInstance(static_cast<QtKernel*>(pKernel)->QmlEngine(), pThis, initParams);
  pUserData = pQObject;

  // Populate the glue's descriptor with the meta from the QObject
  ComponentDescInl *pDesc = (ComponentDescInl*)pThis->GetDescriptor();
  if (pDesc->info.flags & ComponentInfoFlags::Unpopulated)
  {
    internal::PopulateComponentDesc(pDesc, pQObject);
    pDesc->info.flags &= ~ComponentInfoFlags::Unpopulated;
  }
  if (pThis->IsType("ep.window"))
  {
    EPTHROW_IF(!pQObject->isWindowType(), Result::Failure, "Window component must create a QWindow based object");
    static_cast<ep::Window*>(pThis)->PostInit(pQObject);
  }
  else
  {
    EPTHROW_IF(qobject_cast<QQuickItem*>(pQObject) == nullptr, Result::Failure, "UI based components must create a QQuickItem");
    static_cast<ep::UIComponent*>(pThis)->PostInit(pQObject);
  }

  // Construction is complete, so we don't need this anymore
  pQmlComponentData = nullptr;
}

} // namespace qt

#else
EPEMPTYFILE
#endif
