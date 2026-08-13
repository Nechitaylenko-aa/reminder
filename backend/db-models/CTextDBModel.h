//
// Created by artem on 03.05.26.
//

#ifndef REMINDER_CTEXTDBMODEL_H
#define REMINDER_CTEXTDBMODEL_H

#include <ADatabaseModel.h>
#include "../sources/include/base-types.h"

constexpr Tsize fieldsCount = 6;
static inline Tstring resTable = "text_resources";

class CTextDBModel : public ADatabaseModel
{
public:
    explicit CTextDBModel(CAbstractConnection * connection);
    CTextDBModel(const CTextDBModel &) = delete;
    CTextDBModel() = delete;
    ~CTextDBModel() override;

    std::vector<TextEntry>  textDataByParent(uint32_t parenId);
    bool    updateEntry(const TextEntry &entry);
    bool    removeEntry(uint32_t id_entry);
    bool    addEntry(TextEntry &entry);
    bool    moveEntries(std::vector<TextEntry> &entries, uint32_t newParentId);
    bool    changeParent(uint32_t idParent, const std::vector<uint32_t> & items);

    bool     finalDelete();

    std::vector<TextEntry> prefetchData(uint32_t idUser);

protected:


private: // members
    CAbstractConnection * m_connection;

private: // methods
    bool prepareConnection();
    static TextEntry rowToEntry(std::vector<CDBValue*> *row);
    static std::vector<CDBValue*> entryToRow(const TextEntry &entry);
};


#endif //REMINDER_CTEXTDBMODEL_H
