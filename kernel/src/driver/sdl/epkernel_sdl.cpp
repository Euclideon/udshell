#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_SDL

#include "kernel.h"
#include "renderscene.h"
#include "components/view.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

SDL_Window* s_window = nullptr;
SDL_GLContext s_context = nullptr;
bool s_done = false;
static int s_displayWidth, s_displayHeight;
static Uint32 s_sdlEvent = (Uint32)-1;

using namespace ep;

class SDLKernel : public Kernel
{
public:
  SDLKernel() {}

  udResult InitInternal() override;
  udResult Destroy() override;
  udResult RunMainLoop() override;
};

Kernel *Kernel::CreateInstanceInternal(epInitParams commandLine)
{
  return new SDLKernel;
}

udResult SDLKernel::InitInternal()
{
  s_displayWidth = 1280;
  s_displayHeight = 720;

  int sdlInit = SDL_Init(SDL_INIT_VIDEO);
  if (sdlInit < 0)
    return udR_Failure_;

  s_sdlEvent = SDL_RegisterEvents(1);
  if (s_sdlEvent < 0)
  {
    SDL_Quit();
    return udR_Failure_;
  }

  s_window = SDL_CreateWindow("udPointCloud Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_displayWidth, s_displayHeight, SDL_WINDOW_OPENGL);
  if (!s_window)
  {
    SDL_Quit();
    return udR_Failure_;
  }

  s_context = SDL_GL_CreateContext(s_window);
  if (!s_context)
  {
    SDL_DestroyWindow(s_window);
    SDL_Quit();
    return udR_Failure_;
  }

  udResult result = InitRender();
  if (result != udR_Success)
  {
    SDL_GL_DeleteContext(s_context);
    SDL_DestroyWindow(s_window);
    SDL_Quit();
    return result;
  }

  return udR_Success;
}

udResult SDLKernel::Destroy()
{
  // this
  SDL_GL_DeleteContext(s_context);
  SDL_Quit();

  DeinitRender();
  return udR_Success;
}

ViewRef Kernel::SetFocusView(ViewRef spView)
{
  ViewRef spOld = spFocusView;
  spFocusView = spView;
  spFocusView->Resize(s_displayWidth, s_displayHeight);
  return spOld;
}

struct DelegateWithSemaphore
{
  FastDelegateMemento m;
  udSemaphore *pSem;
};

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

udResult SDLKernel::RunMainLoop()
{
  DoInit(this);

  while (!s_done)
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
          d(this);
        }
        else if (event.user.code == 1)
        {
          DelegateWithSemaphore *pDispatch = (DelegateWithSemaphore*)event.user.data1;

          MainThreadCallback d;
          d.SetMemento(pDispatch->m);
          d(this);

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

    // TODO: need to translate input polling into messages...
    epInput_Update();

    // render a frame (this could move to another thread!)
    RenderableViewRef spRenderView = spFocusView->GetRenderableView();
    if (spRenderView)
      spRenderView->RenderGPU();

    SDL_GL_SwapWindow(s_window);
  }
  return udR_Success;
}

udResult Kernel::Terminate()
{
  s_done = true;
  return udR_Success;
}

#endif
