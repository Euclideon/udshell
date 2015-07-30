#ifndef WINDOW_H
#define WINDOW_H

#include <QQuickWindow>

namespace qt
{

class Window : public QQuickWindow
{
  Q_OBJECT

public:
  explicit Window(QWindow *parent = 0);
  virtual ~Window();
};

} // namespace qt

#endif  // WINDOW_H
