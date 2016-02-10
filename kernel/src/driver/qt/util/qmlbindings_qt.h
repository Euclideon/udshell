#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component/component.h"
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
  QtKernelQml(QtKernel *_pKernel, QObject *pParent = nullptr) : QObject(pParent), pKernel(_pKernel) {}

  Q_INVOKABLE qt::QtEPComponent *findComponent(const QString &uid) const;
  Q_INVOKABLE qt::QtEPComponent *createComponent(const QString typeId, QVariantMap initParams = QVariantMap());
  Q_INVOKABLE qt::QtEPComponent *getCommandManager() const;

private:
  QtKernel *pKernel;
};

} // namespace qt

#endif  // QMLBINDINGS_QT_H
