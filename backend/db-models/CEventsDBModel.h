//
// Created by artem on 06.05.26.
//

#ifndef REMINDER_CEVENTSDBMODEL_H
#define REMINDER_CEVENTSDBMODEL_H

#include <ADatabaseModel.h>
#include "../sources/include/base-types.h"

inline const Tstring  eventsTableName = "events";

class CEventsDBModel : public ADatabaseModel
{
public:
    explicit CEventsDBModel(CAbstractConnection *connection);
    ~CEventsDBModel() override;
    CEventsDBModel() = delete;
    CEventsDBModel(const CEventsDBModel &) = delete;

    bool   addEvent(EventEntry &entry);
    bool   updateEvent(EventEntry & entry);
    bool   removeEvents(const std::vector<EventEntry> & items);
    bool   changeEventsParent(uint32_t idParentNew, const std::vector<uint32_t> & items);
    std::vector<EventEntry>   getEventsByNode(uint32_t id_node);
    std::vector<EventEntry>   prefetchUsersEvents(uint32_t id_user);
    void   finalDelete();

private:
    bool  prepareConnection();

    static std::vector<CDBValue*> entryToRow(const EventEntry &entry);
    static EventEntry   rowToEntry(const std::vector<CDBValue*> *row);
};


#endif //REMINDER_CEVENTSDBMODEL_H
