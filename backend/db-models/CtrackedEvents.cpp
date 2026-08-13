//
// Created by artem on 13.05.26.
//

#include "CtrackedEvents.h"

CTrackedEvents::CTrackedEvents(CAbstractConnection *connection)
    : ADatabaseModel("tracked_events_nodes", connection)
{}

CTrackedEvents::~CTrackedEvents()
= default;

std::vector<uint32_t> CTrackedEvents::trackedDatesNodes()
{
    if (!m_connection->is_opened())
    {
        if (!m_connection->open())
        {
            return {};
        }
    }

    exec_stored_proc_no_res("refresh_tracked_nodes");

    clear_all();

    std::vector<uint32_t> res;

    bool r = read(true);

    if (r)
    {
        for (auto &row : *answer)
        {
            res.push_back(row->at(0)->integer());
        }
    }
    return res;
}
