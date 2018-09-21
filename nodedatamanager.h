#ifndef NODEDATAMANAGER_H
#define NODEDATAMANAGER_H
#include "optiondatamanager.h"
#include "registerdatamanager.h"
#include <QMap>

class nodeDataManager
{
public:
    nodeDataManager();

    QString type;
    QString name;

    QString description;

    //[id: optionDataManager]键值对
    QMap<QString, optionDataManager> optionDataMap;

     //[id: registerDataManager]键值对
    QMap<QString, registerDataManager> registerDataMap;
};

#endif // NODEDATAMANAGER_H
