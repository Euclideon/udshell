#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_PPAPI

#include <stdarg.h>
#include "udPlatformUtil.h"
#include "udFile.h"
#include "udBlockStreamer.h"

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/c/ppb_image_data.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/input_event.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/point.h"
#include "ppapi/cpp/var_dictionary.h"
#include "ppapi/lib/gl/gles2/gl2ext_ppapi.h"
#include "ppapi/utility/completion_callback_factory.h"

#include "../OpenGL/udOpenGL_Internal.h"

#include "kernel.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"

using namespace ep;

// Pepper module
class udPepperModule : public pp::Module
{
public:
  udPepperModule();
  virtual ~udPepperModule();
  virtual pp::Instance* CreateInstance(PP_Instance instance);

  static udPepperModule *gpModule;
};

udPepperModule *udPepperModule::gpModule = nullptr;

class udNewPepperInstance : public pp::Instance, public Kernel
{
public:
  explicit udNewPepperInstance(PP_Instance instance);
  virtual ~udNewPepperInstance();

  udResult InitInternal() override;

  // Pepper overrides
  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& pepperEvent);
  virtual void HandleMessage(const pp::Var& message);

  void InitRenderer();
  void RenderFrame(int32_t);
  // Temporary Plumbing : This will be removed
  void PostMessageToJS(const char *pPrefix, const char* pFormat, ...);

  int width;
  int height;

  pp::Graphics3D glContext;
  pp::CompletionCallbackFactory<udNewPepperInstance> callbackFactory;

  bool hasRenderFrameRun;

  static void DebugPrintfCallback(const char *pString);
  static UDTHREADLOCAL udNewPepperInstance *pThreadLocalInstance;
  void SendToJsCallback(epString senderUID, epString message, const epVariant &data);
};

UDTHREADLOCAL udNewPepperInstance* udNewPepperInstance::pThreadLocalInstance = NULL; // This can't be nullptr it creates a compile error.


