#include "driver.h"

#if EPKERNEL_DRIVER == EPDRIVER_GLUT

#include "udGLUT_Internal.h"

void epGPU_Init();


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
static void epViewerGLUT_Resize(int width, int height)
{
  epViewer_ResizeFrame(width, height);

  // Update the view port with the new screen dimensions and cause a refresh
  glViewport(0, 0, width, height);
  glutPostRedisplay();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
static void epViewerGLUT_Display()
{
  epViewer_MainLoop();

  glutSwapBuffers();
  glutPostRedisplay();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
epViewerInstance* epViewerDriver_CreateInstance()
{
  return epAllocType(epViewerInstance, 1, epAF_Zero);
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_Init(epViewerInstance *pInstance)
{
  glutInit(&pInstance->initParams.argc, pInstance->initParams.argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  // TODO: split this out into udWindow or something
  glutInitWindowSize(1280, 720);           // TODO: get these values from somewhere
  glutCreateWindow("udPointCloud Viewer"); // TODO: get this string from somewhere
  glutDisplayFunc(epViewerGLUT_Display);
  glutReshapeFunc(epViewerGLUT_Resize);

  epGPU_Init();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_Deinit(epViewerInstance *)
{
  // deinit renderer
  //...
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_RunMainLoop(epViewerInstance *)
{
  glutMainLoop();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void epViewerDriver_Quit(epViewerInstance *)
{
  glutLeaveMainLoop();
}

#else
EPEMPTYFILE
#endif
