/*
 * A basic abstract object that contains a name of object and
 * a static counter for creating an unique names when a user
 * doesn't set the name in the constructor. All object units
 * inherits from this object. This one has an virtual function
 * objType() that must be override in the inherits.
*/

#ifndef DATAOBJECT_H
#define DATAOBJECT_H
#include "MemoryDirector.h"
//!Need to find how to use a shared pointer for data objects
class DataObject {
private:
    static unsigned long obj_num;
    QString obj_name;
public:

    explicit DataObject(const QString & name = QString());
    DataObject(const DataObject & other);
    DataObject & operator=(const DataObject &other);
    virtual ~DataObject();
    virtual int objType() const = 0;
    static unsigned long numberofobjects();
    void changeName(const QString & name);
    QString getName() const;
    static void reset();
};

#endif // DATAOBJECT_H
