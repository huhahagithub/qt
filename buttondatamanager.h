#ifndef BUTTONDATAMANAGER_H
#define BUTTONDATAMANAGER_H
#include <QString>

class buttonDataManager
{
public:
    buttonDataManager(){}

    QString name;
    QString text;

    QString tooltip;
    QString gcode_file;
};

#endif // BUTTONDATAMANAGER_H
