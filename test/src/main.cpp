#include "gtest/gtest.h"
#include "ep/cpp/platform.h"
#include "ep/cpp/plugin.h"
#include "ep/cpp/variant.h"
#include "ep/cpp/hashmap.h"
#include "ep/cpp/safeptr.h"

namespace ep {
namespace internal {

extern bool gUnitTesting;

void *_Alloc(size_t size, epAllocationFlags flags, const char * pFile, int line);
void *_AllocAligned(size_t size, size_t alignment, epAllocationFlags flags, const char * pFile, int line);
void _Free(void *pMemory);

} // namespace internal
} // namespace ep

static ep::AVLTreeAllocator<ep::VariantAVLNode> s_varAVLAllocator;
static ep::HashMap<ep::internal::SafeProxy<void>*, void*, ep::internal::PointerHash> s_weakRefRegistry(65536);

static ep::Instance s_instance =
{
  EP_APIVERSION,  // apiVersion;
  nullptr,        // pKernelInstance;

  [](size_t size, epAllocationFlags flags, const char* pFile, int line) -> void* { return ep::internal::_Alloc(size, flags, pFile, line); }, //  Alloc

  [](size_t size, size_t alignment, epAllocationFlags flags, const char *pFile, int line) -> void* { return ep::internal::_AllocAligned(size, alignment, flags, pFile, line); }, // AllocAligned

  [](void *pMem) -> void { ep::internal::_Free(pMem); }, // Free

  [](ep::String condition, ep::String message, ep::String file, int line) -> void { IF_EPASSERT(epAssertFailed(condition, message, file, line);) }, // AssertFailed

  [](ep::Component *) -> void { }, // DestroyComponent, dec it with the internal function which actually performs the cleanup

  []() -> void* { return (void*)&s_varAVLAllocator; }, // TreeAllocator

  []() -> void* { return (void*)&s_weakRefRegistry; } // weakRefRegistry

};

struct GlobalInstanceInitializer
{
  GlobalInstanceInitializer()
  {
    ep::s_pInstance = &s_instance;
  }
};

GlobalInstanceInitializer globalInstanceInitializer;

int main(int argc, char **argv)
{
  ep::internal::gUnitTesting = true;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
