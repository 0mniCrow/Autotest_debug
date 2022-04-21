#include "ConfigEditor_v2.h"

SettingNode::SettingNode(QString valname,
                         const QVariant &data,
                         SettingNode *ansestor):
    nameVal(valname),valueDat(data),parentItem(ansestor)
{}

SettingNode::~SettingNode()
{
    qDeleteAll(childItems);
}

SettingNode * SettingNode::parent()
{
    return parentItem;
}

SettingNode* SettingNode::child(int number)
{
    if(number<0||number>=childItems.size())
        return nullptr;
    return childItems.at(number);
}

int SettingNode::childCount() const
{
    return childItems.count();
}

int SettingNode::childNumber()const
{
    if(parentItem)
        return parentItem->childItems.indexOf(const_cast<SettingNode*>(this));
    return 0;
}

int SettingNode::columnCount() const
{
    return SN_DATASIZE;
}

QVariant SettingNode::data(int column) const
{
    switch(column)
    {
    case SN_NAMECOLUMN:
    {
        return nameVal;
    }
        break;
    case SN_VALUECOLUMN:
    {
        return valueDat;
    }
        break;
    default:
    {
        return QVariant();
    }
        break;
    }
}

bool SettingNode::setData(int column, const QVariant &val)
{
    switch(column)
    {
    case SN_NAMECOLUMN:
    {
        nameVal = val.toString();
        return true;
    }
        break;
    case SN_VALUECOLUMN:
    {
        if(childCount()==0)
        {
            valueDat = val;
            return true;
        }
        else
            return false;
    }
        break;
    default:
    {
        return false;
    }
        break;
    }
}

bool SettingNode::insertChildren(int position, int count)
{
    if(position<0||position>childItems.size())
    {
        return false;
    }
    for(int row=0;row<count;++row)
    {
        SettingNode * item = new SettingNode("NewValue",QVariant(),this);
        valueDat = QVariant();
        childItems.insert(position,item);
    }
    return true;
}

bool SettingNode::removeChildren(int position, int count)
{
    if(position<0||position+count>childItems.size())
        return false;
    for(int row = 0; row<count;++row)
    {
        delete childItems.takeAt(position);
    }
    return true;
}

QString SettingNode::getparentname() const
{
    if(parentItem)
    {
        return parentItem->getparentname()+QString("/")+nameVal;
    }
    else
        return QString();
}

void SettingNode::appendChild(SettingNode * child)
{
    childItems.append(child);
}
/*
SettingNode::SettingNode_Type SettingNode::typeinfo() const
{
    return typeName;
}
*/
bool SettingNode::convertVariant(QVariant*data,Conversion_Type typeTo)
{
    bool ok;
    switch(typeTo)
    {
    case CN_toInt:
    {
        *data=data->toInt(&ok);
    }
        break;
    case CN_toDouble:
    {
        *data=data->toDouble(&ok);
    }
        break;
    case CN_toString:
    {
        *data=data->toString();
        ok=true;
    }
        break;
    case CN_toBool:
    {
        *data=data->toBool();
        ok=true;
    }
        break;
    default:
    {
        ok=false;
    }
        break;
    }
    return ok;
}

SettingNode::Conversion_Type SettingNode::stringType(const QString data)
{
    QRegExp bolval("^(true|false)$");
    QRegExp intval("^(?:(?:\\d+)|(?:0[xX][0-9A-Fa-f]+))$");
    QRegExp dobval("^\\d+\\.\\d+$");
    if(bolval.indexIn(data)>=0)
        return CN_toBool;
    else if(dobval.indexIn(data)>=0)
        return CN_toDouble;
    else if(intval.indexIn(data)>=0)
        return CN_toInt;
    else
        return CN_toString;
}

SettingModel::SettingModel(const QStringList &headers,
                           QSettings *data,
                           QObject *parent):
    QAbstractItemModel(parent)
{
    if((!headers.isEmpty())&&headers.size()>=2)
        rootItem = new SettingNode(headers.at(SN_NAMECOLUMN),headers.at(SN_VALUECOLUMN));
    else
        rootItem = new SettingNode(QString("NAME"),QString("VALUE"));
    dataItem=data;
    setupModelData(rootItem);
}

SettingModel::~SettingModel()
{
    delete rootItem;
}

QVariant SettingModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    if(role!=Qt::DisplayRole&&role!=Qt::EditRole)
        return QVariant();
    SettingNode * item = getItem(index);
    return item->data(index.column());
}

QVariant SettingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation==Qt::Horizontal&&role==Qt::DisplayRole)
        return rootItem->data(section);
    return QVariant();
}

SettingNode * SettingModel::getItem(const QModelIndex &index) const
{
    if(index.isValid())
    {
        SettingNode * item = static_cast<SettingNode*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

int SettingModel::rowCount(const QModelIndex &parent) const
{
    const SettingNode * parentItem = getItem(parent);
    return parentItem?parentItem->childCount():0;
}

int SettingModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return SN_DATASIZE;
}

Qt::ItemFlags SettingModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags flgs = QAbstractItemModel::flags(index);
    if(index.column()==SN_VALUECOLUMN)
    {
        SettingNode * item = static_cast<SettingNode*>(index.internalPointer());

        if(item->childCount()==0)
            return flgs|Qt::ItemIsEditable;
        else
            return flgs;
    }
    else if(index.column()==SN_NAMECOLUMN)
    {
        return flgs|Qt::ItemIsEditable;
    }
    return flgs;
}

