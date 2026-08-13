//
// Created by artem on 06.05.26.
//
#include <vector>
#include "CEventsDBModel.h"
#include "ctreedbmodel.h"
#include "CUserModelDB.h"

extern uint32_t id_admin;

CEventsDBModel::CEventsDBModel(CAbstractConnection *connection)
    : ADatabaseModel(eventsTableName, connection)
{
    if (id_admin == 0)
    {
        CUserModelDB users_db(connection);
        users_db.defineAdminId();
    }
}

CEventsDBModel::~CEventsDBModel()
= default;

bool CEventsDBModel::addEvent(EventEntry &entry)
{
    if (entry.description.empty())
    {
        return false;
    }
    if (!prepareConnection())
    {
        return false;
    }

    std::vector<CDBValue*> array = entryToRow(entry);
    request->values->insert(request->values->end(), array.begin(), array.end());
    bool res = add(false);
    if (res)
    {
        entry.id = get_last_id();
    }

    return res;
}

bool CEventsDBModel::updateEvent(EventEntry &entry)
{
    if (!prepareConnection())
    {
        return false;
    }

    request->conditions->push_back(new SDBCondition("id", std::to_string(entry.id)));
    auto array = entryToRow(entry);
    request->values->insert(request->values->end(), array.begin(), array.end());

    bool res = write();
    return res;
}

bool CEventsDBModel::removeEvents(const std::vector<EventEntry> &items)
{
    if (!prepareConnection())
    {
        return false;
    }

    int index = 0;
    for (auto & item : items)
    {
        SDBCondition condition;
        condition.name = "id";
        condition.value_s = std::to_string(item.id);

        if (index > 0)
        {
            condition.logic = ELogicOpers::ELO_OR;
        }
        request->conditions->push_back(condition);
        request->values->push_back(new CDBValue("is_removed_b", 1));

        index++;
    }

    bool res = write();
    return res;
}

bool CEventsDBModel::changeEventsParent(uint32_t idParentNew, const std::vector<uint32_t> & items)
{
    if (!prepareConnection())
    {
        return false;
    }

    int index = 0;
    for (auto & id : items)
    {
        SDBCondition cond("id", std::to_string(id));

        if (index > 0)
        {
            cond.logic = ELogicOpers::ELO_OR;
        }
        request->conditions->push_back(cond);

        index++;
    }
    request->values->emplace_back(new CDBValue("id_parent_i", idParentNew));

    bool res = write();
    return res;
}

std::vector<EventEntry> CEventsDBModel::getEventsByNode(uint32_t id_node)
{
    if (!prepareConnection())
    {
        return {};
    }

    request->conditions->push_back(SDBCondition("id_parent_i", std::to_string(id_node)));
    request->conditions->push_back(SDBCondition("is_removed_b",EConditionOpers::ECO_EQ, "0", ELogicOpers::ELO_AND));

    bool res = read(true);
    if (!res)
    {
        return {};
    }

    std::vector<EventEntry> result;

    for (auto *& row : *answer)
    {
        EventEntry event = rowToEntry(row);
        result.push_back(event);
    }
    return  result;
}

std::vector<EventEntry> CEventsDBModel::prefetchUsersEvents(uint32_t id_user)
{

    CTreeDBModel treeModel(m_connection);

    std::vector<EventEntry> events;

    auto nodes = treeModel.get_nodes(id_admin);
    if (id_admin != id_user && id_user > 0)
    {
        auto items = treeModel.get_nodes(id_user);
        if (!items.empty())
        {
            nodes.insert(nodes.end(), items.begin(), items.end());
        }
    }


    if (nodes.empty())
    {
        return {};
    }

    for (const SNode &node : nodes)
    {
        if (node.res_type != ResourceType::DATES)
        {
            continue;
        }

        auto items = getEventsByNode(node.id);
        if (!items.empty())
        {
            for (auto &event : items)
            {
                events.push_back(event);
            }
        }
    }
    return events;
}

bool CEventsDBModel::prepareConnection()
{
    if (!m_connection->is_opened())
    {
        if (!m_connection->open())
            return false;
    }
    clear_all();
    return true;
}

std::vector<CDBValue*> CEventsDBModel::entryToRow(const EventEntry &entry)
{
    std::vector<CDBValue*> array;

    array.emplace_back(new CDBValue("id_parent_i", entry.id_parent));
    array.emplace_back(new CDBValue("period_i", entry.period));
    array.emplace_back(new CDBValue("period_count_i", entry.period_count));
    array.emplace_back(new CDBValue("event_ul", entry.event));
    array.emplace_back(new CDBValue("was_shown_ul", entry.was_shown));
    array.emplace_back(new CDBValue("event_type_i", entry.type));
    array.emplace_back(new CDBValue("event_trigger_i", entry.trigger));
    array.emplace_back(new CDBValue("description_s", entry.description));
    array.emplace_back(new CDBValue("priority_i", entry.priority));
    array.emplace_back(new CDBValue("is_enabled_b", entry.isEnabled));
    array.emplace_back(new CDBValue("is_removed_b", entry.is_removed));

    return array;
}

EventEntry CEventsDBModel::rowToEntry(const std::vector<CDBValue *> *row)
{
    if (!row || row->empty())
    {
        return {};
    }

    EventEntry event;

    event.id = row->at(0)->integer();
    event.id_parent = row->at(1)->integer();
    event.period = (EventPeriod)row->at(2)->integer();
    event.period_count = row->at(3)->integer();
    event.event = row->at(4)->integer();
    event.was_shown = row->at(5)->integer();
    event.type = (EventType)row->at(6)->integer();
    event.trigger = (EventTrigger)row->at(7)->integer();
    event.description = row->at(8)->string();
    event.priority = (EventPriority)row->at(9)->integer();
    event.isEnabled = row->at(10)->integer();
    event.is_removed = row->at(11)->integer();

    return event;
}

void CEventsDBModel::finalDelete()
{
    if (!prepareConnection())
    {
        return;
    }

    SDBCondition cond("is_removed_b", "1");
    request->conditions->push_back(cond);

    bool res = remove();
    if (!res)
    {
        fprintf(stderr, "Can't remove marked items from events model");
    }
}
