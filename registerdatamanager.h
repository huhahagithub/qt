#ifndef REGISTERDATAMANAGER_H
#define REGISTERDATAMANAGER_H
#include <QString>

class registerDataManager
{
public:
    registerDataManager();
    QString type;
    QString value;
    QString visible;
    QString updateRate;
    QString userName;
    QString id;
    QString unit;
};

#endif // REGISTERDATAMANAGER_H