bool SettingModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    SettingNode * parentItem=getItem(parent);
    if(!parentItem)
        return false;
    beginInsertRows(parent,position,position+rows-1);
    if(parentItem->childCount()==0)
    {
        parentItem->setData(SN_VALUECOLUMN,QVariant());
        QString name = parentItem->getparentname();
        dataItem->remove(name);
        dataItem->beginGroup(name);
        dataItem->endGroup();
    }
    const bool success = parentItem->insertChildren(position,rows);
    if(success)
    {
        SettingNode * childElement = parentItem->child(position);
        QString name = childElement->getparentname();
        dataItem->setValue(name,childElement->data(SN_VALUECOLUMN));
    }
    endInsertRows();
    emit dataChanged(parent,parent);
    return success;
}

bool SettingModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    SettingNode * parentItem = getItem(parent);
    if(!parentItem)
        return false;
    beginRemoveRows(parent,position,position+rows-1);
    QString path = parentItem->child(position)->getparentname();
    dataItem->remove(path);
    const bool success = parentItem->removeChildren(position,rows);
    endRemoveRows();
    return success;
}

bool SettingModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(role!=Qt::EditRole)
        return false;
    SettingNode * item = getItem(index);
    QVariant tempdata = value;
    bool result;

    if(item->childCount()==0)
    {
        if(index.column()==SN_NAMECOLUMN)
        {

                QString snpath = item->getparentname();
                QVariant snvalue = item->data(index.column());
                dataItem->remove(snpath);
                snpath=snpath.left(snpath.lastIndexOf("/")+1).append(value.toString());
                dataItem->setValue(snpath,snvalue);

            result = item->setData(index.column(),value);
        }
        else if(index.column()==SN_VALUECOLUMN)
        {
            if(tempdata.type()== QVariant::String)
            {
                 SettingNode::convertVariant(&tempdata,SettingNode::stringType(tempdata.toString()));
            }

                dataItem->setValue(item->getparentname(),tempdata);

            result = item->setData(index.column(),tempdata);
        }
        else
            return false;
    }
    else
    {
        if(index.column()==SN_NAMECOLUMN)
        {

            QString snpath = item->getparentname();
            QString newname = snpath.left(snpath.lastIndexOf('/')+1).append(value.toString());
            if(snpath==newname)
                return false;
            dataItem->beginGroup(snpath);
            QStringList groups = dataItem->allKeys();
            dataItem->endGroup();
            for(int i = 0; i < groups.size();i++)
            {
                dataItem->beginGroup(snpath);
                QVariant tempdat = dataItem->value(groups.at(i));
                dataItem->endGroup();
                dataItem->beginGroup(newname);
                dataItem->setValue(groups.at(i),tempdat);
                dataItem->endGroup();
            }
            dataItem->beginGroup(snpath);
            dataItem->remove("");
            dataItem->endGroup();

            result = item->setData(index.column(),value);
        }
        else
            return false;
    }
    return result;
}

QModelIndex SettingModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid()&&parent.column()!=0)
        return QModelIndex();
    SettingNode * parentItem=getItem(parent);
    if(!parentItem)
        return QModelIndex();
    SettingNode * childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row,column,childItem);
    return QModelIndex();
}

QModelIndex SettingModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();
    SettingNode * childItem = getItem(index);
    SettingNode * parentItem = childItem?childItem->parent():nullptr;
    if(parentItem==rootItem||!parentItem)
        return QModelIndex();
    return createIndex(parentItem->childNumber(),0,parentItem);
}

void SettingModel::setupModelData(SettingNode *parent)
{
    QStringList groups = dataItem->childGroups();
    if(!groups.isEmpty())
    {
        for(int i = 0;i<groups.size();i++)
        {
            SettingNode * child = new SettingNode(groups.at(i),QVariant(),parent);
            parent->appendChild(child);
            dataItem->beginGroup(groups.at(i));
            setupModelData(child);
            dataItem->endGroup();
        }
    }
    groups.clear();
    groups = dataItem->childKeys();
    if(!groups.isEmpty())
    {
        QVariant tempdata;
        for(int i = 0;i<groups.size();i++)
        {
            tempdata= dataItem->value(groups.at(i));
            SettingNode::convertVariant(&tempdata,SettingNode::stringType(tempdata.toString()));
            SettingNode * child = new SettingNode(groups.at(i),
                                                  tempdata,parent);
            parent->appendChild(child);
        }
    }
    return;
}

QMap<QString,QVariant> SettingModel::getvariablemap(const QModelIndex & direct)
{
    SettingNode * parent = getItem(direct);
    if(!parent)
        return QMap<QString,QVariant>();
    else
        return collectvariables(parent);
}

QMap<QString,QVariant> SettingModel::collectvariables(SettingNode * parent)
{
    QMap<QString,QVariant> answer;
    if(parent->childCount()>0)
    {
        for(int i = 0; i<parent->childCount();i++)
        {
            SettingNode * child = parent->child(i);
            answer.unite(collectvariables(child));
        }
    }
    else
    {
        answer.insert(parent->data(SN_NAMECOLUMN).toString(),parent->data(SN_VALUECOLUMN));
    }
    return answer;
}
