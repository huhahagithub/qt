#include "xmldatamanager.h"
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QDebug>
#include <QString>
#include <QStringList>


/*******************************xml结构*****************************
<project>
    <system_config>
        <node type="" name="" description="">
            <options>
                <option type="" value="" id="" name="" unit=""/>
            </options>
            <registers>
                <register type="" value="" visible="" updata_rate="" user_name="" id="" name="" unit=""/>
            </registers>
        </node>
        <buttons>
            <button name="" text="" tooltip="" gcode_file=""/>
        </buttons>
    </system_config>
    <gcode_files>
        <gcode_file path=""/>
    </gcode_files>
</project>
*******************************************************************/


/*******************************公有函数*****************************/

/*******************************************************************
 *brief         :构造函数
 *
 *other         :无
 ******************************************************************/
xmlDataManager::xmlDataManager(QString filePath)
{
    readFile(filePath);
}

/*******************************************************************
 *brief         :构造函数
 *
 *other         :无
 ******************************************************************/
xmlDataManager::xmlDataManager()
{

}

/******************************************************************
 *brief         :从文件中解析xml
 *
 *return        :true--成功  false--失败
 ******************************************************************/
bool xmlDataManager::readFile(QString filePath)
{
    bool ret = true;
    //读文件
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug()<<"Can't open file:"<<filePath;
        return false;
    }

    //将文件流传递给QDomDocument解析
    QDomDocument document;
    QString error;
    int errorRow = 0, errorColumn = 0;

    if(!document.setContent(&file, false, &error, &errorRow, &errorColumn))
    {
        qDebug()<<"parse xmlfile failed at line row and column" + QString::number(errorRow, 10) + QString(",") + QString::number(errorColumn, 10);

        return false;
    }

    if(document.isNull())
    {
        qDebug()<<"document is null!";

        return false;
    }

    QDomElement root = document.documentElement();

    //root_tag_name为project
    QString root_tag_name = root.tagName();
    if(root_tag_name != "project")
    {
        qDebug()<< "Error: Not have <project> tag";
        return false;
    }

    ret = parseProjectTag(root);
    if(ret == false)
    {
        return false;
    }
    transform2FullGcodePath();
    return true;
}

/*******************************************************************
 *brief         :清空数据
 *
 *return        :无
 ******************************************************************/
void xmlDataManager::clear()
{
    this->buttonDataMap.clear();

    this->gcode_fileList.clear();

    this->nodeDataMap.clear();
}

/*******************************************************************
 *brief         :获取QStringList gcode_fileList
 *
 *return        :QStringList gcode_fileList;
 ******************************************************************/
QStringList xmlDataManager::getGcodeFileList()
{
    return this->gcode_fileList;
}

/*******************************************************************
 *brief         :获取QMap<QString, nodeDataManager> nodeDataMap
 *
 *return        :QMap<QString, nodeDataManager> nodeDataMap
 ******************************************************************/
QMap<QString, nodeDataManager> xmlDataManager::getNodeData()
{
    return this->nodeDataMap;
}

/*******************************************************************
 *brief         :获取QMap<QString, buttonDataManager> buttonDataMap
 *
 *return        :QMap<QString, buttonDataManager> buttonDataMap
 ******************************************************************/
QMap<QString, buttonDataManager> xmlDataManager::getButtonData()
{
    return this->buttonDataMap;
}

/*******************************私有函数*****************************/

