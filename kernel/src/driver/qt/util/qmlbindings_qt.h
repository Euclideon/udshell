#pragma once
#ifndef QMLBINDINGS_QT_H
#define QMLBINDINGS_QT_H

#include "components/component.h"

#include "qobject.h"

namespace qt {

void PopulateComponentDesc(kernel::Component *pComponent, QObject *pObject);

} // namespace qt

#endif  // QMLBINDINGS_QT_H
