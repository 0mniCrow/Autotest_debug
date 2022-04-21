#ifndef DATAMODELS_V2_H
#define DATAMODELS_V2_H

#include "ScripterCore_v2.h"

#define _c_NUMBERPOSITION 0       //Number of the message
#define _c_INFOPOSITION 1         //Data
#define _c_DATAPOSITION 2         //Count of columns


/*
 * Obsolete log view.
 * It worked as a tableview
*/
/*
class LogModel:public QAbstractTableModel
{
    Q_OBJECT
private:
    QVector<QPair<int,QString>> work_log;
public:
    LogModel(QObject * obj = nullptr);
    ~LogModel();
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
public slots:
    void IncomesData(const QString info, const int number_of_test);     //Slot catches new data
    void cleartable();
};

*/

/*
 * Representation of the variables for debug mode.
 * The view allows to modify variables during an execution of the script.
*/

class VarModel:public QAbstractTableModel
{
    Q_OBJECT
private:
    bool walker_test;                                                   //Debug flag
    QMap<QString,QVariant> * work_map;                                  //Current variables
    QVector<QString> names;                                             //Variables, that are already loaded
public:
    VarModel(QMap<QString,QVariant> * inf, QObject * obj = nullptr);
    ~VarModel();
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
public slots:
    void Walk_mode_switch(bool mode);
    void Refresh();
    void IncomesData(QString name);
    void cleartable();
signals:
    void DatChange(QString name);                                       //Signal informes about updating of the variable
};

/*
 * Representation of a script code.
 * It works only with debug mode.
*/

class CodeModel:public QAbstractTableModel
{
    Q_OBJECT
private:
    bool walker_test;                                                   //Debug flag
    QVector<QString> *var_log;                                          //Text of the code
    int num;                                                            //A current code string.
public:
    enum doing{INSERT_Begin,INSERT_End,REMOVE_Begin,REMOVE_End};
    CodeModel(QVector<QString> *inf, QObject * obj = nullptr);
    ~CodeModel();
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool InfoToUpdate(doing variant,int nrow = -1);                     //Informs that string is updaated
public slots:
    void RowChanged(int inf);
    void Walk_mode_switch(bool mode);
};

#endif // DATAMODELS_V2_H
