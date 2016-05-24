#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_QT

#include "hal/vertex.h"
#include "hal/shader.h"

#include "eprender_qt.h"

#include "ep/c/error.h"

#include <QOpenGLBuffer>

// ***************************************************************************************
epShaderInputConfig *epVertex_CreateShaderInputConfig(const epArrayElement *pElementArray, size_t numElements, epShaderProgram *pProgram)
{
  // Note that this will over alloc when not all attributes are present in the shader but who cares
  size_t size = sizeof(epShaderInputConfig) + (sizeof(epArrayElement) + sizeof(epArrayElementData))*numElements;
  epShaderInputConfig *pCfg = (epShaderInputConfig*)epAlloc(size);
  pCfg->pElements = (epArrayElement*)(pCfg + 1);
  pCfg->pElementData = (epArrayElementData*)(pCfg->pElements + numElements);

  int count = 0;
  for (size_t i = 0; i < numElements; ++i)
  {
    size_t numAttribs = epShader_GetNumAttributes(pProgram);
    for (size_t j = 0; j < numAttribs; ++j)
    {
      if (strncmp(pElementArray[i].attributeName, epShader_GetAttributeName(pProgram, j), sizeof(pElementArray[i].attributeName)) == 0)
      {
        // TODO: Add some error checking to ensure the the array elemnt matches or can be unpacked to match the shader attribute.
        memcpy(&pCfg->pElements[count], &pElementArray[i], sizeof(pElementArray[i]));
        pCfg->pElementData[count].attribLocation = epShader_GetAttributeType(pProgram, j).location;
        count++;
        break;
      }
    }
  }

  pCfg->numElements = count;
  return pCfg;
}

// ***************************************************************************************
void epVertex_DestroyShaderInputConfig(epShaderInputConfig **ppCfg)
{
  epFree(*ppCfg);
  *ppCfg = nullptr;
}

// ***************************************************************************************
void epVertex_GetShaderInputConfigStreams(const epShaderInputConfig *pConfig, int *pStreams, size_t streamsLength, int *pNumStreams)
{
  size_t streamFoundSize = sizeof(bool) * pConfig->numElements;
  bool *streamFound = (bool*)alloca(streamFoundSize);
  memset(streamFound, 0, streamFoundSize);

  int numStreams = 0;

  for (int i = 0; i < pConfig->numElements && numStreams < (int)streamsLength; ++i)
  {
    int stream = pConfig->pElements[i].stream;
    if (!streamFound[stream])
    {
      streamFound[stream] = true;
      pStreams[numStreams++] = stream;
    }
  }
  *pNumStreams = numStreams;
}

// ***************************************************************************************
epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format)
{
  bool result = true;
  epArrayBuffer *pIB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  EP_ERROR_IF(!pQtBuffer->create(), false);
  EP_ERROR_IF(!pQtBuffer->bind(), false);

  pIB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat));
  pIB->pFormat = (epArrayDataFormat*)&pIB[1];
  *pIB->pFormat = format;
  pIB->numElements = 1;
  pIB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    epDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pIB;
}

// ***************************************************************************************
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements)
{
  bool result = true;
  epArrayBuffer *pVB = nullptr;
  QOpenGLBuffer *pQtBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  EP_ERROR_IF(!pQtBuffer->create(), false);
  EP_ERROR_IF(!pQtBuffer->bind(), false);

  pVB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*numElements);
  pVB->pFormat = (epArrayDataFormat*)&pVB[1];
  memcpy(pVB->pFormat, elements, sizeof(epArrayDataFormat)*numElements);
  pVB->numElements = numElements;
  pVB->pBuffer = pQtBuffer;

epilogue:
  if (!result)
  {
    epDebugPrintf("Error creating vertex buffer\n");
    delete pQtBuffer;
  }
  return pVB;
}

// ***************************************************************************************
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppBuffer)
{
  (*ppBuffer)->pBuffer->release();
  (*ppBuffer)->pBuffer->destroy();
  delete (*ppBuffer)->pBuffer;
  epFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
void epVertex_SetArrayBufferData(epArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  // TODO: remove these checks once we are confident in Kernel and the Qt driver
  EPASSERT(pBuffer->pBuffer != nullptr, "Qt buffer doesn't exist");

  pBuffer->pBuffer->bind();
  pBuffer->pBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  pBuffer->pBuffer->allocate(pVertexData, static_cast<int>(bufferLen));
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_QT
