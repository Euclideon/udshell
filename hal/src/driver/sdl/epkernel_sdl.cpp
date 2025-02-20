#include "driver.h"

#if EPKERNEL_DRIVER == EPDRIVER_SDL

#include "kernelimpl.h"
#include "components/viewimpl.h"
#include "renderscene.h"
#include "hal/input.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "input_internal.h"

SDL_Window* s_window = nullptr;
SDL_GLContext s_context = nullptr;
bool s_done = false;
static int s_displayWidth, s_displayHeight;
static Uint32 s_sdlEvent = (Uint32)-1;

struct DelegateWithSemaphore
{
  FastDelegateMemento m;
  udSemaphore *pSem;
};

using namespace ep;

class SDLKernel : public Kernel
{
  EP_DECLARE_COMPONENT(ep, SDLKernel, Kernel, EPKERNEL_PLUGINVERSION, "SDL Kernel instance", 0)
public:
  SDLKernel(Variant::VarMap commandLine);
  ~SDLKernel();

  void runMainLoop() override final;
  void quit() override final;

  ViewRef setFocusView(ViewRef spView) override final;
  void dispatchToMainThread(MainThreadCallback callback) override final;
  void dispatchToMainThreadAndWait(MainThreadCallback callback) override final;

private:
  static ComponentDescInl *makeKernelDescriptor();

  void eventLoop();
};

void SDLKernel::eventLoop()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    if (event.type == s_sdlEvent)
    {
      if (event.user.code == 0)
      {
        FastDelegateMemento m;
        void **ppPtrs = (void**)&m;
        ppPtrs[0] = event.user.data1;
        ppPtrs[1] = event.user.data2;

        MainThreadCallback d;
        d.SetMemento(m);
        try {
          d();
        } catch (std::exception &e) {
          logError("Exception occurred in MainThreadCallback : {0}", e.what());
        } catch (...) {
          logError("Exception occurred in MainThreadCallback : C++ Exception");
        }
      }
      else if (event.user.code == 1)
      {
        DelegateWithSemaphore *pDispatch = (DelegateWithSemaphore*)event.user.data1;

        MainThreadCallback d;
        d.SetMemento(pDispatch->m);
        try {
          d();
        } catch (std::exception &e) {
          logError("Exception occurred in MainThreadCallback : {0}", e.what());
        } catch (...) {
          logError("Exception occurred in MainThreadCallback : C++ Exception");
        }
        udIncrementSemaphore(pDispatch->pSem);
      }
    }
    else
    {
      switch (event.type)
      {
      case SDL_QUIT:
        s_done = true;
        break;
      case SDL_WINDOWEVENT:
      {
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_RESIZED:
          s_displayWidth = event.window.data1;
          s_displayHeight = event.window.data2;
          getImpl()->spFocusView->resize(s_displayWidth, s_displayHeight);
          glViewport(0, 0, s_displayWidth, s_displayHeight);
          break;
        }
        break;
      }
      }
    }
  }
}

