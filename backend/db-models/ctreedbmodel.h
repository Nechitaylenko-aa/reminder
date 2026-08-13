#ifndef CTREEDBMODEL_H
#define CTREEDBMODEL_H

#include <ADatabaseModel.h>
#include "../sources/include/base-types.h"

// table name in the database
inline const Tstring  treeTableName = "tree";

class CTreeDBModel : public ADatabaseModel
{
public:
    explicit CTreeDBModel(CAbstractConnection *connection);
    CTreeDBModel() = delete;
    CTreeDBModel(const CTreeDBModel &) = delete;
    ~CTreeDBModel() override;

    std::vector<SNode>   get_nodes(uint32_t user_id = 0); //!< 0 - is default public tree
    std::vector<SNode>   get_childrenNodes(uint32_t id_parent);
    bool    updateNode(const SNode &node);
    bool    addNode(SNode &node);
    bool    removeNode(uint32_t idNode);//!< Помечает на удаление;
    bool    cleanRemoved();    //!< Удаляет из базы помеченные на удаление

private:
    CAbstractConnection * m_connection;
private:
    bool prepare_connection();
    static SNode  rowToNode(std::vector<CDBValue*> * row);

    std::vector<CDBValue *> nodeToRow(const SNode &node);
};

#endif // CTREEDBMODEL_H
