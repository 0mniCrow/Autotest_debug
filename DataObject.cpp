#include "DataObject.h"

unsigned long DataObject::obj_num = 0UL;

DataObject::DataObject(const QString & name)
{
    obj_name=(name.isEmpty()?("Object_num_"+QString::number(obj_num)):name);    //If the name is empty, object name is generated.
    obj_num++;                                                                  //The counter is incremented.
}

DataObject::DataObject(const DataObject & other)
{
    if(other.obj_name.contains("Object_num_"))
    {
        obj_name = "C_"+other.getName()+"_num_"+QString::number(obj_num);
        obj_num++;
    }
    else
    {
        obj_name = other.obj_name;
    }
}

DataObject & DataObject::operator=(const DataObject &other)
{
    obj_name = "C_"+other.getName()+"_num_"+QString::number(obj_num);
    obj_num++;
    return *this;
}

DataObject::~DataObject()
{
    //obj_num is not decremented
}


unsigned long DataObject::numberofobjects()
{
    return obj_num;
}

void DataObject::changeName(const QString & name)
{
    if(!name.isEmpty())
    {
        obj_name = name;
    }
}

QString DataObject::getName() const
{
    return obj_name;
}

void DataObject::reset()
{
    obj_num = 0UL;
}
