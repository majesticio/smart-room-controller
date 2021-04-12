#ifndef _TIMER_H_ // if not defined
#define _TIMER_H_

class Timer {

    unsigned int _timerStart;
    unsigned int _timerTarget;
    public:
      void startTimer(unsigned int msec) {
      _timerStart = millis();
      _timerTarget = msec;
    }

      bool isTimerReady() {
        if ((millis() - _timerStart) >_timerTarget) {
          return true;
        }
        else {
          return false;
        }
    }
};
#endif // _TIMER_H_
