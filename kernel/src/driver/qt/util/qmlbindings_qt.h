#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component.h"
#include "../components/component_qt.h"

#include <QObject>

namespace qt {

void PopulateComponentDesc(Component *pComponent, QObject *pObject);

class QtKernel;

// qml exposed kernel shim
class QtKernelQml : public QObject
{
  Q_OBJECT

public:
  QtKernelQml(QtKernel *_pKernel, QObject *pParent = nullptr) : pKernel(_pKernel), QObject(pParent) {}

  Q_INVOKABLE qt::QtEPComponent *FindComponent(const QString &uid) const;
  Q_INVOKABLE qt::QtEPComponent *CreateComponent(const QString typeId, QVariantMap initParams);
  Q_INVOKABLE qt::QtEPComponent *GetCommandManager() const;

private:
  QtKernel *pKernel;
};

} // namespace qt

#endif  // QMLBINDINGS_QT_H
