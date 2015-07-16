#ifndef WINDOW_H
#define WINDOW_H

#include <qquickview.h>

class Window : public QQuickView
{
  Q_OBJECT

public:
  explicit Window(QWindow *parent = 0);
  virtual ~Window();
};


#endif  // WINDOW_H
