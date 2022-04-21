#include "DataModels_v2.h"

/*
LogModel::LogModel(QObject *obj):QAbstractTableModel(obj)
{

}

LogModel::~LogModel()
{
    cleartable();
}

//Данные по журналу
QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    if((index.row()>=0&&index.row()<work_log.size())&&(role==Qt::DisplayRole))
    {
        //В зависимости от номера столбца отображается номер теста
        //либо текст сообщения журнала.
        if(index.column()==_c_NUMBERPOSITION)
            return work_log.at(index.row()).first;
        else if(index.column()==_c_INFOPOSITION)
            return work_log.at(index.row()).second;
        else
            return QVariant();
    }
    else
        return QVariant();
}


bool LogModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //Нет необходимости в обновлении данных, лишь отображение.
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
    return false;
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return work_log.size();
}

int LogModel::columnCount(const QModelIndex &parent) const
{
    if(parent.isValid())
        return 0;
    return _c_DATAPOSITION;
}

QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role !=Qt::DisplayRole)
    {
        return QVariant();
    }
    else if(orientation==Qt::Horizontal)
    {
        if(section==_c_NUMBERPOSITION)
            return QString("Test №");
        else if(section==_c_INFOPOSITION)
            return QString("Message");
        else
            return QVariant();
    }
    else
        return QString::number(section+1);
}

Qt::ItemFlags LogModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::ItemIsEnabled;
    return QAbstractTableModel::flags(index);
}

//В слот поступает инофрмация о сообщении, и модель добавляет
//новое сообщение в новую созданную строку.
void LogModel::IncomesData(const QString info, const int number_of_test)
{
    beginInsertRows(QModelIndex(),work_log.size(),work_log.size());
    work_log.append(QPair<int,QString>(number_of_test,info));
    endInsertRows();
}

void LogModel::cleartable()
{
    int size = work_log.size()-1;
    beginRemoveRows(QModelIndex(),0,size);
    work_log.clear();
    endRemoveRows();
}
*/
//__________________________________________________________________________________

VarModel::VarModel(QMap<QString,QVariant> * inf, QObject * obj):QAbstractTableModel(obj)
{
    walker_test=false;
    work_map=inf;
    names.resize(work_map->size());
    int i=0;
    //Модель отображения переменных имеет два хранилища, внешнее work_map и своё names.
    //В work_map хранятся значения переменных по именам, а в names хранятся имена переменных
    //по номерам отображения. Нужно это для правильной идентификации записей в таблице.
    //Следующий цикл заполняет местный объект names в соответствии со значениями в контейнере
    //work_map.
    for (QMap<QString,QVariant>::const_iterator var = work_map->begin();var!=work_map->end();)
    {
        names[i++]=var.key();
    }
}

VarModel::~VarModel()
{
    work_map = nullptr;
    names.clear();
}

QVariant VarModel::data(const QModelIndex &index, int role) const
{
    if(walker_test)
    {
        if(!index.isValid())
            return QVariant();
        else if((index.row()<0)||(index.row()>=names.size()))
            return QVariant();
        else if(role==Qt::DisplayRole||role==Qt::EditRole)
        {
            if((index.column()==_c_NUMBERPOSITION)&&(role==Qt::DisplayRole))
                return names.at(index.row());
            else if(index.column()==_c_INFOPOSITION)
            {
                if(work_map->contains(names.at(index.row())))
                {
                    QVariant dat= work_map->find(names.at(index.row())).value();
                    QString typeN = dat.typeName();
                    if(typeN.contains("string",Qt::CaseInsensitive))
                        return dat.toString();
                    else if(typeN.contains("int",Qt::CaseInsensitive))
                        return dat.toInt();
                    else if(typeN.contains("double",Qt::CaseInsensitive))
                        return dat.toDouble();
                    else if(typeN.contains("bool",Qt::CaseInsensitive))
                        return dat.toBool();
                    else return QVariant();
                }
                else
                    return QVariant();
            }
            else
                return QVariant();
        }
        else
            return QVariant();
    }
    return QVariant();
}


bool VarModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(walker_test)
    {
        if(index.isValid()&&role==Qt::EditRole)
        {
            QString name = names.at(index.row());
            work_map->operator [](name)=value;
            emit dataChanged(index,index);
            emit DatChange(names.at(index.row()));
            return true;
        }
        else
            return false;
    }
    return false;
}

int VarModel::rowCount(const QModelIndex &parent) const
{
        if(parent.isValid())
            return 0;
        return names.size();
}

int VarModel::columnCount(const QModelIndex &parent) const
{
        if(parent.isValid())
            return 0;
        return _c_DATAPOSITION;
}

QVariant VarModel::headerData(int section, Qt::Orientation orientation, int role) const
{
        if(role==Qt::DisplayRole)
        {
            if(orientation==Qt::Horizontal)
            {
                if(section==_c_NUMBERPOSITION)
                    return QString("Name");
                else if(section==_c_INFOPOSITION)
                    return QString("Value");
                else
                    return QVariant();
            }
            else
            {
                return QString::number(section);
            }
        }
        else
            return QVariant();
}

