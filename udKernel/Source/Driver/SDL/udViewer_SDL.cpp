#include "udDriver.h"

#if UDINPUT_DRIVER == UDDRIVER_SDL

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "../../udViewer_Internal.h"

void udGPU_Init();

SDL_Window* s_window = nullptr;
SDL_GLContext s_context = nullptr;
bool s_done = false;

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
udViewerInstance* udViewerDriver_CreateInstance()
{
  return udAllocType(udViewerInstance, 1, udAF_Zero);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Init(udViewerInstance *)
{
  SDL_Init(SDL_INIT_VIDEO);
  s_window = SDL_CreateWindow("udPointCloud Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
  SDL_GL_CreateContext(s_window);

  udGPU_Init();

  udViewer_ResizeFrame(1280, 720);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Deinit(udViewerInstance *)
{
  // deinit renderer
  //...
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_RunMainLoop(udViewerInstance *)
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
          switch(event.window.event)
          {
            case SDL_WINDOWEVENT_RESIZED:
              udViewer_ResizeFrame(event.window.data1, event.window.data2);
              glViewport(0, 0, event.window.data1, event.window.data2);
              break;
          }
          break;
        }
      }
    }
    udViewer_MainLoop();
    SDL_GL_SwapWindow(s_window);
  }

  // this
  SDL_GL_DeleteContext(s_context);
  SDL_Quit();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Quit(udViewerInstance *)
{
  s_done = true;
}

#endif
