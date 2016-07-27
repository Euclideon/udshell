#include "driver.h"

#if EPUI_DRIVER == EPDRIVER_QT

#include "driver/qt/epqt.h"
#include "focusmanager_qt.h"
#include "../epkernel_qt.h"

#include <QQuickItem>
#include <QGuiApplication>

namespace qt {

QtFocusManager::~QtFocusManager()
{
  for (auto iter = stackMap.begin(); iter != stackMap.end(); iter++)
    clear(iter.key());

  for (auto iter = activeFocusMap.begin(); iter != activeFocusMap.end(); iter++)
  {
    if (iter.key() == qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow()))
      ActiveFocusLost(iter.value());
  }
}

void QtFocusManager::push(QQuickItem *pQtItem)
{
  if (!pQtItem)
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't push item onto focus stack. Item is null.");
    return;
  }

  QQuickWindow *pQtWindow = pQtItem->window();
  if (!pQtWindow)
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't push item onto focus stack. Item has no parent window.");
    return;
  }

  if (!stackMap.contains(pQtWindow))
    CreateWindowStack(pQtWindow);

  QList<QQuickItem *> &stack = stackMap[pQtWindow];

  if (!stack.contains(pQtItem))
  {
    stack.append(pQtItem);

    ConnectItemSignals(pQtItem);
  }
}

void QtFocusManager::pushActiveFocusItem(QQuickWindow *pQtWindow)
{
  if (!pQtWindow)
    pQtWindow = qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow());

  if (!pQtWindow)
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't push active focus item onto focus stack. No window specified and no active window.");
    return;
  }

  QQuickItem *pActiveQtItem = pQtWindow->activeFocusItem();
  if (pActiveQtItem == nullptr)
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't push item onto focus stack. Item is null.");
    return;
  }
  else if (pActiveQtItem == pQtWindow->contentItem())
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't push item onto focus stack. Pushing window's root content item to focus stack is not alloved.");
    return;
  }

  push(pActiveQtItem);
}

// Pushes the current active focus item onto the focus stack then switches focus to the given item.
void QtFocusManager::setFocus(QQuickItem *pQtItem)
{
  if (!pQtItem)
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't set focus to item. Item is null.");
    return;
  }

  pushActiveFocusItem(pQtItem->window());
  pQtItem->forceActiveFocus();
}

QQuickItem * QtFocusManager::pop(QQuickWindow *pQtWindow)
{
  if (!pQtWindow)
    pQtWindow = qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow());

  if (!pQtWindow || !stackMap.contains(pQtWindow))
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't pop item from window's focus stack. No window specified and no active window.");
    return nullptr;
  }

  QList<QQuickItem *> &stack = stackMap[pQtWindow];

  for (int i = stack.size() - 1; i >= 0; --i)
  {
    QQuickItem *pQtItem = stack.at(i);

    // Skip past items that are invisible as they probably belong to a different tab which acts independantly
    // Also skip past items that are disabled or that belong to another window (which should never exist.)
    // These dead items will get pruned by the OnItemActiveFocusChanged event handler
    if (!pQtItem->isVisible() || pQtItem->window() != pQtWindow || !pQtItem->isEnabled())
      continue;

    stack.pop_back();

    DisconnectItemSignals(pQtItem);

    if (stack.empty())
      DestroyWindowStack(pQtWindow);

    return pQtItem;
  }

  return nullptr;
}

// Pop the top item on the given window's focus stack and set it as the active focus item
QQuickItem * QtFocusManager::restoreFocus(QQuickWindow *pQtWindow)
{
  QQuickItem *pQtItem = pop(pQtWindow);

  if (pQtItem)
    pQtItem->forceActiveFocus();

  return pQtItem;
}

void QtFocusManager::clear(QQuickWindow *pQtWindow)
{
  if (!pQtWindow)
    pQtWindow = qobject_cast<QQuickWindow *>(QGuiApplication::focusWindow());

  if (!pQtWindow || !stackMap.contains(pQtWindow))
  {
    QtApplication::Kernel()->logWarning(1, "QtFocusManager -- Can't clear window's focus stack. Given window is null and there is no active window.");
    return;
  }

  QList<QQuickItem *> &stack = stackMap[pQtWindow];

  while (!stack.empty())
  {
    QQuickItem *pQtItem = stack.takeLast();
    DisconnectItemSignals(pQtItem);
  }

  DestroyWindowStack(pQtWindow);
}