// ---------------------------------------------------------------------------------------
Kernel *Kernel::CreateInstanceInternal(epInitParams commandLine)
{
  return udNewPepperInstance::pThreadLocalInstance;
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::SendToJsCallback(epString sender, epString message, const epVariant &data)
{
  // TODO: Need to wrangle this to include the sender
  epSharedString s = data.stringify();
  PostMessageToJS(message.toStringz(), ":%s", (const char*)s.toStringz());
}

// ---------------------------------------------------------------------------------------
udResult udNewPepperInstance::InitInternal()
{
  RegisterMessageHandler("js", MakeDelegate((udNewPepperInstance*)this, &udNewPepperInstance::SendToJsCallback));
  return udR_Success;
}

// ---------------------------------------------------------------------------------------
ViewRef Kernel::SetFocusView(ViewRef spView)
{
  ViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

udResult Kernel::RunMainLoop()
{
//  udNaClInstance *pInternal = (udNaClInstance*)pInstance;
//
//  // kick off the main loop (which is asynchronous)
//  pInternal->pPepperInstance->RenderFrame(0);
//
//  // wait for the main loop to terminate
//  udWaitSemaphore(pInternal->pPepperInstance->pTerminateSem, -1);

  return udR_Success;
}

udResult Kernel::Terminate()
{
//  udNaClInstance *pInternal = (udNaClInstance*)pInstance;
//
//  // signal to exit the main loop
//  pInternal->pPepperInstance->bQuit = true;

  return udR_Success;
}


// ---------------------------------------------------------------------------------------
udNewPepperInstance::udNewPepperInstance(PP_Instance instance)
  : pp::Instance(instance),
    callbackFactory(this),
    hasRenderFrameRun(false)
{
  gpudDebugPrintfOutputCallback = DebugPrintfCallback;
}

// ---------------------------------------------------------------------------------------
udNewPepperInstance::~udNewPepperInstance()
{
}

// Forward declaration of the entry point function
int EntryInit(uint32_t argc, const char* argn[], const char* argv[]);  // Should this be void? Are we going to check the return value and bail?

// ---------------------------------------------------------------------------------------
bool udNewPepperInstance::Init(uint32_t argc, const char* argn[], const char* argv[])
{
  pThreadLocalInstance = this;

  // Move into HAL::Init()??
  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);

  udFile_RegisterNaclHTTP((pp::Instance*)this);

  InitRenderer();

  EntryInit(argc, argn, argv);

  return true;
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::InitRenderer()
{
  if (!glInitializePPAPI(pp::Module::Get()->get_browser_interface()))
  {
    udDebugPrintf("Unable to initialize GL PPAPI!");
    return;
  }

  // HACK! we need an init size...
  int width = 100, height = 100;

  const int32_t attrib_list[] =
  {
    PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
    PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
    PP_GRAPHICS3DATTRIB_WIDTH, width,
    PP_GRAPHICS3DATTRIB_HEIGHT, height,
    PP_GRAPHICS3DATTRIB_NONE
  };

  glContext = pp::Graphics3D(this, attrib_list);
  if (!BindGraphics(glContext))
  {
    udDebugPrintf("Unable to bind 3d context!");
    glContext = pp::Graphics3D();
    glSetCurrentContextPPAPI(0);
    return;
  }

  glSetCurrentContextPPAPI(glContext.pp_resource());
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udNewPepperInstance::DidChangeView(const pp::View& view)
{
  const pp::Rect &rect = view.GetRect();
  width = rect.width();
  height = rect.height();

  if (!glContext.is_null())
  {
    int32_t result = glContext.ResizeBuffers(width, height);
    epUnused(result); // Correctly handle this
    glViewport(0, 0, width, height);

    ViewRef spView = GetFocusView();
    if (spView )
      spView->Resize(width, height);
  }

  if (!hasRenderFrameRun)
  {
    RenderFrame(0);
    hasRenderFrameRun = true;
  }
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::DebugPrintfCallback(const char *pString)
{
  if (pThreadLocalInstance)
    pThreadLocalInstance->PostMessageToJS("log", "%s", pString);

  fprintf(stdout, "%s\n", pString);
}

// ---------------------------------------------------------------------------------------
// Temporary !!!!!!!
void udNewPepperInstance::PostMessageToJS(const char *pPrefix, const char* pFormat, ...)
{
  const int LOG_BUFFER_LEN = 4096;
  char buffer[LOG_BUFFER_LEN];
  va_list args;
  va_start(args, pFormat);
  strcpy(buffer, pPrefix);
  size_t prefixLen = strlen(pPrefix);
  size_t size = sizeof(buffer) - prefixLen;
  vasnprintf(buffer + prefixLen, &size, pFormat, args);
  va_end(args);

  buffer[LOG_BUFFER_LEN - 1] = 0;
  PostMessage(buffer);
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::RenderFrame(int32_t)
{
  ViewRef spView = GetFocusView();

  if (spView)
    spView->Render();

  glContext.SwapBuffers(callbackFactory.NewCallback(&udNewPepperInstance::RenderFrame));
}

// ---------------------------------------------------------------------------------------
static inline int32_t MapPPKeyToUDKey(int32_t )
{
  // TODO: Implement this
  return 0;
}
// ---------------------------------------------------------------------------------------
bool udNewPepperInstance::HandleInputEvent(const pp::InputEvent& pepperEvent)
{
  ViewRef spView = GetFocusView();
  if (!spView)
    return false;

  epInputEvent inputEvent;
  switch (pepperEvent.GetType())
  {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    case PP_INPUTEVENT_TYPE_MOUSEUP:
    {
      pp::MouseInputEvent mouse_event(pepperEvent);
      inputEvent.deviceType = epID_Mouse;
      inputEvent.deviceId = 0;
      inputEvent.eventType = epInputEvent::Key;

      PP_InputEvent_MouseButton button = mouse_event.GetButton();
      int32_t key = -1;
      switch (button)
      {
        case PP_INPUTEVENT_MOUSEBUTTON_LEFT:
          key = 0; // TODO when KeyEvent enum is defined
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_MIDDLE:
          key = 0; // TODO when KeyEvent enum is defined
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_RIGHT:
          key = 0; // TODO when KeyEvent enum is defined
          break;
        case PP_INPUTEVENT_MOUSEBUTTON_NONE:
        {
          // Do we fall through to PP_INPUTEVENT_TYPE_MOUSEMOVE ?
          // goto PP_INPUTEVENT_TYPE_MOUSEMOVE; ? Asking this question based off of code from udWebViewInstance::HandleInputEvent() Release_0.15
        }
      }

      inputEvent.key.state = key;
      inputEvent.key.state = pepperEvent.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN ? 1 : 0;
      spView->InputEvent(inputEvent);
    }
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
    {
      pp::MouseInputEvent mouse_event(pepperEvent);
      inputEvent.deviceType = epID_Mouse;
      inputEvent.deviceId = 0;
      inputEvent.eventType = epInputEvent::Move;

      pp::Point delta = mouse_event.GetMovement();
      pp::Point absolute = mouse_event.GetPosition();

      inputEvent.move.xAbsolute = absolute.x();
      inputEvent.move.yAbsolute = absolute.y();
      inputEvent.move.xDelta = delta.x();
      inputEvent.move.yDelta = delta.y();
      spView->InputEvent(inputEvent);
    }
    break;

    case PP_INPUTEVENT_TYPE_CHAR:
    {
      pp::KeyboardInputEvent keyEvent(pepperEvent);
      inputEvent.deviceType = epID_Keyboard;
      inputEvent.deviceId = 0;
      inputEvent.eventType = epInputEvent::Text;
      // TODO : Once epInput::TextEvent is fleshed out finish this.
    }
    break;

    case PP_INPUTEVENT_TYPE_KEYDOWN:
    case PP_INPUTEVENT_TYPE_KEYUP:
    {
      pp::KeyboardInputEvent keyEvent(pepperEvent);

      inputEvent.deviceType = epID_Keyboard;
      inputEvent.deviceId = 0;
      inputEvent.eventType = epInputEvent::Key;

      inputEvent.key.key = MapPPKeyToUDKey(keyEvent.GetKeyCode());
      inputEvent.key.state = pepperEvent.GetType() == PP_INPUTEVENT_TYPE_KEYDOWN ? 1 : 0;
      spView->InputEvent(inputEvent);
    }
    break;

    case PP_INPUTEVENT_TYPE_MOUSEENTER:
      // TODO : Implement this.
      break;

    case PP_INPUTEVENT_TYPE_MOUSELEAVE:
      // TODO : Implement this.
      break;

  default:
    break;
  }
  return true;
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::HandleMessage(const pp::Var& message)
{
  if (message.is_string())
  {
    std::string str = message.AsString();
    epString data(str.c_str(), str.size());

    // TODO: js needs to be adapted to include the destination (ie, $webview)
    epString msg = data.popToken(":");
    SendMessage("$webview", "$js", msg, data);
  }
}


#if 0
struct udNaClInstance : public udViewerInstance
{
  struct udPepperInstance *pPepperInstance;
};

// pepper classes
class udPepperInstance : public pp::Instance
{
public:
  explicit udPepperInstance(PP_Instance instance);
  virtual ~udPepperInstance();

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view); // this needs to route to an external view
  virtual bool HandleInputEvent(const pp::InputEvent& pepperEvent); // route to epInput
  virtual void HandleMessage(const pp::Var& message);

  void InitRenderer();
  void RenderFrame(int32_t);

  int width, height;
  volatile bool bQuit;

  udNaClInstance *pInstance;

  udSemaphore *pTerminateSem;
  udSemaphore *pTerminatedSem;

  pp::Graphics3D glContext;
  pp::CompletionCallbackFactory<udPepperInstance> callbackFactory;

  static uint32_t MainThread(void *pUserData);
  static void JSMessageHandler(const char *pMessage, void *pUserData);
  static void DebugPrintfCallback(const char *pString);
};


udPepperModule *udPepperModule::gpModule = nullptr;

UDTHREADLOCAL udPepperInstance *s_pStartingInstance;


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udViewerInstance* udViewerDriver_CreateInstance()
{
  return udAllocType(udNaClInstance, 1, udAF_Zero);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Init(udViewerInstance *pInstance)
{
  udNaClInstance *pInternal = (udNaClInstance*)pInstance;

  s_pStartingInstance->pInstance = pInternal;
  pInternal->pPepperInstance = s_pStartingInstance;

  udFile_RegisterNaclHTTP(pInternal->pPepperInstance);

  epGPU_Init();

//  udViewer_ResizeFrame(width, height);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Deinit(udViewerInstance *pInstance)
{
  udNaClInstance *pInternal = (udNaClInstance*)pInstance;

  // deinit renderer
  //...
}


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
int udMain(int, char*[]);
uint32_t udPepperInstance::MainThread(void *pUserData)
{
  udPepperInstance *pPepper = (udPepperInstance*)pUserData;
  s_pStartingInstance = pPepper;

  glSetCurrentContextPPAPI(pPepper->glContext.pp_resource());

  char *pArgs[] = {
    (char*)"udViewer" // TODO: can I know the pexe name? get it from the frontend?
  };
  udMain(1, pArgs);

  // signal terminat*ed* semaphore
  udIncrementSemaphore(pPepper->pTerminatedSem);

  return 0;
}

void udPepperInstance::DebugPrintfCallback(const char *pString)
{
  udViewer_PostMessage("js", "log", "%s", pString);
  fprintf(stdout, "%s\n", pString);
}

void udPepperInstance::JSMessageHandler(const char *pMessage, void *pUserData)
{
  udPepperInstance *pPepper = (udPepperInstance*)pUserData;
  pPepper->PostMessage(pMessage);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udPepperInstance::udPepperInstance(PP_Instance instance)
  : pp::Instance(instance)
  , width(0)
  , height(0)
  , bQuit(false)
  , pInstance(nullptr)
  , pTerminateSem(nullptr)
  , pTerminatedSem(nullptr)
  , callbackFactory(this)
{
  gpudDebugPrintfOutputCallback = DebugPrintfCallback;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udPepperInstance::~udPepperInstance()
{
  udViewer_SetCurrentInstance(pInstance);

  // signal to return from main
  bQuit = true;

  if (pTerminatedSem)
  {
    // wait for main thread to return
    udWaitSemaphore(pTerminatedSem, -1);
    udDestroySemaphore(&pTerminatedSem);
  }
  if (pTerminateSem)
    udDestroySemaphore(&pTerminateSem);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
bool udPepperInstance::Init(uint32_t argc, const char* argn[], const char* argv[])
{
  pThreadLocalInstance = this;
  udViewer_RegisterMessageHandler("js", &JSMessageHandler, this);

  // TODO: communicate with frontend?
  //... get commandline (url) args, get pexe name

  RequestFilteringInputEvents(PP_INPUTEVENT_CLASS_KEYBOARD);
  RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE);

  InitRenderer();

  pTerminateSem = udCreateSemaphore(1, 0);
  pTerminatedSem = udCreateSemaphore(1, 0);

//  udCreateThread(MainThread, this);
  MainThread(this);

  return true;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udPepperInstance::InitRenderer()
{
  if (!glInitializePPAPI(pp::Module::Get()->get_browser_interface()))
  {
    udDebugPrintf("Unable to initialize GL PPAPI!");
    return;
  }

  // HACK! we need an init size...
  int width = 100, height = 100;

  const int32_t attrib_list[] =
  {
    PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
    PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 24,
    PP_GRAPHICS3DATTRIB_WIDTH, width,
    PP_GRAPHICS3DATTRIB_HEIGHT, height,
    PP_GRAPHICS3DATTRIB_NONE
  };

  glContext = pp::Graphics3D(this, attrib_list);
  if (!BindGraphics(glContext))
  {
    udDebugPrintf("Unable to bind 3d context!");
    glContext = pp::Graphics3D();
    glSetCurrentContextPPAPI(0);
    return;
  }

  glSetCurrentContextPPAPI(glContext.pp_resource());
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udPepperInstance::RenderFrame(int32_t)
{
  udViewer_MainLoop();

  if(bQuit)
  {
    // signal the terminate semaphore causing RunMainLoop to return
    udIncrementSemaphore(pTerminateSem);
  }
  else
  {
    // set a callback to recurse this function on 'SwapBuffers'
    glContext.SwapBuffers(callbackFactory.NewCallback(&udPepperInstance::RenderFrame));
  }
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udPepperInstance::DidChangeView(const pp::View& view)
{
  udViewer_SetCurrentInstance(pInstance);

  width = view.GetRect().width();
  height = view.GetRect().height();

  if (!glContext.is_null())
  {
    int32_t result = glContext.ResizeBuffers(width, height);
    udViewer_ResizeFrame(width, height);
    glViewport(0, 0, width, height);
  }
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
bool udPepperInstance::HandleInputEvent(const pp::InputEvent& event)
{
  udViewer_SetCurrentInstance(pInstance);

  switch (event.GetType())
  {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
    {
      pp::MouseInputEvent mouse_event(event);

      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_NONE)
        return true;
//      mouse_ = mouse_event.GetPosition();
//      mouse_down_ = true;
      break;
    }
    case PP_INPUTEVENT_TYPE_MOUSEUP:
//      mouse_down_ = false;
      break;

    case PP_INPUTEVENT_TYPE_CHAR:
    {
      pp::KeyboardInputEvent key_event(event);
      //const char *str = key_event.GetCharacterText().DebugString().c_str();
      //udDebugPrintf("Keypress: keycode=%d chartext.asint=%d chartext.debugstring=%s\n", key_event.GetKeyCode(), key_event.GetCharacterText().AsInt(), str);
      break;
    }

    case PP_INPUTEVENT_TYPE_KEYDOWN:
    {
      pp::KeyboardInputEvent key_event(event);
//      int code = key_event.GetKeyCode();
//      if (code >= 0 && code < 256)
//      {
//        //bool before=ud.IsKeyDown(code);
//        SetKeyState(code, true);
//        //bool after=ud.IsKeyDown(code);
//        //udDebugPrintf("keydown event: %c/%d before:%s after:%s\n", code, code, before?"down":"up", after?"down":"up");
//      }
      break;
    }

    case PP_INPUTEVENT_TYPE_KEYUP:
    {
      pp::KeyboardInputEvent key_event(event);
//      int code = key_event.GetKeyCode();
//      if (code >= 0 && code < 256)
//      {
//        //bool before=ud.IsKeyDown(code);
//        SetKeyState(code, false);
//        //bool after=ud.IsKeyDown(code);
//        //udDebugPrintf("keyup event: %c/%d before:%s after:%s\n", code, code, before?"down":"up", after?"down":"up");
//      }
      break;
    }

    default:
      break;
  }

  return true;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udPepperInstance::HandleMessage(const pp::Var& message)
{
  udViewer_SetCurrentInstance(pInstance);

  if(pInstance->initParams.pReceiveMessage)
  {
    if(message.is_string())
    {
      std::string str = message.AsString();
      pInstance->initParams.pReceiveMessage(str.c_str(), pInstance->data.pUserData);
    }
  }
}


#endif
// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udPepperModule::udPepperModule() : pp::Module()
{
  gpModule = this;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udPepperModule::~udPepperModule()
{
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
pp::Instance* udPepperModule::CreateInstance(PP_Instance instance)
{
  return udNew(udNewPepperInstance, instance);
}

namespace pp
{
  Module* CreateModule() { return udNew(udPepperModule); }
}

#endif
