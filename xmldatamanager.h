#ifndef XMLDATAMANAGER_H
#define XMLDATAMANAGER_H
#include "buttondatamanager.h"
#include "nodedatamanager.h"
#include <QDomElement>

class xmlDataManager
{
public:
    xmlDataManager(QString filePath);

    xmlDataManager();

    bool readFile(QString filePath);

    void clear();

    QStringList getGcodeFileList();

    QMap<QString, nodeDataManager> getNodeData();

    QMap<QString, buttonDataManager> getButtonData();


private:
    //[nodename:nodeDataManager]键值对
    QMap<QString, nodeDataManager> nodeDataMap;

    //[buttonname:buttonDataManager]键值对
    QMap<QString, buttonDataManager> buttonDataMap;

    QStringList gcode_fileList;

    bool parseProjectTag(const QDomElement &root);

    bool parseSystemConfigTag(const QDomElement &systemConfig);

    bool parseGcodeFilesTag(const QDomElement &gcodeFiles);

    bool parseGcodeFileTag(const QDomElement &gcodeFile);

    bool parseNodeTag(const QDomElement &node);

    bool parseButtonsTag(const QDomElement &buttons);

    bool parseButtonTag(const QDomElement &button);

    bool parseOptionsTag(const QDomElement &options, nodeDataManager &nodeManager);

    bool parseRegistersTag(const QDomElement &registers, nodeDataManager &nodeManager);

    bool parseOptionTag(const QDomElement &options, nodeDataManager &nodeManager);

    bool parseRegisterTag(const QDomElement &registers, nodeDataManager &nodeManager);

    void transform2FullGcodePath();

};

#endif // XMLDATAMANAGER_H
