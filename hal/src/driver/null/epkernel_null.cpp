#include "driver.h"

#if EPKERNEL_DRIVER == EPDRIVER_NULL

#include "kernelimpl.h"

#if defined(EP_WINDOWS)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# include <GL/gl.h>
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
  {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }
  static void CreateGLContext()
  {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc{ 0, WindowProc, 0, 0, hInstance, NULL, NULL, NULL, NULL, L"epdaemon" };
    RegisterClass(&wc);
    HWND hWnd = CreateWindow(L"epdaemon", L"Daemon", WS_POPUP, 0, 0, 16, 16, NULL, NULL, hInstance, NULL);
    HDC hDC = GetDC(hWnd);

    PIXELFORMATDESCRIPTOR pf{
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
      PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
      24,                       //Colordepth of the framebuffer.
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      24,                       //Number of bits for the depthbuffer
      8,                        //Number of bits for the stencilbuffer
      0,                        //Number of Aux buffers in the framebuffer.
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
    };
    int fmt = ChoosePixelFormat(hDC, &pf);
    SetPixelFormat(hDC, fmt, &pf);

    HGLRC hRC = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hRC);
    SetActiveWindow(hWnd);
  }
#elif defined(EP_LINUX)
  static void CreateGLContext()
  {
    EPASSERT(false, "TODO");
  }
#endif

namespace ep {

bool s_done = false;

class NullKernel : public Kernel
{
  EP_DECLARE_COMPONENT(ep, NullKernel, Kernel, EPKERNEL_PLUGINVERSION, "Generic Kernel instance", 0)
public:
  NullKernel(Variant::VarMap commandLine);
  ~NullKernel();

  void RunMainLoop() override final;
  void Quit() override final;

  void DispatchToMainThread(MainThreadCallback callback) override final;
  void DispatchToMainThreadAndWait(MainThreadCallback callback) override final;

private:
  static ComponentDescInl *MakeKernelDescriptor();
};

ComponentDescInl *NullKernel::MakeKernelDescriptor()
{
  ComponentDescInl *pDesc = epNew(ComponentDescInl);
  EPTHROW_IF_NULL(pDesc, Result::AllocFailure, "Memory allocation failed");

  pDesc->info = NullKernel::componentInfo();
  pDesc->info.flags = ep::ComponentInfoFlags::Unregistered;
  pDesc->baseClass = Kernel::componentID();

  pDesc->pInit = nullptr;
  pDesc->pCreateInstance = nullptr;
  pDesc->pCreateImpl = nullptr;
  pDesc->pSuperDesc = nullptr;

  // build search trees
  for (auto &p : NullKernel::getPropertiesImpl())
    pDesc->propertyTree.insert(p.id, { p, p.pGetterMethod, p.pSetterMethod });
  for (auto &m : NullKernel::getMethodsImpl())
    pDesc->methodTree.insert(m.id, { m, m.pMethod });
  for (auto &e : NullKernel::getEventsImpl())
    pDesc->eventTree.insert(e.id, { e, e.pSubscribe });
  for (auto &f : NullKernel::getStaticFuncsImpl())
    pDesc->staticFuncTree.insert(f.id, { f, (void*)f.pCall });

  return pDesc;
}

NullKernel::NullKernel(Variant::VarMap commandLine)
  : Kernel(NullKernel::MakeKernelDescriptor(), commandLine)
{

  // TODO: init stuff?


  // create offscreen GL context
  CreateGLContext();

  GetImpl()->InitRender();

  // this is usually performed at the start of mainloop...
  // but since daemon may not run a mainloop, it should happen here.
  FinishInit();
}

NullKernel::~NullKernel()
{
  GetImpl()->Shutdown();

  GetImpl()->DeinitRender();
}

void NullKernel::RunMainLoop()
{
  while (!s_done)
  {
    // TODO: wait for events... (DispatchToMainThread, timers, etc)
#if defined(EP_WINDOWS)
    Sleep(16);
#endif
  }
}

void NullKernel::Quit()
{
  s_done = true;
}

void NullKernel::DispatchToMainThread(MainThreadCallback callback)
{
  EPASSERT(false, "TODO");
}

void NullKernel::DispatchToMainThreadAndWait(MainThreadCallback callback)
{
  EPASSERT(false, "TODO");
}

Kernel* Kernel::CreateInstanceInternal(Variant::VarMap commandLine)
{
  return KernelImpl::CreateComponentInstance<NullKernel>(commandLine);
}

}

#else
EPEMPTYFILE
#endif
