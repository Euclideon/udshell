#include "gtest/gtest.h"
#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"
#include "ep/cpp/variant.h"

namespace ep {
namespace internal {

extern bool gUnitTesting;

void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line);
void *_AllocAligned(size_t size, size_t alignment, epAllocationFlags flags, const char * pFile, int line);
void _Free(void *pMemory);

} // namespace internal
} // namespace ep

static AVLTreeAllocator<VariantAVLNode> s_varAVLAllocator;

static Instance s_intance =
{
  EP_APIVERSION,  // apiVersion;
  nullptr,        // pKernelInstance;

  [](size_t size, epAllocationFlags flags, const char* pFile, int line) -> void* { return ep::internal::_Alloc(size, flags, pFile, line); }, //  Alloc

  [](size_t size, size_t alignment, epAllocationFlags flags, const char *pFile, int line) -> void* { return ep::internal::_AllocAligned(size, alignment, flags, pFile, line); }, // AllocAligned

  [](void *pMem) -> void { ep::internal::_Free(pMem); }, // Free

  [](String condition, String message, String file, int line) -> void { IF_EPASSERT(epAssertFailed(condition, message, file, line);) }, // AssertFailed

  [](Component *) -> void { }, // DestroyComponent, dec it with the internal function which actually performs the cleanup

  []() -> void* { return (void*)&s_varAVLAllocator; }, // TreeAllocator
};

struct GlobalInstanceInitializer
{
  GlobalInstanceInitializer()
  {
    ep::s_pInstance = &s_intance;
  }
};

GlobalInstanceInitializer globalInstanceInitializer;

int main(int argc, char **argv)
{
  internal::gUnitTesting = true;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
