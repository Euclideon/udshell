#include "driver.h"

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

#include "ep/cpp/kernel.h"
#include "components/view.h"
#include "components/scene.h"
#include "components/nodes/camera.h"

using namespace ep;

// Pepper module
class epPepperModule : public pp::Module
{
public:
  epPepperModule();
  virtual ~epPepperModule();
  virtual pp::Instance* CreateInstance(PP_Instance instance);

  static epPepperModule *gpModule;
};

epPepperModule *epPepperModule::gpModule = nullptr;

class udNewPepperInstance : public pp::Instance, public Kernel
{
public:
  explicit udNewPepperInstance(PP_Instance instance);
  virtual ~udNewPepperInstance();

  epResult InitInternal() override;

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
  void SendToJsCallback(String senderUID, String message, const Variant &data);
};

UDTHREADLOCAL udNewPepperInstance* udNewPepperInstance::pThreadLocalInstance = NULL; // This can't be nullptr it creates a compile error.


// ---------------------------------------------------------------------------------------
Kernel *Kernel::CreateInstanceInternal(Slice<const KeyValuePair> commandLine)
{
  return udNewPepperInstance::pThreadLocalInstance;
}

// ---------------------------------------------------------------------------------------
void udNewPepperInstance::SendToJsCallback(String sender, String message, const Variant &data)
{
  // TODO: Need to wrangle this to include the sender
  SharedString s = data.stringify();
  PostMessageToJS(message.toStringz(), ":%s", (const char*)s.toStringz());
}

// ---------------------------------------------------------------------------------------
epResult udNewPepperInstance::InitInternal()
{
  RegisterMessageHandler("js", MakeDelegate((udNewPepperInstance*)this, &udNewPepperInstance::SendToJsCallback));
  return epR_Success;
}

// ---------------------------------------------------------------------------------------
ViewRef Kernel::SetFocusView(ViewRef spView)
{
  ViewRef spOld = spFocusView;
  spFocusView = spView;
  return spOld;
}

epResult Kernel::RunMainLoop()
{
//  epNaClInstance *pInternal = (epNaClInstance*)pInstance;
//
//  // kick off the main loop (which is asynchronous)
//  pInternal->pPepperInstance->RenderFrame(0);
//
//  // wait for the main loop to terminate
//  udWaitSemaphore(pInternal->pPepperInstance->pTerminateSem, -1);

  return epR_Success;
}

