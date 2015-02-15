#ifndef BOARDFACTORYPLUGININTERFACE_H
#define BOARDFACTORYPLUGININTERFACE_H

#include "board/abstractboard.h"

#include <QList>
#include <QtPlugin>

/*============================================================================
================================ BoardFactoryPluginInterface =================
============================================================================*/

class OLOLORD_EXPORT BoardFactoryPluginInterface
{
public:
    virtual ~BoardFactoryPluginInterface() {}
public:
    virtual QList<AbstractBoard *> createBoards() = 0;
};

Q_DECLARE_INTERFACE(BoardFactoryPluginInterface, "ololord.BoardFactoryPluginInterface")

#endif // BOARDFACTORYPLUGININTERFACE_H