Qt::ItemFlags VarModel::flags(const QModelIndex &index) const
{
     if(!index.isValid())
        return Qt::ItemIsEnabled;

        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if(index.column()==_c_INFOPOSITION)
        {
            flags|=Qt::ItemIsEditable;
        }
        return flags;
}

void VarModel::IncomesData(QString name)
{
    if(walker_test)
    {
        if(names.contains(name))
        {
            if(work_map->contains(name))
            {
                //Update variable, if embedded container has that variable
                emit dataChanged(this->index(names.indexOf(name),_c_INFOPOSITION),this->index(names.indexOf(name),_c_INFOPOSITION));
            }
            else
            {
                //Delete variable from the view in other case
                int riow = names.indexOf(name);
                beginRemoveRows(QModelIndex(),riow,riow);
                names.remove(riow);
                endRemoveRows();
            }
        }
        else
        {
            //Append new variable
            beginInsertRows(QModelIndex(),names.size(),names.size());
            names.append(name);
            endInsertRows();
        }
    }
}


void VarModel::cleartable()
{
    if(!names.isEmpty())
    {
         int size = names.size()-1;
         beginRemoveRows(QModelIndex(),0,size);
         names.clear();
         endRemoveRows();
    }
}

void VarModel::Walk_mode_switch(bool mode)
{
    walker_test=mode;
}


void VarModel::Refresh()
{
    if(walker_test)
    {
        cleartable();
        names.resize(work_map->size());
        beginInsertRows(QModelIndex(),0,work_map->size()-1);
        int i=0;
        for (QMap<QString,QVariant>::iterator var = work_map->begin();var!=work_map->end();)
        {
            names[i++]=var.key();

        }
        endInsertRows();
    }
}

//-------------------------------------------------------------------------
CodeModel::CodeModel(QVector<QString> * dat, QObject *obj):QAbstractTableModel(obj)
{
    walker_test=false;
    var_log=dat;
    num=0;
}

CodeModel::~CodeModel()
{
    var_log=nullptr;
    num=0;
}

QVariant CodeModel::data(const QModelIndex &index, int role) const
{
    if(walker_test)
    {
        if(!index.isValid())
            return QVariant();
        else if(index.row()<0||index.row()>=var_log->size())
            return QVariant();
        else
        {
            if(index.column()==_c_NUMBERPOSITION&&role==Qt::CheckStateRole)
            {
                //The checkbox is representation of an executed string
                if(index.row()==num)
                    return Qt::Checked;
                else
                    return Qt::Unchecked;
            }
            else if(index.column()==_c_INFOPOSITION&&role==Qt::DisplayRole)
                return var_log->at(index.row());
            else
                return QVariant();
        }
    }
    return QVariant();
}

bool CodeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

int CodeModel::rowCount(const QModelIndex &parent) const
{

        if(parent.isValid())
            return 0;
        return var_log->size();
}

int CodeModel::columnCount(const QModelIndex &parent) const
{
        if(parent.isValid())
            return 0;
        return _c_DATAPOSITION;
}

QVariant CodeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
        if(role!=Qt::DisplayRole)
            return QVariant();
        if(orientation== Qt::Horizontal)
        {
            if(section==_c_NUMBERPOSITION)
                return QString("Pos");
            else if(section==_c_INFOPOSITION)
                return QString("Code");
            else
                return QVariant();
        }
        else if(orientation==Qt::Vertical)
        {
            return QString::number(section+1);
        }
        else
            return QVariant();
}

Qt::ItemFlags CodeModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::ItemIsEnabled;
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    return flags;
}

//Метод нужен для информирования представлений об добавлении/удалении
//строк информации. Необходим т.к. обновление данных происходит во внешнем
//классе, а сигналы об обновлении являются защищенными внутри данного класса
//При обновлении строк может учитываться номер строки, по-умолчанию он равен -1
//Но если он больше этого значения, обновление происходит с учетом этого номера.
bool CodeModel::InfoToUpdate(doing variant,int nrow)
{
    switch(variant)
    {
    case INSERT_Begin:
    {
        beginInsertRows(QModelIndex(),nrow>=0?nrow:var_log->size(),nrow>=0?nrow:var_log->size());
    }
        break;
    case INSERT_End:
    {
        endInsertRows();
    }
        break;
    case REMOVE_Begin:
    {
        beginRemoveRows(QModelIndex(),nrow<0?0:nrow,nrow<0?(var_log->size()-1):nrow);
    }
        break;
    case REMOVE_End:
    {
        endRemoveRows();
    }
        break;
    default:
    {
        return false;
    }
        break;
    }
    return true;
}

void CodeModel::RowChanged(int inf)
{
    if(walker_test)
    {
        int old=num;
        num = inf;
        emit dataChanged(this->index(old,_c_NUMBERPOSITION),this->index(old,_c_NUMBERPOSITION));
        emit dataChanged(this->index(num,_c_NUMBERPOSITION),this->index(num,_c_NUMBERPOSITION));
    }
}


void CodeModel::Walk_mode_switch(bool mode)
{
    walker_test=mode;
}

