#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "udKernel.h"
#include "udView.h"

#include "../../udViewer_Internal.h"

void udGPU_Init();

SDL_Window* s_window = nullptr;
SDL_GLContext s_context = nullptr;
bool s_done = false;
static int s_displayWidth, s_displayHeight;

udView *pView;



udKernel *udKernel::CreateInstanceInternal(udInitParams commandLine)
{
  s_displayWidth = 1280;
  s_displayHeight = 720;

  SDL_Init(SDL_INIT_VIDEO);
  s_window = SDL_CreateWindow("udPointCloud Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, s_displayWidth, s_displayHeight, SDL_WINDOW_OPENGL);
  SDL_GL_CreateContext(s_window);

  udGPU_Init();

  return new udKernel;
}

udResult udKernel::InitInstanceInternal()
{
  return InitRender();
}

udResult udKernel::DestroyInstanceInternal()
{
  // this
  SDL_GL_DeleteContext(s_context);
  SDL_Quit();

  delete this;
  return udR_Success;
}

udView *udKernel::SetFocusView(udView *pView)
{
  udView *pOld = pFocusView;
  pFocusView = pView;
  pFocusView->Resize(s_displayWidth, s_displayHeight);
  return pOld;
}

udResult udKernel::RunMainLoop()
{
  while (!s_done)
  {
    SDL_Event event;
    if (SDL_PollEvent(&event))
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
              pView->Resize(s_displayWidth, s_displayHeight);
              glViewport(0, 0, s_displayWidth, s_displayHeight);
              break;
          }
          break;
        }
      }
    }

    // TODO: need to translate input polling into messages...
    udInput_Update();

    pView->Render();
    SDL_GL_SwapWindow(s_window);
  }
  return udR_Success;
}

udResult udKernel::Terminate()
{
  s_done = true;
  return udR_Success;
}

#endif
