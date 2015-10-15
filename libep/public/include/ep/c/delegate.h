#if !defined(_EPDELEGATE_H)
#define _EPDELEGATE_H

#if defined(__cplusplus)
extern "C" {
#endif

struct epDelegate
{
  void *memento = nullptr;
};

#if defined(__cplusplus)
}
#endif

#endif // _EPDELEGATE_H
