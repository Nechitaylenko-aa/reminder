//
// Created by artem on 12.05.26.
//

#ifndef REMINDER_CTRACKEDTEXTS_H
#define REMINDER_CTRACKEDTEXTS_H

#include <ADatabaseModel.h>
#include "../sources/include/base-types.h"

class CTrackedTexts : public ADatabaseModel
{
public:
    explicit CTrackedTexts(CAbstractConnection *connection);
    ~CTrackedTexts() override;

    std::vector<uint32_t>   trackedTextNodes();

private:

private:



};


#endif //REMINDER_CTRACKEDTEXTS_H
