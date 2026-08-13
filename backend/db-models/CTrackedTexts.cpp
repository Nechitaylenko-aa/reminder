//
// Created by artem on 12.05.26.
//

#include "CTrackedTexts.h"

CTrackedTexts::CTrackedTexts(CAbstractConnection *connection)
    : ADatabaseModel("tracked_text_nodes", connection)
{}

CTrackedTexts::~CTrackedTexts()
= default;

std::vector<uint32_t> CTrackedTexts::trackedTextNodes()
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
