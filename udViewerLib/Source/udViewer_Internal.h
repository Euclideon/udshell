#include "udViewer.h"

struct RegisteredCallback
{
  udViewerCallback *pCallback;
  void *pUserData;
};

void udViewer_MainLoop();
void udViewer_BeginFrame();
void udViewer_EndFrame();
void udViewer_Update();
void udViewer_Draw();

void udViewer_ResizeFrame(int width, int height);