void QtFocusManager::OnItemDestroyed(QObject *obj)
{
  QQuickItem *pQtItem = qobject_cast<QQuickItem *>(obj);

  if (!pQtItem)
    return;

  QQuickWindow *pQtWindow = pQtItem->window();

  if (!pQtWindow || !stackMap.contains(pQtWindow))
  {
    QtApplication::Kernel()->logWarning(2, "QtFocusManager -- Can't remove item from focus stack, because parent window of item not found.");
    return;
  }

  QList<QQuickItem *> &stack = stackMap[pQtWindow];
  stack.removeAll(pQtItem);
}

// If an item's parent window changes, remove it from all focus stacks.
void QtFocusManager::OnItemWindowChanged(QQuickWindow *pQtWindow)
{
  QQuickItem *pQtItem = qobject_cast<QQuickItem *>(QObject::sender());

  OnItemParentChanged(pQtItem);
}

// If an item's parent item changes we just remove it from all focus stacks.
// Since its order in the current stack no longer means anything
void QtFocusManager::OnItemParentChanged(QQuickItem *pQtItem)
{
  for (auto iter = stackMap.begin(); iter != stackMap.end(); iter++)
    iter.value().removeAll(pQtItem);

  DisconnectItemSignals(pQtItem);

  return;
}

// When an item on a focus stack gains active focus, prune away all higher (visible) items
// on the stack. Since they are no longer needed.
void QtFocusManager::OnItemActiveFocusChanged(bool activeFocus)
{
  if (!activeFocus)
    return;

  QQuickItem *pQtItem = qobject_cast<QQuickItem *>(QObject::sender());

  QQuickWindow *pQtWindow = pQtItem->window();
  if (!pQtWindow || !stackMap.contains(pQtWindow))
  {
    QtApplication::Kernel()->logWarning(2, "QtFocusManager -- Parent window of item on focus stack not found.");
    return;
  }

  QList<QQuickItem *> &stack = stackMap[pQtWindow];

  int itemIndex = stack.indexOf(pQtItem);
  if (itemIndex == -1)
  {
    QtApplication::Kernel()->logWarning(2, "QtFocusManager -- Unable to find Item in parent window's focus stack.");
    return;
  }

  for (int i = stack.size() - 1; i > itemIndex; --i)
  {
    QQuickItem *pPruneItem = stack.at(i);

    if (pPruneItem->isVisible())
    {
      DisconnectItemSignals(pPruneItem);
      stack.removeAt(i);
    }
  }
}

void QtFocusManager::OnWindowVisibleChanged(bool visible)
{
  if (!visible)
  {
    QQuickWindow *pQtWindow = qobject_cast<QQuickWindow *>(QObject::sender());
    clear(pQtWindow);
  }
}

void QtFocusManager::OnWindowFocusItemChanged()
{
  QQuickWindow *pQtWindow = qobject_cast<QQuickWindow *>(QObject::sender());
  QQuickItem *pQtItem = pQtWindow->activeFocusItem();

  if (pQtItem == pQtWindow->contentItem()) // Qt will give focus to the window's contentItem if the item with active focus gets destroyed (Possibly in some other cases.) So when this happens, we restore focus to the top item on the focus stack
    restoreFocus(pQtWindow);
  else if (pQtItem)
    ActiveFocusGained(pQtItem);
}

void QtFocusManager::OnFocusItemActiveFocusChanged(bool activeFocus)
{
  // Active focus has changed, but it has probably intentionally been changed to another item, so just disconnect our signals from the old active focus item

  if (!activeFocus)
  {
    QQuickItem *pQtItem = qobject_cast<QQuickItem *>(QObject::sender());
    ActiveFocusLost(pQtItem);
  }
}

