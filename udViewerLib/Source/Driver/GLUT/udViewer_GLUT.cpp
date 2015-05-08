#include "udViewerPlatform.h"

#if UDINPUT_DRIVER == UDDRIVER_GLUT

#include "udGLUT_Internal.h"
#include "../../udViewer_Internal.h"

void udGPU_Init();


// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
static void udViewerGLUT_Resize(int width, int height)
{
  udViewer_ResizeFrame(width, height);

  // Update the view port with the new screen dimensions and cause a refresh
  glViewport(0, 0, width, height);
  glutPostRedisplay();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
static void udViewerGLUT_Display()
{
  udViewer_MainLoop();

  glutSwapBuffers();
  glutPostRedisplay();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Init(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  // TODO: split this out into udWindow or something
  glutInitWindowSize(1280, 720);           // TODO: get these values from somewhere
  glutCreateWindow("udPointCloud Viewer"); // TODO: get this string from somewhere
  glutDisplayFunc(udViewerGLUT_Display);
  glutReshapeFunc(udViewerGLUT_Resize);

  udGPU_Init();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_RunMainLoop()
{
  glutMainLoop();
}

// ---------------------------------------------------------------------------------------
// Author: Manu Evans, May 2015
void udViewerDriver_Quit()
{
  glutLeaveMainLoop();
}

#endif