ComponentDescInl *SDLKernel::makeKernelDescriptor()
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, Result::AllocFailure, "Memory allocation failed");

  pDesc->info = SDLKernel::componentInfo();
  pDesc->info.flags = ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Kernel::componentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : SDLKernel::getPropertiesImpl())
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : SDLKernel::getMethodsImpl())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : SDLKernel::getEventsImpl())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : SDLKernel::getStaticFuncsImpl())
    pDesc->staticFuncTree.insert(f.id, { f, (void*)f.pCall });

  return pDesc;
}
SDLKernel::SDLKernel(Variant::VarMap commandLine)
  : Kernel(SDLKernel::makeKernelDescriptor(), commandLine)
{
  s_displayWidth = 1280;
  s_displayHeight = 720;

  int sdlInit = SDL_Init(SDL_INIT_VIDEO);
  EPTHROW_IF(sdlInit < 0, Result::Failure, "Failed to initialise SDL");
  epscope(fail) { SDL_Quit(); };

  s_sdlEvent = SDL_RegisterEvents(1);
  EPTHROW_IF(s_sdlEvent == (Uint32)-1, Result::Failure, "Failed to register SDL events");

  s_window = SDL_CreateWindow("udPointCloud Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_displayWidth, s_displayHeight, SDL_WINDOW_OPENGL);
  EPTHROW_IF(!s_window, Result::Failure, "Failed to create SDL Window");
  epscope(fail) { SDL_DestroyWindow(s_window); };

  s_context = SDL_GL_CreateContext(s_window);
  EPTHROW_IF(!s_context, Result::Failure, "Failed to create SDL Window");
  epscope(fail) { SDL_GL_DeleteContext(s_context); };

  getImpl()->InitRender();
}

SDLKernel::~SDLKernel()
{
  getImpl()->Shutdown();
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.
  eventLoop();

  getImpl()->DeinitRender();

  SDL_GL_DeleteContext(s_context);
  SDL_Quit();
}

void SDLKernel::runMainLoop()
{
  finishInit();

  while (!s_done)
  {
    eventLoop();

    // TODO: need to translate input polling into messages...
    epInput_Update();

    InputState &input = gInputState[gCurrentInputState];
    InputState &prevInput = gInputState[1 - gCurrentInputState];

    InputEvent ie;
    if (input.mouse[0][MouseControls::XDelta] || input.mouse[0][MouseControls::YDelta])
    {
      ie.deviceType = InputDevice::Mouse;
      ie.eventType = InputEvent::EventType::Move;
      ie.move = InputEvent::MoveEvent{ input.mouse[0][MouseControls::XDelta], input.mouse[0][MouseControls::YDelta], input.mouse[0][MouseControls::XAbsolute], input.mouse[0][MouseControls::YAbsolute] };
      getImpl()->spFocusView->getImpl<ep::ViewImpl>()->inputEvent(ie);
    }

    ie.deviceType = InputDevice::Mouse;
    ie.eventType = InputEvent::EventType::Key;
    for (size_t i = (int)MouseControls::LeftButton; i < (int)MouseControls::Button5; i++)
    {
      if (input.mouse[0][i] != prevInput.mouse[0][i])
      {
        ie.key = InputEvent::KeyEvent{ (int)i, (int)input.mouse[0][i] };
        getImpl()->spFocusView->getImpl<ep::ViewImpl>()->inputEvent(ie);
      }
    }

    ie.deviceType = InputDevice::Keyboard;
    ie.eventType = InputEvent::EventType::Key;
    for (size_t i = 0; i < (int)KeyCode::Max; i++)
    {
      if (input.keys[0][i] != prevInput.keys[0][i])
      {
        ie.key = InputEvent::KeyEvent{ (int)i, (int)input.keys[0][i] };
        getImpl()->spFocusView->getImpl<ep::ViewImpl>()->inputEvent(ie);
      }
    }

    // render a frame (this could move to another thread!)
    RenderableViewRef spRenderView = getImpl()->spFocusView->getImpl<ViewImpl>()->getRenderableView();
    if (spRenderView)
      spRenderView->RenderGPU();

    SDL_GL_SwapWindow(s_window);
  }
}

ViewRef SDLKernel::setFocusView(ViewRef spView)
{
  KernelImpl *pKernelImpl = getImpl();

  if (!spView)
    pKernelImpl->spFocusView->getImpl<ViewImpl>()->setLatestFrame(nullptr);

  ViewRef spOld = pKernelImpl->spFocusView;
  pKernelImpl->spFocusView = spView;

  if (pKernelImpl->spFocusView)
    pKernelImpl->spFocusView->resize(s_displayWidth, s_displayHeight);
  return spOld;
}

void SDLKernel::dispatchToMainThread(MainThreadCallback callback)
{
  FastDelegateMemento m = callback.GetMemento();
  void **ppPtrs = (void**)&m;

  SDL_Event e;
  SDL_zero(e);
  e.type = s_sdlEvent;
  e.user.code = 0;
  e.user.data1 = ppPtrs[0];
  e.user.data2 = ppPtrs[1];
  SDL_PushEvent(&e);
}
void SDLKernel::dispatchToMainThreadAndWait(MainThreadCallback callback)
{
  DelegateWithSemaphore dispatch;
  dispatch.m = callback.GetMemento();
  dispatch.pSem = udCreateSemaphore(1, 0);

  SDL_Event e;
  SDL_zero(e);
  e.type = s_sdlEvent;
  e.user.code = 1;
  e.user.data1 = &dispatch;
  SDL_PushEvent(&e);

  udWaitSemaphore(dispatch.pSem);
  udDestroySemaphore(&dispatch.pSem);
}

void SDLKernel::quit()
{
  s_done = true;
}

namespace ep {

Kernel* Kernel::createInstanceInternal(Variant::VarMap commandLine)
{
  return KernelImpl::CreateComponentInstance<SDLKernel>(commandLine);
}

} // namespace ep

#else
EPEMPTYFILE
#endif
