#ifndef TASKS_h
#define TASKS_h

#define TASK_LIST_MAX_LENGTH 10

struct continuous_task {
  uint32_t duration; //ms
  uint8_t (*available)();
  void (*execute)();
};

struct periodical_task {
  uint32_t period;  //ms
  void (*execute)();
  uint32_t next_start_t;
};


class TaskWrapper {
  private:
    uint16_t _min_update_period;
    uint32_t _last_update_time;
    
    continuous_task task_queue[TASK_LIST_MAX_LENGTH];
    uint8_t task_queue_length;

    continuous_task _task;
    uint8_t _task_index;
    uint32_t _task_start_t;

    periodical_task task_list[TASK_LIST_MAX_LENGTH];
    uint8_t task_list_length;

  public:
    void reset() {
      task_queue_length = 0;
      _task.execute = NULL;
      _task_start_t = 0;
      task_list_length = 0;
      _last_update_time = millis()-_min_update_period;  //0-delay after reset
      update();
    }

    /*
     * min_update_period - минимальный период (мс) между реальными обновлениями.
     * Вызовы метода update() чаще чем через min_update_period будут игнорироваться
     */
    TaskWrapper(uint16_t min_update_period = 0) {
      _min_update_period = min_update_period;
      reset();
    }

    void update() {
      uint32_t t = millis();

      if (t - _last_update_time >= _min_update_period) {
          _last_update_time = t;
    
          if ((!_task.execute)                                            //не назначен метод выполнения (reset)
          || (_task.duration && t >= _task_start_t + _task.duration)      //не бесконечная задача, время выполнения завершено
          || (_task.available && !_task.available())) {                   //метод проверки назначен и возвращает false(невозможно дальше выполнять)
            //смена задачи
            uint8_t _index = _task_index;
            while (1) {
              if (_task_index++ >= task_queue_length) {
                _task_index = 0;
              }
    
              if (_index == _task_index) { //цикл замкнулся, нет задач к выполнению
                _task.available = NULL;
                _task.execute = NULL;
                break;
              } else {
                if (task_queue[_task_index].available && task_queue[_task_index].available()) {
                  _task.duration = task_queue[_task_index].duration;
                  _task.available = task_queue[_task_index].available;
                  _task.execute = task_queue[_task_index].execute;
                  _task_start_t = t;
                  break;
                }
              }
            }
          }
    
          if (_task.execute) {
            if (!_task.available || _task.available()) {
              _task.execute();
            }
          }
    
          //periodical tasks
          for (int i = 0; i < task_list_length; i++) {
            if (t >= task_list[i].next_start_t) {
              task_list[i].next_start_t = t + task_list[i].period;
              task_list[i].execute();
            }
          }
      }
    }

    int8_t callContinuousTask(uint32_t duration, uint8_t (*available)(), void (*execute)()) {
      if (execute) {
        _task.duration = duration;
        _task.available = available;
        _task.execute = execute;
        _task_start_t = millis();
        return 1;
      }
      return 0;
    }

     /**
     * вызов задачи, завершение выполнения которой будет выполнено через duration 
     */
    int8_t callContinuousTask(uint32_t duration, void (*execute)()) {
      return callContinuousTask(duration, NULL, execute);
    }


    /**
     * вызов задачи, завершение выполнения которой определяется функцией available
     */
    int8_t callContinuousTask(uint8_t (*available)(), void (*execute)()) {
      return callContinuousTask(0, available, execute);
    }

    /**
     * вызов бесконечной задача, переход в другую задачу
     * возможен только вызовом callContinuousTask еще раз
     */
    int8_t callContinuousTask(void (*execute)()) {
      return callContinuousTask(0, NULL, execute);
    }

    int8_t addContinuousTask(uint32_t duration, uint8_t (*available)(), void (*execute)()) {
      if (task_queue_length < TASK_LIST_MAX_LENGTH) {
        if (duration && execute) {
          task_queue[task_queue_length].duration = duration;
          task_queue[task_queue_length].available = available;
          task_queue[task_queue_length].execute = execute;
          return task_queue_length++;
        }
      }
      return -1;
    }

    int8_t addPeriodicalTask(uint32_t delay, uint32_t period, void (*execute)()) {
      if (task_list_length < TASK_LIST_MAX_LENGTH) {
        if (period && execute) {
          task_list[task_list_length].period = period;
          task_list[task_list_length].execute = execute;
          task_list[task_list_length].next_start_t = delay;
          return task_list_length++;
        }
      }
      return -1;
    }
};

#endif