// Monitor when the active focus item becomes invisible, in which case set the focus to the top item on the focus stack
void QtFocusManager::OnFocusItemVisibleChanged()
{
  QQuickItem *pQtItem = qobject_cast<QQuickItem *>(QObject::sender());

  if (!pQtItem->isVisible())
  {
    ActiveFocusLost(pQtItem);

    QQuickWindow *pQtWindow = pQtItem->window();

    if (!pQtWindow || !stackMap.contains(pQtWindow))
      return;

    restoreFocus(pQtWindow);
  }
}

// Monitor when the active focus is destroyed, in which case set the focus to the top item on the focus stack
void QtFocusManager::OnFocusItemDestroyed(QObject *obj)
{
  QQuickItem *pQtItem = qobject_cast<QQuickItem *>(obj);
  if (!pQtItem)
    return;

  ActiveFocusLost(pQtItem);

  QQuickWindow *pQtWindow = pQtItem->window();

  if (!pQtWindow || !stackMap.contains(pQtWindow))
    return;

  restoreFocus(pQtWindow);
}

void QtFocusManager::ActiveFocusGained(QQuickItem *pQtItem)
{
  activeFocusMap.insert(pQtItem->window(), pQtItem);
  QObject::connect(pQtItem, &QQuickItem::activeFocusChanged, this, &QtFocusManager::OnFocusItemActiveFocusChanged);
  QObject::connect(pQtItem, &QQuickItem::visibleChanged, this, &QtFocusManager::OnFocusItemVisibleChanged);
  QObject::connect(pQtItem, &QQuickItem::destroyed, this, &QtFocusManager::OnFocusItemDestroyed);
}

void QtFocusManager::ActiveFocusLost(QQuickItem *pQtItem)
{
  QObject::disconnect(pQtItem, &QQuickItem::activeFocusChanged, this, &QtFocusManager::OnFocusItemActiveFocusChanged);
  QObject::disconnect(pQtItem, &QQuickItem::visibleChanged, this, &QtFocusManager::OnFocusItemVisibleChanged);
  QObject::disconnect(pQtItem, &QQuickItem::destroyed, this, &QtFocusManager::OnFocusItemDestroyed);
  activeFocusMap.remove(pQtItem->window());
}

void QtFocusManager::ConnectItemSignals(QQuickItem *pQtItem)
{
  QObject::connect(pQtItem, &QQuickItem::destroyed, this, &QtFocusManager::OnItemDestroyed);
  QObject::connect(pQtItem, &QQuickItem::windowChanged, this, &QtFocusManager::OnItemWindowChanged);
  QObject::connect(pQtItem, &QQuickItem::parentChanged, this, &QtFocusManager::OnItemParentChanged);
  QObject::connect(pQtItem, &QQuickItem::activeFocusChanged, this, &QtFocusManager::OnItemActiveFocusChanged);
}

void QtFocusManager::DisconnectItemSignals(QQuickItem *pQtItem)
{
  QObject::disconnect(pQtItem, &QQuickItem::destroyed, this, &QtFocusManager::OnItemDestroyed);
  QObject::disconnect(pQtItem, &QQuickItem::windowChanged, this, &QtFocusManager::OnItemWindowChanged);
  QObject::disconnect(pQtItem, &QQuickItem::parentChanged, this, &QtFocusManager::OnItemParentChanged);
  QObject::disconnect(pQtItem, &QQuickItem::activeFocusChanged, this, &QtFocusManager::OnItemActiveFocusChanged);
}

void QtFocusManager::CreateWindowStack(QQuickWindow *pQtWindow)
{
  stackMap.insert(pQtWindow, QList<QQuickItem *>());
  QObject::connect(pQtWindow, &QQuickWindow::activeFocusItemChanged, this, &QtFocusManager::OnWindowFocusItemChanged);
  QObject::connect(pQtWindow, &QQuickWindow::visibleChanged, this, &QtFocusManager::OnWindowVisibleChanged);
}

void QtFocusManager::DestroyWindowStack(QQuickWindow *pQtWindow)
{
  QObject::disconnect(pQtWindow, &QQuickWindow::activeFocusItemChanged, this, &QtFocusManager::OnWindowFocusItemChanged);
  QObject::disconnect(pQtWindow, &QQuickWindow::visibleChanged, this, &QtFocusManager::OnWindowVisibleChanged);
  stackMap.remove(pQtWindow);
}

} // end namespace qt

#else
EPEMPTYFILE
#endif
