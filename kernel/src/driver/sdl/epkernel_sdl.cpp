#include "hal/driver.h"
#include "components/viewimpl.h"

#if EPWINDOW_DRIVER == EPDRIVER_SDL

#include "kernelimpl.h"
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

using namespace ep;

class SDLKernel : public Kernel
{
  EP_DECLARE_COMPONENT(ep, SDLKernel, Kernel, EPKERNEL_PLUGINVERSION, "SDL Kernel instance", 0)
public:
  SDLKernel(Variant::VarMap commandLine);
  ~SDLKernel();

  void RunMainLoop() override final;
  void Quit() override final;

  ViewRef SetFocusView(ViewRef spView) override final;
  void DispatchToMainThread(MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override final;

private:
  static ComponentDescInl *MakeKernelDescriptor();

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
        size_t errorDepth = ErrorLevel();
        try
        {
          d();
          if (ErrorLevel() > errorDepth)
          {
            LogError("Exception occurred in MainThreadCallback : {0}", GetError()->message);
            PopErrorToLevel(errorDepth);
          }
        }
        catch (std::exception &e)
        {
          LogError("Exception occurred in MainThreadCallback : {0}", e.what());
          PopErrorToLevel(errorDepth);
        }
        catch (...)
        {
          LogError("Exception occurred in MainThreadCallback : C++ Exception");
          PopErrorToLevel(errorDepth);
        }
      }
      else if (event.user.code == 1)
      {
        DelegateWithSemaphore *pDispatch = (DelegateWithSemaphore*)event.user.data1;

        MainThreadCallback d;
        d.SetMemento(pDispatch->m);
        size_t errorDepth = ErrorLevel();
        try
        {
          d();
          if (ErrorLevel() > errorDepth)
          {
            LogError("Exception occurred in MainThreadCallback : {0}", GetError()->message);
            PopErrorToLevel(errorDepth);
          }
        }
        catch (std::exception &e)
        {
          LogError("Exception occurred in MainThreadCallback : {0}", e.what());
          PopErrorToLevel(errorDepth);
        }
        catch (...)
        {
          LogError("Exception occurred in MainThreadCallback : C++ Exception");
          PopErrorToLevel(errorDepth);
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
          GetImpl()->spFocusView->Resize(s_displayWidth, s_displayHeight);
          glViewport(0, 0, s_displayWidth, s_displayHeight);
          break;
        }
        break;
      }
      }
    }
  }
}

ComponentDescInl *SDLKernel::MakeKernelDescriptor()
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, epR_AllocFailure, "Memory allocation failed");

  pDesc->info = SDLKernel::ComponentInfo();
  pDesc->info.flags = ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Kernel::ComponentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : CreateHelper<SDLKernel>::GetProperties())
    pDesc->propertyTree.Insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : CreateHelper<SDLKernel>::GetMethods())
    pDesc->methodTree.Insert(m.id, { m, m.pMethod });
  for (auto &e : CreateHelper<SDLKernel>::GetEvents())
    pDesc->eventTree.Insert(e.id, { e, e.pSubscribe });
  for (auto &f : CreateHelper<SDLKernel>::GetStaticFuncs())
    pDesc->staticFuncTree.Insert(f.id, { f, (void*)f.pCall });

  return pDesc;
}
SDLKernel::SDLKernel(Variant::VarMap commandLine)
  : Kernel(SDLKernel::MakeKernelDescriptor(), commandLine)
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

  GetImpl()->InitRender();
}

SDLKernel::~SDLKernel()
{
  GetImpl()->Shutdown();
  // TODO: Consider whether or not to catch exceptions and then continuing the deinit path or just do nothing.
  EventLoop();

  GetImpl()->DeinitRender();

  SDL_GL_DeleteContext(s_context);
  SDL_Quit();
}

void SDLKernel::RunMainLoop()
{
  FinishInit();

  while (!s_done)
  {
    EventLoop();

    // TODO: need to translate input polling into messages...
    epInput_Update();

    // render a frame (this could move to another thread!)
    RenderableViewRef spRenderView = GetImpl()->spFocusView->GetImpl<ViewImpl>()->GetRenderableView();
    if (spRenderView)
      spRenderView->RenderGPU();

    SDL_GL_SwapWindow(s_window);
  }
}

ViewRef SDLKernel::SetFocusView(ViewRef spView)
{
  KernelImpl *pKernelImpl = GetImpl();

  if (!spView)
    pKernelImpl->spFocusView->GetImpl<ViewImpl>()->SetLatestFrame(nullptr);

  ViewRef spOld = pKernelImpl->spFocusView;
  pKernelImpl->spFocusView = spView;

  if (pKernelImpl->spFocusView)
    pKernelImpl->spFocusView->Resize(s_displayWidth, s_displayHeight);
  return spOld;
}

void SDLKernel::DispatchToMainThread(MainThreadCallback callback)
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
void SDLKernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
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

void SDLKernel::Quit()
{
  s_done = true;
}

namespace ep {

Kernel* Kernel::CreateInstanceInternal(Variant::VarMap commandLine)
{
  return KernelImpl::CreateComponentInstance<SDLKernel>(commandLine);
}

} // namespace ep

#else
EPEMPTYFILE
#endif
