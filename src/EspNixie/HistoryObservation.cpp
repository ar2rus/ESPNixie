#include "HistoryObservation.h"

HistoryObservation::HistoryObservation(){
	//load history from disk
}

void HistoryObservation::put_to_history(uint32_t hour, uint16_t value){
	if (hour && hour >= history.hour){
		int32_t delta_hours = min(hour - history.hour, HISTORY_NUM_HOURS);
		if (delta_hours){
			for (int i=HISTORY_NUM_HOURS-1; i >= delta_hours ; i--){
				history.values[i] = history.values[i - delta_hours];
			}
			for (int i = 1; i < delta_hours; i++){
				history.values[i] = 0;
			}
		}
    history.hour = hour;
		history.values[0] = value;
		if (delta_hours){
			//save to disk
		}
	}
}

void HistoryObservation::addNextValue(uint32_t time, uint16_t value){
	if (time){
		uint32_t hour = HOUR(time);
		if (current_history.hour != hour){
			current_history.reset();
			current_history.hour = hour;
		}
		current_history.add(value);
		put_to_history(hour, current_history.avg());
	}
}

uint16_t HistoryObservation::getHourValue(uint32_t time){
	if (time){
		uint32_t hour = HOUR(time);
		if (hour <= history.hour && hour> history.hour-HISTORY_NUM_HOURS){
			return history.values[history.hour-hour];
		}
	}
	return 0;
}

uint16_t HistoryObservation::getDayValue(uint32_t time){
	if (time){
		uint16_t day = DAY(time);
		int32_t hour = HOUR(time);
		
		_current_history tmp;
		
		uint32_t _hour = hour;
		for (int sign: { -1, 1 }) {
			while (_hour <= history.hour && _hour> history.hour-HISTORY_NUM_HOURS){
				if (DAY_BY_HOUR(_hour) == day){
					tmp.add(history.values[history.hour-_hour]);
				}else{
					break;
				}
				_hour += sign;
			}
			_hour=hour+1;
		}
		
		return tmp.avg();
	}
 return 0;
}