/*******************************************************************
 *brief         :解析<project>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseProjectTag(const QDomElement &root)
{
    bool ret = true;
    QDomNodeList childList = root.childNodes();

    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <project> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "system_config")
        {
            ret = parseSystemConfigTag(childList.at(i).toElement());
            if(ret == false)
            {
                qDebug()<<"Error: parse <system_config> tag fail";
                return false;
            }
        }
        else if(childList.at(i).toElement().tagName() == "gcode_files")
        {
            parseGcodeFilesTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<project>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<system_config>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseSystemConfigTag(const QDomElement &systemConfig)
{
    QDomNodeList childList = systemConfig.childNodes();
    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <system_config> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "node")
        {
            parseNodeTag(childList.at(i).toElement());
        }
        else if(childList.at(i).toElement().tagName() == "buttons")
        {
            parseButtonsTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<system_config>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<gcode_files>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseGcodeFilesTag(const QDomElement &gcodeFiles)
{
    QDomNodeList childList = gcodeFiles.childNodes();
    if(childList.isEmpty())
    {
        //qDebug()<<"Error: parse <gcode_file> tag fail";
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "gcode_file")
        {
            parseGcodeFileTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<gcode_file>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<gcode_file>标签,获取path属性
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseGcodeFileTag(const QDomElement &gcodeFile)
{
    QString path = gcodeFile.attribute("path");

    if(!path.isEmpty())
    {
        this->gcode_fileList.append(path);
    }

    return true;
}


/*******************************************************************
 *brief         :解析<node>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseNodeTag(const QDomElement &node)
{
    nodeDataManager nodeManager;

    nodeManager.description = node.attribute("description");
    nodeManager.name = node.attribute("name");
    nodeManager.type = node.attribute("type");
    //FIXME:获取更多<node>标签属性

    QDomNodeList childList = node.childNodes();

    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <node> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "options")
        {
            parseOptionsTag(childList.at(i).toElement(), nodeManager);
        }
        else if(childList.at(i).toElement().tagName() == "registers")
        {
            parseRegistersTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<node>子标签
    }

    this->nodeDataMap.insert(node.attribute("name"), nodeManager);

    return true;
}

/*******************************************************************
 *brief         :解析<buttons>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseButtonsTag(const QDomElement &buttons)
{
    QDomNodeList childList = buttons.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "button")
        {
            parseButtonTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<buttons>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<options>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseOptionsTag(const QDomElement &options, nodeDataManager &nodeManager)
{
    QDomNodeList childList = options.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "option")
        {
            parseOptionTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<options>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<registers>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseRegistersTag(const QDomElement &registers, nodeDataManager &nodeManager)
{
    QDomNodeList childList = registers.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "register")
        {
            parseRegisterTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<registers>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<button>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseButtonTag(const QDomElement &button)
{
    this->buttonDataMap;
    buttonDataManager buttonData;

    buttonData.gcode_file = button.attribute("gcode_file");
    buttonData.name = button.attribute("name");

    buttonData.text = button.attribute("text");
    buttonData.tooltip = button.attribute("tooltip");
    //FIXME:添加更多<button>标签属性

    this->buttonDataMap.insert(button.attribute("name"), buttonData);

    return true;
}

/*******************************************************************
 *brief         :解析<register>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseRegisterTag(const QDomElement &regi, nodeDataManager &nodeManager)
{
    registerDataManager regiData;

    regiData.id = regi.attribute("id");
    regiData.type = regi.attribute("type");

    regiData.unit = regi.attribute("unit");
    regiData.updateRate = regi.attribute("updata_rate");

    regiData.userName = regi.attribute("user_name");
    regiData.value = regi.attribute("value");

    regiData.visible = regi.attribute("visible");
    //FIXME: 添加更多<register>属性

    nodeManager.registerDataMap.insert(regi.attribute("id"), regiData);

    return true;
}

/*******************************************************************
 *brief         :解析<option>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseOptionTag(const QDomElement &option, nodeDataManager &nodeManager)
{
    optionDataManager optionData;

    optionData.id = option.attribute("id");
    optionData.type = option.attribute("type");

    optionData.unit = option.attribute("unit");
    optionData.name = option.attribute("name");

    optionData.value = option.attribute("value");
    //FIXME: 添加更多<option>属性

    nodeManager.optionDataMap.insert(option.attribute("id"), optionData);

    return true;
}

/*******************************************************************
 *brief         :将buttonDataMap中的gcode_file转换为绝对路径
 *
 *return        :
 ******************************************************************/
void xmlDataManager::transform2FullGcodePath()
{
    int pos;

    QMap<QString, buttonDataManager>::iterator itr;

    for(itr = this->buttonDataMap.begin(); itr != this->buttonDataMap.end(); ++itr)
    {
        for(int i = 0; i < this->gcode_fileList.size(); ++i)
        {
            QString tempStr = this->gcode_fileList.at(i);
            pos = tempStr.indexOf(itr.value().gcode_file);
            if(pos != -1)
            {
                QString name = itr.key();
                buttonDataManager temp;
                temp.name = itr.value().name;
                temp.text = itr.value().text;
                temp.tooltip = itr.value().tooltip;
                temp.gcode_file = tempStr;
                this->buttonDataMap.insert(name, temp);
            }
        }
    }
}
#include "xmldatamanager.h"
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QDebug>
#include <QString>
#include <QStringList>


