#include "hal/driver.h"
#include "components/viewimpl.h"

#if EPWINDOW_DRIVER == EPDRIVER_SDL

#include "kernel.h"
#include "renderscene.h"
#include "hal/input.h"
#include "components/viewimpl.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

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

class SDLKernel : public kernel::Kernel
{
public:
  SDLKernel() {}
  ~SDLKernel();

  void InitInternal() override;
  void RunMainLoop() override;
  void EventLoop();
};

void SDLKernel::EventLoop()
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
        try { d(this); }
        catch (std::exception &e) { LogError("Exception occurred in MainThreadCallback : {0}", e.what()); }
        catch (...) { LogError("Exception occurred in MainThreadCallback : C++ Exception"); }
      }
      else if (event.user.code == 1)
      {
        DelegateWithSemaphore *pDispatch = (DelegateWithSemaphore*)event.user.data1;

        MainThreadCallback d;
        d.SetMemento(pDispatch->m);
        try { d(this); }
        catch (std::exception &e) { LogError("Exception occurred in MainThreadCallback : {0}", e.what()); }
        catch (...) { LogError("Exception occurred in MainThreadCallback : C++ Exception"); }
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
          spFocusView->Resize(s_displayWidth, s_displayHeight);
          glViewport(0, 0, s_displayWidth, s_displayHeight);
          break;
        }
        break;
      }
      }
    }
  }
}


void SDLKernel::InitInternal()
{
  s_displayWidth = 1280;
  s_displayHeight = 720;

  int sdlInit = SDL_Init(SDL_INIT_VIDEO);
  EPTHROW_IF(sdlInit < 0, epR_Failure, "Failed to initialise SDL");
  epscope(fail) { SDL_Quit(); };

  s_sdlEvent = SDL_RegisterEvents(1);
  EPTHROW_IF(s_sdlEvent == (Uint32)-1, epR_Failure, "Failed to register SDL events");

  s_window = SDL_CreateWindow("udPointCloud Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_displayWidth, s_displayHeight, SDL_WINDOW_OPENGL);
  EPTHROW_IF(!s_window, epR_Failure, "Failed to create SDL Window");
  epscope(fail) { SDL_DestroyWindow(s_window); };

  s_context = SDL_GL_CreateContext(s_window);
  EPTHROW_IF(!s_context, epR_Failure, "Failed to create SDL Window");
  epscope(fail) { SDL_GL_DeleteContext(s_context); };

  InitRender();
}

SDLKernel::~SDLKernel()
{
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.
  EventLoop();

  DeinitRender();

  SDL_GL_DeleteContext(s_context);
  SDL_Quit();
}

void SDLKernel::RunMainLoop()
{
  DoInit(this);

  while (!s_done)
  {
    EventLoop();

    // TODO: need to translate input polling into messages...
    epInput_Update();

    // render a frame (this could move to another thread!)
    RenderableViewRef spRenderView = spFocusView->GetImpl<ViewImpl>()->GetRenderableView();
    if (spRenderView)
      spRenderView->RenderGPU();

    SDL_GL_SwapWindow(s_window);
  }
}

namespace kernel {

Kernel *Kernel::CreateInstanceInternal(Slice<const KeyValuePair> commandLine)
{
  return new SDLKernel;
}

ViewRef Kernel::SetFocusView(ViewRef spView)
{
  if (!spView)
    spFocusView->GetImpl<ViewImpl>()->SetLatestFrame(nullptr);

  ViewRef spOld = spFocusView;
  spFocusView = spView;

  if (spFocusView)
    spFocusView->Resize(s_displayWidth, s_displayHeight);
  return spOld;
}

void Kernel::DispatchToMainThread(MainThreadCallback callback)
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
void Kernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
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

void Kernel::Terminate()
{
  s_done = true;
}

} // namespace kernel

#else
EPEMPTYFILE
#endif
