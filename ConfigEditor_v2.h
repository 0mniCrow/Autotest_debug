#ifndef CONFIGEDITOR_V2_H
#define CONFIGEDITOR_V2_H

#include "predefines.h"

#define SN_NAMECOLUMN 0
#define SN_VALUECOLUMN 1
#define SN_DATASIZE 2

class SettingNode
{
public:
    enum Conversion_Type{CN_toInt,CN_toDouble,CN_toString,CN_toBool};
    explicit SettingNode(QString valname, const QVariant & data, SettingNode * ansestor = nullptr);
    ~SettingNode();
    SettingNode * child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChildren(int position,int count);
    SettingNode * parent();
    bool removeChildren(int position, int count);
    int childNumber() const;
    bool setData(int column, const QVariant & val);
    QString getparentname() const;
    void appendChild(SettingNode * child);
    static bool convertVariant(QVariant*data,Conversion_Type typeTo);
    static Conversion_Type stringType(const QString data);
private:
    QString nameVal;
    QVariant valueDat;
    SettingNode * parentItem;
    QVector<SettingNode*> childItems;
};

class SettingModel:public QAbstractItemModel
{
    Q_OBJECT
public:
    SettingModel(const QStringList & headers, QSettings * data, QObject * parent = nullptr);
    ~SettingModel();
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role=Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    QMap<QString,QVariant> getvariablemap(const QModelIndex & direct);
    bool removeRows(int row, int count, const QModelIndex &parent=QModelIndex()) override;
private:
    void setupModelData(SettingNode * parent);
    QMap<QString,QVariant> collectvariables(SettingNode * parent);
    SettingNode *getItem(const QModelIndex & index) const;
    SettingNode * rootItem;
    QSettings * dataItem;
};
#endif // CONFIGEDITOR_V2_H