epResult Kernel::Quit()
{
//  epNaClInstance *pInternal = (epNaClInstance*)pInstance;
//
//  // signal to exit the main loop
//  pInternal->pPepperInstance->bQuit = true;

  return epR_Success;
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
static inline int32_t MapPPKeyToEPKey(int32_t )
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

  InputEvent inputEvent;
  switch (pepperEvent.GetType())
  {
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
    case PP_INPUTEVENT_TYPE_MOUSEUP:
    {
      pp::MouseInputEvent mouse_event(pepperEvent);
      inputEvent.deviceType = InputDevice::Mouse;
      inputEvent.deviceId = 0;
      inputEvent.eventType = InputEvent::EventType::Key;

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
      inputEvent.deviceType = InputDevice::Mouse;
      inputEvent.deviceId = 0;
      inputEvent.eventType = InputEvent::EventType::Move;

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
      inputEvent.deviceType = InputDevice::Keyboard;
      inputEvent.deviceId = 0;
      inputEvent.eventType = InputEvent::Text;
      // TODO : Once epInput::TextEvent is fleshed out finish this.
    }
    break;

    case PP_INPUTEVENT_TYPE_KEYDOWN:
    case PP_INPUTEVENT_TYPE_KEYUP:
    {
      pp::KeyboardInputEvent keyEvent(pepperEvent);

      inputEvent.deviceType = InputDevice::Keyboard;
      inputEvent.deviceId = 0;
      inputEvent.eventType = InputEvent::EventType::Key;

      inputEvent.key.key = MapPPKeyToEPKey(keyEvent.GetKeyCode());
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
    String data(str.c_str(), str.size());

    // TODO: js needs to be adapted to include the destination (ie, $webview)
    String msg = data.popToken(":");
    SendMessage("$webview", "$js", msg, data);
  }
}


#if 0
struct epNaClInstance : public epViewerInstance
{
  struct epPepperInstance *pPepperInstance;
};

// pepper classes
class epPepperInstance : public pp::Instance
{
public:
  explicit epPepperInstance(PP_Instance instance);
  virtual ~epPepperInstance();

  virtual bool Init(uint32_t argc, const char* argn[], const char* argv[]);
  virtual void DidChangeView(const pp::View& view); // this needs to route to an external view
  virtual bool HandleInputEvent(const pp::InputEvent& pepperEvent); // route to epInput
  virtual void HandleMessage(const pp::Var& message);

  void InitRenderer();
  void RenderFrame(int32_t);

  int width, height;
  volatile bool bQuit;

  epNaClInstance *pInstance;

  udSemaphore *pTerminateSem;
  udSemaphore *pTerminatedSem;

  pp::Graphics3D glContext;
  pp::CompletionCallbackFactory<epPepperInstance> callbackFactory;

  static uint32_t MainThread(void *pUserData);
  static void JSMessageHandler(const char *pMessage, void *pUserData);
  static void DebugPrintfCallback(const char *pString);
};


epPepperModule *epPepperModule::gpModule = nullptr;

UDTHREADLOCAL epPepperInstance *s_pStartingInstance;


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epViewerInstance* epViewerDriver_CreateInstance()
{
  return epAllocType(epNaClInstance, 1, epAF_Zero);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_Init(epViewerInstance *pInstance)
{
  epNaClInstance *pInternal = (epNaClInstance*)pInstance;

  s_pStartingInstance->pInstance = pInternal;
  pInternal->pPepperInstance = s_pStartingInstance;

  udFile_RegisterNaclHTTP(pInternal->pPepperInstance);

  epGPU_Init();

//  epViewer_ResizeFrame(width, height);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_Deinit(epViewerInstance *pInstance)
{
  epNaClInstance *pInternal = (epNaClInstance*)pInstance;

  // deinit renderer
  //...
}


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
int udMain(int, char*[]);
uint32_t epPepperInstance::MainThread(void *pUserData)
{
  epPepperInstance *pPepper = (epPepperInstance*)pUserData;
  s_pStartingInstance = pPepper;

  glSetCurrentContextPPAPI(pPepper->glContext.pp_resource());

  char *pArgs[] = {
    (char*)"epViewer" // TODO: can I know the pexe name? get it from the frontend?
  };
  udMain(1, pArgs);

  // signal terminat*ed* semaphore
  udIncrementSemaphore(pPepper->pTerminatedSem);

  return 0;
}

void epPepperInstance::DebugPrintfCallback(const char *pString)
{
  epViewer_PostMessage("js", "log", "%s", pString);
  fprintf(stdout, "%s\n", pString);
}

void epPepperInstance::JSMessageHandler(const char *pMessage, void *pUserData)
{
  epPepperInstance *pPepper = (epPepperInstance*)pUserData;
  pPepper->PostMessage(pMessage);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epPepperInstance::epPepperInstance(PP_Instance instance)
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
epPepperInstance::~epPepperInstance()
{
  epViewer_SetCurrentInstance(pInstance);

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
bool epPepperInstance::Init(uint32_t argc, const char* argn[], const char* argv[])
{
  pThreadLocalInstance = this;
  epViewer_RegisterMessageHandler("js", &JSMessageHandler, this);

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
void epPepperInstance::InitRenderer()
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
void epPepperInstance::RenderFrame(int32_t)
{
  epViewer_MainLoop();

  if(bQuit)
  {
    // signal the terminate semaphore causing RunMainLoop to return
    udIncrementSemaphore(pTerminateSem);
  }
  else
  {
    // set a callback to recurse this function on 'SwapBuffers'
    glContext.SwapBuffers(callbackFactory.NewCallback(&epPepperInstance::RenderFrame));
  }
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epPepperInstance::DidChangeView(const pp::View& view)
{
  epViewer_SetCurrentInstance(pInstance);

  width = view.GetRect().width();
  height = view.GetRect().height();

  if (!glContext.is_null())
  {
    int32_t result = glContext.ResizeBuffers(width, height);
    epViewer_ResizeFrame(width, height);
    glViewport(0, 0, width, height);
  }
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
bool epPepperInstance::HandleInputEvent(const pp::InputEvent& event)
{
  epViewer_SetCurrentInstance(pInstance);

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
void epPepperInstance::HandleMessage(const pp::Var& message)
{
  epViewer_SetCurrentInstance(pInstance);

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
epPepperModule::epPepperModule() : pp::Module()
{
  gpModule = this;
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epPepperModule::~epPepperModule()
{
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
pp::Instance* epPepperModule::CreateInstance(PP_Instance instance)
{
  return new udNewPepperInstance(instance);
}

namespace pp
{
  Module* CreateModule() { return new epPepperModule; }
}

#else
EPEMPTYFILE
#endif
