#include "udViewerPlatform.h"

#if UDINPUT_DRIVER == UDDRIVER_PPAPI

#include "udPlatformUtil.h"
#include "udFile.h"

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

#include "../../udViewer_Internal.h"
#include "../OpenGL/udOpenGL_Internal.h"

#include <stdarg.h>


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
  virtual void DidChangeView(const pp::View& view);
  virtual bool HandleInputEvent(const pp::InputEvent& event);
  virtual void HandleMessage(const pp::Var& message);

  void InitRenderer();
  void MainLoop(int32_t);

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

  udGPU_Init();

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
void udViewerDriver_RunMainLoop(udViewerInstance *pInstance)
{
  udNaClInstance *pInternal = (udNaClInstance*)pInstance;

  // kick off the main loop (which is asynchronous)
  pInternal->pPepperInstance->MainLoop(0);

  // wait for the main loop to terminate
//  udWaitSemaphore(pInternal->pPepperInstance->pTerminateSem, -1);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Quit(udViewerInstance *pInstance)
{
  udNaClInstance *pInternal = (udNaClInstance*)pInstance;

  // signal to exit the main loop
  pInternal->pPepperInstance->bQuit = true;
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
void udPepperInstance::MainLoop(int32_t)
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
    glContext.SwapBuffers(callbackFactory.NewCallback(&udPepperInstance::MainLoop));
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
  return new udPepperInstance(instance);
}

namespace pp
{
  Module* CreateModule() { return new udPepperModule(); }
}

#endif
