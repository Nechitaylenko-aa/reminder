//
// Created by artem on 03.05.26.
//

#include <cassert>
#include "CTextDBModel.h"
#include "CUserModelDB.h"
#include "ctreedbmodel.h"

extern uint32_t id_admin;

CTextDBModel::CTextDBModel(CAbstractConnection *connection)
    : ADatabaseModel(resTable, connection)
    , m_connection(connection)
{
    if (id_admin == 0)
    {
        CUserModelDB users_db(connection);
        users_db.defineAdminId();
    }
}

CTextDBModel::~CTextDBModel()
= default;

std::vector<TextEntry> CTextDBModel::textDataByParent(uint32_t parenId)
{
    if (!prepareConnection())
    {
        return {};
    }
    std::vector<TextEntry> result;

    SDBCondition condition, remCond;
    condition.name = "id_parent_i";
    condition.value_s = std::to_string(parenId);
    request->conditions->push_back(condition);

    remCond.name = "is_removed_b";
    remCond.value_s = "0";
    remCond.logic = ELogicOpers::ELO_AND;
    request->conditions->push_back(remCond);


    read(true);

    if (!answer->empty())
    {
        for (auto &row : *this->answer)
        {
            TextEntry entry = rowToEntry(row);
            result.push_back(entry);
        }
    }

    return result;
}

bool CTextDBModel::updateEntry(const TextEntry &entry)
{
    if (!prepareConnection())
    {
        return false;
    }

    SDBCondition cond("id", std::to_string(entry.id));
    request->conditions->push_back(cond);

    auto arr = entryToRow(entry);
    request->values->insert(request->values->end(), arr.begin(), arr.end());

    bool res = write();
    return res;
}

bool CTextDBModel::removeEntry(uint32_t id_entry)
{
    if (!prepareConnection())
    {
        return false;
    }

    SDBCondition cond("id", std::to_string(id_entry));
    request->conditions->push_back(cond);
    auto val = new CDBValue("is_removed_b", 1);
    request->values->push_back(val);

    bool res = write();
    return res;
}

bool CTextDBModel::moveEntries(std::vector<TextEntry> &entries, uint32_t newParentId)
{
    if (!prepareConnection())
    {
        return false;
    }

    for (auto &entry : entries)
    {
        entry.id_parent = newParentId;
        updateEntry(entry);
    }
    return true;
}

bool CTextDBModel::finalDelete()
{
    if (!prepareConnection())
    {
        return false;
    }

    SDBCondition cond("is_removed_b", "1");
    request->conditions->push_back(cond);

    bool res = remove();
    return res;
}

bool CTextDBModel::prepareConnection()
{
    if (!m_connection->is_opened())
    {
        if (!m_connection->open())
        {
            return false;
        }
    }
    clear_all();
    return true;
}

TextEntry CTextDBModel::rowToEntry(std::vector<CDBValue *> *row)
{
    //assert(row->size() == fieldsCount);
    TextEntry entry;

    entry.id = row->at(0)->integer();
    entry.id_parent = row->at(1)->integer();
    entry.desc = row->at(2)->string();
    entry.data = row->at(3)->string();
    entry.type = (TextEntryType)row->at(4)->integer();
    entry.is_removed = row->at(5)->integer();

    return entry;
}

std::vector<CDBValue *> CTextDBModel::entryToRow(const TextEntry &entry)
{
    std::vector<CDBValue*> result;

    result.emplace_back(new CDBValue("id_parent_i", entry.id_parent));
    result.emplace_back(new CDBValue("descript_s", entry.desc));
    result.emplace_back(new CDBValue("value_s", entry.data));
    result.emplace_back(new CDBValue("type_i", static_cast<int>(entry.type)));
    result.emplace_back(new CDBValue("is_removed_b", entry.is_removed));

    return result;
}

bool CTextDBModel::addEntry(TextEntry &entry)
{
    if (!prepareConnection())
    {
        return false;
    }

    auto row = entryToRow(entry);
    request->values->insert(request->values->end(), row.begin(), row.end());

    bool res = this->add(false);

    uint32_t id = get_last_id();
    entry.id = id;

    return res;
}

bool CTextDBModel::changeParent(uint32_t idParent, const std::vector<uint32_t> &items)
{
    if (!prepareConnection())
    {
        return false;
    }

    bool res = true;

    for (auto &idEntry : items)
    {
        clear_all();
        SDBCondition cond ("id", std::to_string(idEntry));
        request->conditions->push_back(cond);
        request->values->emplace_back(new CDBValue("id_parent_i", idParent));
        if (!write())
            res = false;
    }
    return res;
}

std::vector<TextEntry> CTextDBModel::prefetchData(uint32_t id_user)
{
    CTreeDBModel treeModel(m_connection);

    std::vector<TextEntry> textData;

    auto nodes = treeModel.get_nodes(id_user);
    if (nodes.empty())
    {
        return {};
    }

    for (const SNode &node : nodes)
    {
        if (node.res_type != ResourceType::TEXT)
        {
            continue;
        }

        auto items = textDataByParent(node.id);
        if (!items.empty())
        {
            for (auto &event : items)
            {
                textData.push_back(event);
            }
        }
    }
    return textData;
}
