#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "ep/cpp/component.h"

#include "qobject.h"

namespace qt {

void PopulateComponentDesc(Component *pComponent, QObject *pObject);

} // namespace qt

#endif  // QMLBINDINGS_QT_H
