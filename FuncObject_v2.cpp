#include "FuncObject_v2.h"

FuncObject_v2::FuncObject_v2(const QString & f_name):DataObject(f_name),name(f_name)
{}

void FuncObject_v2::setPreferDll(const QString &dll_name)
{
    prefer_dll=dll_name;
}

QString FuncObject_v2::fName() const
{
    return name;
}

QString FuncObject_v2::fPref() const
{
    return prefer_dll;
}

int FuncObject_v2::objType() const
{
    return Tfunction;
}