/*******************************xml结构*****************************
<project>
    <system_config>
        <node type="" name="" description="">
            <options>
                <option type="" value="" id="" name="" unit=""/>
            </options>
            <registers>
                <register type="" value="" visible="" updata_rate="" user_name="" id="" name="" unit=""/>
            </registers>
        </node>
        <buttons>
            <button name="" text="" tooltip="" gcode_file=""/>
        </buttons>
    </system_config>
    <gcode_files>
        <gcode_file path=""/>
    </gcode_files>
</project>
*******************************************************************/


/*******************************公有函数*****************************/

/*******************************************************************
 *brief         :构造函数
 *
 *other         :无
 ******************************************************************/
xmlDataManager::xmlDataManager(QString filePath)
{
    readFile(filePath);
}

/*******************************************************************
 *brief         :构造函数
 *
 *other         :无
 ******************************************************************/
xmlDataManager::xmlDataManager()
{

}

/******************************************************************
 *brief         :从文件中解析xml
 *
 *return        :true--成功  false--失败
 ******************************************************************/
bool xmlDataManager::readFile(QString filePath)
{
    bool ret = true;
    //读文件
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug()<<"Can't open file:"<<filePath;
        return false;
    }

    //将文件流传递给QDomDocument解析
    QDomDocument document;
    QString error;
    int errorRow = 0, errorColumn = 0;

    if(!document.setContent(&file, false, &error, &errorRow, &errorColumn))
    {
        qDebug()<<"parse xmlfile failed at line row and column" + QString::number(errorRow, 10) + QString(",") + QString::number(errorColumn, 10);

        return false;
    }

    if(document.isNull())
    {
        qDebug()<<"document is null!";

        return false;
    }

    QDomElement root = document.documentElement();

    //root_tag_name为project
    QString root_tag_name = root.tagName();
    if(root_tag_name != "project")
    {
        qDebug()<< "Error: Not have <project> tag";
        return false;
    }

    ret = parseProjectTag(root);
    if(ret == false)
    {
        return false;
    }
    transform2FullGcodePath();
    return true;
}

/*******************************************************************
 *brief         :清空数据
 *
 *return        :无
 ******************************************************************/
void xmlDataManager::clear()
{
    this->buttonDataMap.clear();

    this->gcode_fileList.clear();

    this->nodeDataMap.clear();
}

/*******************************************************************
 *brief         :获取QStringList gcode_fileList
 *
 *return        :QStringList gcode_fileList;
 ******************************************************************/
QStringList xmlDataManager::getGcodeFileList()
{
    return this->gcode_fileList;
}

/*******************************************************************
 *brief         :获取QMap<QString, nodeDataManager> nodeDataMap
 *
 *return        :QMap<QString, nodeDataManager> nodeDataMap
 ******************************************************************/
QMap<QString, nodeDataManager> xmlDataManager::getNodeData()
{
    return this->nodeDataMap;
}

/*******************************************************************
 *brief         :获取QMap<QString, buttonDataManager> buttonDataMap
 *
 *return        :QMap<QString, buttonDataManager> buttonDataMap
 ******************************************************************/
QMap<QString, buttonDataManager> xmlDataManager::getButtonData()
{
    return this->buttonDataMap;
}

/*******************************私有函数*****************************/

