#ifndef HISTORY_OBSERVATIONS_h
#define HISTORY_OBSERVATIONS_h

#include <stdint.h>
#include <initializer_list>

struct _current_history {
	uint32_t hour = 0;
	uint32_t sum = 0;
	uint16_t cnt = 0;
	
	void add(uint16_t value){
    if (value){
		  sum += value;
		  cnt++;
    }
	}
	
	uint16_t avg(){ 
    if (cnt){
		  return sum / cnt;
    }
    return 0;
	}
	
	uint16_t reset(){
		sum = 0;
		cnt = 0;
	}
};

#define min(a,b) ((a)<(b)?(a):(b))

#define HISTORY_NUM_DAYS 7
#define HISTORY_NUM_HOURS HISTORY_NUM_DAYS * 24

#define HOUR(time) (time/3600)
#define DAY_BY_HOUR(hour) (hour/24)
#define DAY(time) (DAY_BY_HOUR(HOUR(time)))

struct _history{
	uint32_t hour = 0;
	uint16_t values[HISTORY_NUM_HOURS];
};


class HistoryObservation{
private:
	_current_history current_history;
	_history history;
	
	void put_to_history(uint32_t hour, uint16_t value);
public:
	HistoryObservation();

  void addNextValue(uint32_t time, uint16_t value);
	
	uint16_t getHourValue(uint32_t time);
	
	uint16_t getDayValue(uint32_t time);
	
};

#endif
