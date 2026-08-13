//
// Created by artem on 10.05.26.
//

#ifndef REMINDER_DATETIMECALCULATOR_H
#define REMINDER_DATETIMECALCULATOR_H

#ifndef DATETIMECALCULATOR_H
#define DATETIMECALCULATOR_H

#include "../include/base-types.h"  // для EventEntry, EventTrigger



class DateTimeCalculator {
public:
    // Основные методы расчета
    static time_t calculateNextMinute(const EventEntry& event, time_t current);
    static time_t calculateNextHour(const EventEntry& event, time_t current);
    static time_t calculateNextDay(const EventEntry& event, time_t current);
    static time_t calculateNextWeek(const EventEntry& event, time_t current);
    static time_t calculateNextMonth(const EventEntry& event, time_t current);
    static time_t calculateNextYear(const EventEntry& event, time_t current);

    // Вспомогательные методы
    static time_t calculateStepBack(const EventEntry& event, time_t futureTime);
    static time_t applyTrigger(time_t timestamp, EventTrigger trigger);
    static bool isWeekend(time_t time);

    // Универсальный метод, выбирающий нужную стратегию
    static time_t calculateNext(const EventEntry& event, time_t current);
    static struct tm safeLocaltime(time_t time);
private:


};

#endif // DATETIMECALCULATOR_H


#endif //REMINDER_DATETIMECALCULATOR_H
