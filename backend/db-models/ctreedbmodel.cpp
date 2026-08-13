#include "ctreedbmodel.h"

extern uint32_t id_admin;

CTreeDBModel::CTreeDBModel(CAbstractConnection *connection)
    : ADatabaseModel(treeTableName, connection)
    , m_connection(connection)
{}

CTreeDBModel::~CTreeDBModel()
= default;

std::vector<SNode> CTreeDBModel::get_nodes(uint32_t user_id)
{
    if (!prepare_connection())
        return {};

    this->clear_all();

    SDBCondition cond1, cond2, cond3;

    if (user_id == 0 || user_id == id_admin)
    {
        // admin or 0 - load public tree
        cond1.name = "id_user_i";
        cond1.value_s = "0";
        this->request->conditions->push_back(cond1);

        cond2.name = "id_user_i";
        cond2.value_s = "1";
        cond2.logic = ELogicOpers::ELO_OR;
        this->request->conditions->push_back(cond2);

        cond3.name = "is_removed_b";
        cond3.value_s = "0";
        cond3.logic = ELogicOpers::ELO_AND;
        this->request->conditions->push_back(cond3);
    }

    else {
        cond1.name = "id_user_i";
        cond1.value_s = std::to_string(user_id);

        cond3.name = "is_removed_b";
        cond3.value_s = "0";
        cond3.logic = ELogicOpers::ELO_AND;

        this->request->conditions->push_back(cond1);
        this->request->conditions->push_back(cond3);
    }

    bool res = read(true);

    if (res)
    {
        std::vector<SNode> nodes;
        for (auto &row : *this->answer)
        {
            if (row->empty())
            {
                continue;
            }

            SNode node = rowToNode(row);

            nodes.push_back(node);
        }
        return nodes;
    }
    return {};
}

bool CTreeDBModel::addNode(SNode &node)
{
    if (!prepare_connection())
    {
        return {};
    }

    auto array = nodeToRow(node);
    request->values->insert(request->values->end(), array.begin(), array.end());
    bool insRes = add(false);
    if (insRes)
    {
        node.id = get_last_id();
    }
    return insRes;
}

std::vector<SNode> CTreeDBModel::get_childrenNodes(uint32_t id_parent)
{
    if (!prepare_connection())
        return {};


    SDBCondition cond, cond2;
    cond.name = "id_parent";
    cond.value_s = std::to_string(id_parent);
    this->request->conditions->push_back(cond);

    cond2.name = "is_removed_b";
    cond2.value_s = "0";
    cond2.logic = ELogicOpers::ELO_AND;

    std::vector<SNode> nodes;

    bool res = read(true);

    if (res)
    {
        for (auto &row : *this->answer)
        {
            if (row->empty())
            {
                continue;
            }

            SNode node = rowToNode(row);

            nodes.push_back(node);
        }
    }
    return nodes;
}

bool CTreeDBModel::updateNode(const SNode &node)
{
    if (!prepare_connection())
        return false;

    SDBCondition condition;
    condition.name = "id";
    condition.value_s = std::to_string(node.id);
    this->request->conditions->push_back(condition);

    std::vector<CDBValue*> row = nodeToRow(node);

    for (auto &val : row)
    {
        this->request->values->push_back(val);
    }

    bool res = this->write();
    return res;
}

bool CTreeDBModel::removeNode(uint32_t idNode)
{
    if (!prepare_connection())
        return false;

    SDBCondition condition;
    condition.name = "id";
    condition.value_s = std::to_string(idNode);
    request->values->emplace_back(new CDBValue("is_removed_b", 1));
    bool res = this->write();

    return res;
}

bool CTreeDBModel::prepare_connection()
{
    if (!m_connection->is_opened())
    {
        if (!m_connection->open())
            return false;
    }
    this->clear_all();
    return true;
}

SNode CTreeDBModel::rowToNode(std::vector<CDBValue *> *row)
{
    SNode node;
    node.id = UINT32_MAX;
    if (!row || row->empty())
    {
        return node;
    }

    node.id = row->at(0)->integer();
    node.parent_id = row->at(1)->integer();
    node.user_id = row->at(2)->integer();
    node.res_type = (ResourceType)row->at(3)->integer();
    node.title = row->at(4)->string();
    node.is_public = row->at(5)->integer();
    node.is_editable = row->at(6)->integer();
    node.is_container = row->at(7)->integer();
    node.is_admin = row->at(8)->integer();
    node.is_removed = row->at(9)->integer();

    return node;
}

std::vector<CDBValue *> CTreeDBModel::nodeToRow(const SNode &node)
{
    std::vector<CDBValue *> values;

    values.emplace_back(new CDBValue("id_parent", node.parent_id));
    values.emplace_back(new CDBValue("id_user_i", node.user_id));
    values.emplace_back(new CDBValue("res_type_i", node.res_type));
    values.emplace_back(new CDBValue("title_s", node.title));
    values.emplace_back(new CDBValue("is_public_b", node.is_public));
    values.emplace_back(new CDBValue("is_editable_b", node.is_editable));
    values.emplace_back(new CDBValue("is_container_b", node.is_container));
    values.emplace_back(new CDBValue("is_admin_b", node.is_admin));
    values.emplace_back(new CDBValue("is_removed_b", node.is_removed));

    return values;
}

bool CTreeDBModel::cleanRemoved()
{
    if (!prepare_connection())
    {
        return false;
    }

    SDBCondition condition;
    condition.name = "is_removed_b";
    condition.value_s = "1";
    request->conditions->push_back(condition);

    bool res = remove();
    return res;
}
