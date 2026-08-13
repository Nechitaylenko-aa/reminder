//
// Created by artem on 28.04.26.
//

#ifndef REMINDER_IRESOURCE_H
#define REMINDER_IRESOURCE_H

#include "base-types.h"
#include <CAbstractConnection.h>

class  IResource
{
public:
    virtual ~IResource() = default;

    // --- GUI ------------------------------------------------------------------
    virtual void putWidget() = 0;
    virtual void hideWidget() = 0;
    virtual void clear() = 0;

    // --- Data Management ------------------------------------------------------
    // Массовая загрузка данных (например, при логине)
    virtual void prefetchData(uint32_t userId) = 0;
    virtual void showNode(const SNode &node) = 0;

    virtual void moveItemsToNode(uint32_t idNode, const std::vector<uint32_t> & items) = 0;
    virtual void checkPendingTasks()  = 0;

};

#endif //REMINDER_IRESOURCE_H
