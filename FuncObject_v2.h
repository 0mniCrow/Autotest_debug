#ifndef FUNCOBJECT_V2_H
#define FUNCOBJECT_V2_H
#include"VarObject_v2.h"

typedef std::shared_ptr<DataObject> data_ptr;
typedef std::shared_ptr<VarObject_v2> var_ptr;
typedef QMap<QString,var_ptr> var_map;

class FuncObject_v2:public DataObject
{
public:
    FuncObject_v2(const QString & f_name);
    void setPreferDll(const QString & dll_name);
    QString fName() const;
    QString fPref() const;
    int objType() const override;
private:
    QString name;
    QString prefer_dll;
};

typedef std::shared_ptr<FuncObject_v2> func_ptr;

#endif // FUNCOBJECT_V2_H
