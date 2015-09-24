#include "hal/driver.h"

#if EPWINDOW_DRIVER == EPDRIVER_QT

#include "hal/haltimer.h"
#include <QTimer>

class HalTimerQT : public QObject, public HalTimer
{
public slots:
  void OnTimeout()
  {
    pCallBack(this, pUserData);
    elapsedCount++;
  }

  HalTimerCallback *pCallBack;
  void *pUserData;

  QTimer timer;
};


HalTimer *HalTimer_Create(HalTimerType tt, uint32_t duration,  HalTimerCallback *pCallBack, void *pUserData)
{
  HalTimerQT *pTimer = new HalTimerQT;
  if (!pTimer)
    return nullptr;

  pTimer->duration = duration;
  pTimer->timerType = tt;
  pTimer->pCallBack = pCallBack;
  pTimer->pUserData = pUserData;

  pTimer->elapsedCount = 0;
  pTimer->timer.setSingleShot(tt == CountDown ? true : false);

  QMetaObject::Connection connection = QObject::connect(&pTimer->timer, &QTimer::timeout, pTimer, &HalTimerQT::OnTimeout);
  if (!connection)
  {
    delete pTimer;
    return nullptr;
  }

  pTimer->timer.start(duration);

  return pTimer;
}


void HalTimer_Destroy(HalTimer **ppTimer)
{
  if (ppTimer && *ppTimer)
  {
    HalTimerQT *pTimer = (HalTimerQT*)*ppTimer;
    pTimer->timer.stop();
    QObject::disconnect(&pTimer->timer, &QTimer::timeout, pTimer, &HalTimerQT::OnTimeout);

    delete pTimer;
    *ppTimer = nullptr;
  }
}

#endif // EPWINDOW_DRIVER == EPDRIVER_QT
