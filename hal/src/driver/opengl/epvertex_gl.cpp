#include "driver.h"

#if EPRENDER_DRIVER == EPDRIVER_OPENGL

#include "hal/shader.h"
#include "hal/vertex.h"

#include "ep_opengl.h"

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
// Author: Manu Evans, May 2015
epArrayBuffer* epVertex_CreateIndexBuffer(epArrayDataFormat format)
{
  epArrayBuffer *pIB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat));
  pIB->pFormat = (epArrayDataFormat*)&(pIB[1]);
  pIB->type = epAT_IndexArray;
  *pIB->pFormat = format;
  pIB->numElements = 1;

  glGenBuffers(1, &pIB->buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pIB->buffer);

  return pIB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
epArrayBuffer* epVertex_CreateVertexBuffer(epArrayDataFormat elements[], size_t numElements)
{
  epArrayBuffer *pVB = (epArrayBuffer*)epAlloc(sizeof(epArrayBuffer) + sizeof(epArrayDataFormat)*numElements);
  pVB->pFormat = (epArrayDataFormat*)&(pVB[1]);
  pVB->type = epAT_VertexArray;
  memcpy(pVB->pFormat, elements, sizeof(epArrayDataFormat)*numElements);
  pVB->numElements = numElements;

  glGenBuffers(1, &pVB->buffer);
  glBindBuffer(GL_ARRAY_BUFFER, pVB->buffer);

  return pVB;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_DestroyArrayBuffer(epArrayBuffer **ppBuffer)
{
  glDeleteBuffers(1, &(*ppBuffer)->buffer);
  epFree(*ppBuffer);
  *ppBuffer = nullptr;
}

// ***************************************************************************************
// Author: Manu Evans, May 2015
void epVertex_SetArrayBufferData(epArrayBuffer *pBuffer, const void *pVertexData, size_t bufferLen)
{
  GLenum type = pBuffer->type == epAT_IndexArray ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
  glBindBuffer(type, pBuffer->buffer);
  glBufferData(type, bufferLen, pVertexData, GL_STATIC_DRAW);

  pBuffer->bufferLen = bufferLen;
}

#else
EPEMPTYFILE
#endif // EPRENDER_DRIVER == EPDRIVER_OPENGL