/*******************************************************************
 *brief         :解析<project>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseProjectTag(const QDomElement &root)
{
    bool ret = true;
    QDomNodeList childList = root.childNodes();

    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <project> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "system_config")
        {
            ret = parseSystemConfigTag(childList.at(i).toElement());
            if(ret == false)
            {
                qDebug()<<"Error: parse <system_config> tag fail";
                return false;
            }
        }
        else if(childList.at(i).toElement().tagName() == "gcode_files")
        {
            parseGcodeFilesTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<project>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<system_config>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseSystemConfigTag(const QDomElement &systemConfig)
{
    QDomNodeList childList = systemConfig.childNodes();
    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <system_config> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "node")
        {
            parseNodeTag(childList.at(i).toElement());
        }
        else if(childList.at(i).toElement().tagName() == "buttons")
        {
            parseButtonsTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<system_config>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<gcode_files>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseGcodeFilesTag(const QDomElement &gcodeFiles)
{
    QDomNodeList childList = gcodeFiles.childNodes();
    if(childList.isEmpty())
    {
        //qDebug()<<"Error: parse <gcode_file> tag fail";
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "gcode_file")
        {
            parseGcodeFileTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<gcode_file>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<gcode_file>标签,获取path属性
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseGcodeFileTag(const QDomElement &gcodeFile)
{
    QString path = gcodeFile.attribute("path");

    if(!path.isEmpty())
    {
        this->gcode_fileList.append(path);
    }

    return true;
}


/*******************************************************************
 *brief         :解析<node>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseNodeTag(const QDomElement &node)
{
    nodeDataManager nodeManager;

    nodeManager.description = node.attribute("description");
    nodeManager.name = node.attribute("name");
    nodeManager.type = node.attribute("type");
    //FIXME:获取更多<node>标签属性

    QDomNodeList childList = node.childNodes();

    if(childList.isEmpty())
    {
        qDebug()<<"Error: parse <node> tag fail";
        return false;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "options")
        {
            parseOptionsTag(childList.at(i).toElement(), nodeManager);
        }
        else if(childList.at(i).toElement().tagName() == "registers")
        {
            parseRegistersTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<node>子标签
    }

    this->nodeDataMap.insert(node.attribute("name"), nodeManager);

    return true;
}

/*******************************************************************
 *brief         :解析<buttons>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseButtonsTag(const QDomElement &buttons)
{
    QDomNodeList childList = buttons.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "button")
        {
            parseButtonTag(childList.at(i).toElement());
        }
        //FIXME: 解析更多<buttons>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<options>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseOptionsTag(const QDomElement &options, nodeDataManager &nodeManager)
{
    QDomNodeList childList = options.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "option")
        {
            parseOptionTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<options>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<registers>标签及以下子标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseRegistersTag(const QDomElement &registers, nodeDataManager &nodeManager)
{
    QDomNodeList childList = registers.childNodes();

    if(childList.isEmpty())
    {
        return true;
    }

    for(int i  = 0; i < childList.size(); ++i)
    {
        if(childList.at(i).toElement().tagName() == "register")
        {
            parseRegisterTag(childList.at(i).toElement(), nodeManager);
        }
        //FIXME: 解析更多<registers>子标签
    }

    return true;
}

/*******************************************************************
 *brief         :解析<button>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseButtonTag(const QDomElement &button)
{
    this->buttonDataMap;
    buttonDataManager buttonData;

    buttonData.gcode_file = button.attribute("gcode_file");
    buttonData.name = button.attribute("name");

    buttonData.text = button.attribute("text");
    buttonData.tooltip = button.attribute("tooltip");
    //FIXME:添加更多<button>标签属性

    this->buttonDataMap.insert(button.attribute("name"), buttonData);

    return true;
}

/*******************************************************************
 *brief         :解析<register>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseRegisterTag(const QDomElement &regi, nodeDataManager &nodeManager)
{
    registerDataManager regiData;

    regiData.id = regi.attribute("id");
    regiData.type = regi.attribute("type");

    regiData.unit = regi.attribute("unit");
    regiData.updateRate = regi.attribute("updata_rate");

    regiData.userName = regi.attribute("user_name");
    regiData.value = regi.attribute("value");

    regiData.visible = regi.attribute("visible");
    //FIXME: 添加更多<register>属性

    nodeManager.registerDataMap.insert(regi.attribute("id"), regiData);

    return true;
}

/*******************************************************************
 *brief         :解析<option>标签
 *element[in]   :待解析元素
 *return        :true--成功 false--失败
 ******************************************************************/
bool xmlDataManager::parseOptionTag(const QDomElement &option, nodeDataManager &nodeManager)
{
    optionDataManager optionData;

    optionData.id = option.attribute("id");
    optionData.type = option.attribute("type");

    optionData.unit = option.attribute("unit");
    optionData.name = option.attribute("name");

    optionData.value = option.attribute("value");
    //FIXME: 添加更多<option>属性

    nodeManager.optionDataMap.insert(option.attribute("id"), optionData);

    return true;
}

/*******************************************************************
 *brief         :将buttonDataMap中的gcode_file转换为绝对路径
 *
 *return        :
 ******************************************************************/
void xmlDataManager::transform2FullGcodePath()
{
    int pos;

    QMap<QString, buttonDataManager>::iterator itr;

    for(itr = this->buttonDataMap.begin(); itr != this->buttonDataMap.end(); ++itr)
    {
        for(int i = 0; i < this->gcode_fileList.size(); ++i)
        {
            QString tempStr = this->gcode_fileList.at(i);
            pos = tempStr.indexOf(itr.value().gcode_file);
            if(pos != -1)
            {
                QString name = itr.key();
                buttonDataManager temp;
                temp.name = itr.value().name;
                temp.text = itr.value().text;
                temp.tooltip = itr.value().tooltip;
                temp.gcode_file = tempStr;
                this->buttonDataMap.insert(name, temp);
            }
        }
    }
}
