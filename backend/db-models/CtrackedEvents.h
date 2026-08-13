//
// Created by artem on 13.05.26.
//

#ifndef REMINDER_CTRACKEDEVENTS_H
#define REMINDER_CTRACKEDEVENTS_H

#include <ADatabaseModel.h>
#include "../sources/include/base-types.h"

class CTrackedEvents : public ADatabaseModel
{
public:
    explicit CTrackedEvents(CAbstractConnection *connection);
    ~CTrackedEvents() override;

    std::vector<uint32_t>   trackedDatesNodes();

protected:


private:

private:


};


#endif //REMINDER_CTRACKEDEVENTS_H
