#include "ep/c/plugin.h"
#include "ep/c/kernel.h"

extern "C" {

epResult epKernel_SendMessage(epString target, epString sender, epString message, const epVariant* pData)
{
  return s_pPluginInstance->pKernelAPI->pSendMessage(s_pPluginInstance->pKernelInstance, target, sender, message, pData);
}

epResult epKernel_RegisterComponentType(const epComponentDesc *pDesc)
{
  return s_pPluginInstance->pKernelAPI->pRegisterComponentType(s_pPluginInstance->pKernelInstance, pDesc);
}

epResult epKernel_CreateComponent(epString typeId, const epKeyValuePair *pInitParams, size_t numInitParams, epComponent **ppNewInstance)
{
  return s_pPluginInstance->pKernelAPI->pCreateComponent(s_pPluginInstance->pKernelInstance, typeId, pInitParams, numInitParams, ppNewInstance);
}

epComponent* epKernel_FindComponent(epString uid)
{
  return s_pPluginInstance->pKernelAPI->pFindComponent(s_pPluginInstance->pKernelInstance, uid);
}

void epKernel_Exec(epString code)
{
  s_pPluginInstance->pKernelAPI->pExec(s_pPluginInstance->pKernelInstance, code);
}

void epKernel_LogError(const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogError(s_pPluginInstance->pKernelInstance, text, componentUID);
}

void epKernel_LogWarning(int level, const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogWarning(s_pPluginInstance->pKernelInstance, level, text, componentUID);
}

void epKernel_LogDebug(int level, const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogDebug(s_pPluginInstance->pKernelInstance, level, text, componentUID);
}

void epKernel_LogInfo(int level, const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogInfo(s_pPluginInstance->pKernelInstance, level, text, componentUID);
}

void epKernel_LogScript(const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogScript(s_pPluginInstance->pKernelInstance, text, componentUID);
}

void epKernel_LogTrace(const epString text, const epString componentUID)
{
  s_pPluginInstance->pKernelAPI->pLogTrace(s_pPluginInstance->pKernelInstance, text, componentUID);
}

}
