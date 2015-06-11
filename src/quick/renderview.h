#include <QQuickFramebufferObject>

class RenderView : public QQuickFramebufferObject
{
  Q_OBJECT

public:
  RenderView(QQuickItem *pParent = 0);
  virtual ~RenderView();

  Renderer *createRenderer() const;

protected:
  class Renderer : public QQuickFramebufferObject::Renderer
  {
  protected:
//    ~Renderer();
//    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size);
//    void synchronize(QQuickFramebufferObject *);
    void render();
  };
};
