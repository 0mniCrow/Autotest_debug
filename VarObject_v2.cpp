#include "VarObject_v2.h"

//Get new empty C-type string with null-terminator in the end by the length value
char * VarObject_v2::getString(std::size_t len)
{
    char * newstring = new char[len+1];
    newstring[len] ='\0';
    return newstring;
}

//Get new C-type string with copied data from a std string object
char * VarObject_v2::getCharArr(const std::string & data)
{
    char * newstring = getString(data.size());
    data.copy(newstring,data.size());
    return newstring;
}

//Get new C-type string with copied data from another C-type string
char * VarObject_v2::getCharCopy(const char *data)
{
    int len = strlen(data);
    char * newstring = getString(len);
    strncpy(newstring,data,len);
    return newstring;
}

//Sets new data into object
void VarObject_v2::setData(const VarObject_v2::currentData & other, VarObject_v2::Type_v type, bool setApprop)
{
    switch(type)
    {
    case Type_v::Tint:
    {
        data.Dat_I = other.Dat_I;
    }
        break;
    case Type_v::Tdouble:
    {
        data.Dat_D = other.Dat_D;
    }
        break;
    case Type_v::Tbool:
    {
        data.Dat_B = other.Dat_B;
    }
        break;
    case Type_v::Tstring:
    {
        if(setApprop)
        {
            data.Dat_S=other.Dat_S;
        }
        else
        {
            data.Dat_S=getCharCopy(other.Dat_S);
        }
    }
        break;
    default:
    {
        data.Dat_I=0;
    }
        break;
    }
}

//Clears C-string data if object contains it
void VarObject_v2::deleteIfString()
{
    if(type_id==Type_v::Tstring)
    {
        if(data.Dat_S)
        {
            delete [] data.Dat_S;
            data.Dat_S=nullptr;
        }
    }
}

//Basic constructor with no data
VarObject_v2::VarObject_v2(const QString &name):DataObject(name),type_id(Type_v::Tuncorrect),vision_id(Vtemp)
{}

//Constructor with additional data
VarObject_v2::VarObject_v2(const QString &name, Type_v type, Visibility_v vis):
    DataObject(name),type_id(type),vision_id(vis)
{
    switch(type)
    {
    case Type_v::Tint:
    {
        data.Dat_I=0;
    }
        break;
    case Type_v::Tdouble:
    {
        data.Dat_D=0.0;
    }
        break;
    case Type_v::Tbool:
    {
        data.Dat_B=false;
    }
        break;
    case Type_v::Tstring:
    {
        data.Dat_S = new char[1];
        data.Dat_S[0]='\0';
    }
        break;
    default:
        break;
    }
}

//Copy constructor
VarObject_v2::VarObject_v2(const VarObject_v2 &other):DataObject(other),type_id(other.type_id),vision_id(Vtemp)
{
    /*
    switch(type_id)
    {
    case Type_v::Tint:
    {
        data.Dat_I = other.data.Dat_I;
    }
        break;
    case Type_v::Tdouble:
    {
        data.Dat_D = other.data.Dat_D;
    }
        break;
    case Type_v::Tbool:
    {
        data.Dat_B = other.data.Dat_B;
    }
        break;
    case Type_v::Tstring:
    {
        data.Dat_S=getCharCopy(other.data.Dat_S);
    }
        break;
    default:
    {
        data.Dat_I=0;
    }
        break;
    }
    */
    setData(other.data,other.type_id,false);
}

//Move constructor
VarObject_v2::VarObject_v2(VarObject_v2 && other):DataObject(other),type_id(other.type_id),vision_id(Vtemp)
{
    /*
    switch(type_id)
    {
    case Type_v::Tint:
    {
        data.Dat_I = other.data.Dat_I;
        other.data.Dat_I = 0;
    }
        break;
    case Type_v::Tdouble:
    {
        data.Dat_D = other.data.Dat_D;
        other.data.Dat_D = 0.0;
    }
        break;
    case Type_v::Tbool:
    {
        data.Dat_B = other.data.Dat_B;
        other.data.Dat_B= false;
    }
        break;
    case Type_v::Tstring:
    {
        data.Dat_S=other.data.Dat_S;
        other.data.Dat_S = nullptr;
    }
        break;
    default:
        break;
    }
    */
    setData(other.data,other.type_id,true);
    other.data.Dat_I=0;
    other.type_id = Type_v::Tuncorrect;
}

//Typed constructors:
VarObject_v2::VarObject_v2(int var):DataObject(), type_id(Type_v::Tint), vision_id(Vtemp)
{
    data.Dat_I = var;
}

VarObject_v2::VarObject_v2(double var):DataObject(), type_id(Type_v::Tdouble), vision_id(Vtemp)
{
    data.Dat_D = var;
}

VarObject_v2::VarObject_v2(bool var):DataObject(), type_id(Type_v::Tbool), vision_id(Vtemp)
{
    data.Dat_B = var;
}

VarObject_v2::VarObject_v2(const char * var):DataObject(),type_id(Type_v::Tstring), vision_id(Vtemp)
{
    if(var)
    {
        data.Dat_S = getCharCopy(var);
    }
    else
    {
        data.Dat_S = new char[1];
        data.Dat_S[0]='\0';
    }
}

//Appropriation operators
VarObject_v2 & VarObject_v2::operator = (const VarObject_v2 & other)
{
    deleteIfString();
    setData(other.data,other.type_id,false);
    type_id=other.type_id;
    return *this;
}

VarObject_v2 & VarObject_v2::operator = (VarObject_v2 && other)
{
    deleteIfString();
    setData(other.data,other.type_id,true);
    type_id=other.type_id;
    other.data.Dat_I=0;
    other.type_id=Type_v::Tuncorrect;
    return *this;
}

VarObject_v2 & VarObject_v2::operator = (int var)
{
    deleteIfString();
    data.Dat_I=var;
    type_id = Type_v::Tint;
    return *this;
}

VarObject_v2 & VarObject_v2::operator = (double var)
{
    deleteIfString();
    data.Dat_D=var;
    type_id = Type_v::Tdouble;
    return *this;
}

VarObject_v2 & VarObject_v2::operator = (bool var)
{
    deleteIfString();
    data.Dat_B=var;
    type_id = Type_v::Tbool;
    return *this;
}

VarObject_v2 & VarObject_v2::operator = (const char * var)
{
    deleteIfString();
    data.Dat_S=getCharCopy(var);
    type_id = Type_v::Tstring;
    return *this;
}

//Get the data for variable from a QVariant object
VarObject_v2 & VarObject_v2::operator = (const QVariant & var)
{
    if(!var.isValid())
    {
        return * this;
    }
    deleteIfString();
    switch(var.userType())
    {
    case QMetaType::Int:
    {
        type_id = Type_v::Tint;
        data.Dat_I = var.toInt();
    }
        break;
    case QMetaType::Double:
    {
        type_id = Type_v::Tdouble;
        data.Dat_D = var.toDouble();
    }
        break;
    case QMetaType::Bool:
    {
        type_id = Type_v::Tbool;
        data.Dat_D = var.toBool();
    }
        break;
    case QMetaType::QString:
    {
        type_id = Type_v::Tstring;
        data.Dat_S = getCharCopy(var.toString().toLatin1().data());
    }
        break;
    default:
    {
        type_id = Type_v::Tuncorrect;
        data.Dat_I=0;
    }
        break;
    }
    return *this;
}

//Cast operators
VarObject_v2::operator int() const
{
    currentData local_data;
    if(convert_to(Type::Tint,local_data))
    {
        return local_data.Dat_I;
    }
    return 0;
}

VarObject_v2::operator double() const
{
    currentData local_data;
    if(convert_to(Type::Tdouble,local_data))
    {
        return local_data.Dat_D;
    }
    return 0.0;
}

VarObject_v2::operator bool() const
{
    currentData local_data;
    if(convert_to(Type::Tbool,local_data))
    {
        return local_data.Dat_B;
    }
    return false;
}

VarObject_v2::operator char*() const
{
    return getData();
}

//Destructor
VarObject_v2::~VarObject_v2()
{
   deleteIfString();
}

//Sets a new visibility level for variable and returns old one
VarObject_v2::Visibility_v VarObject_v2::setVision(Visibility_v new_vis)
{
    Visibility_v old_vis= vision_id;
    vision_id = new_vis;
    return old_vis;
}

//Clears local and global objects, does nothing to temp and inline variables
void VarObject_v2::clear()
{
    if((vision_id==Vlocal)||(vision_id==Vglobal))
    {
        deleteIfString();
        data.Dat_I=0;
        type_id=Type::Tvoid;
    }
}

//Static method makes a convertion a variable to the new type. Can fail.
bool VarObject_v2::convert_to(Type type, currentData & container) const
{
    if(!(
        (type==Type_v::Tint)||
        (type==Type_v::Tdouble)||
        (type==Type_v::Tbool)||
        (type==Type_v::Tstring)
      ) )
    {
        return false;
    }

    switch(type)
    {
    case Type_v::Tint:
    {
        switch(type_id)
        {
        case Type::Tint:
        {
            container.Dat_I = data.Dat_I;
        }
            break;
        case Type_v::Tdouble:
        {
            container.Dat_I = static_cast<int>(data.Dat_D);
        }
            break;
        case Type::Tbool:
        {
            container.Dat_I = (data.Dat_B? 1:0);
        }
            break;
        case Type::Tstring:
        {
            try
            {
                container.Dat_I = (std::stoi(data.Dat_S,0,0));
            }
            catch(const std::invalid_argument & ia)
            {
                return false;
            }
        }
            break;
        case Type::Tvoid:
        {
            container.Dat_I = 0;
        }
            break;
        default:
        {
            return false;
        }
            break;
        }
    }
        break;
    case Type::Tdouble:
    {
        switch(type_id)
        {
        case Type::Tint:
        {
            container.Dat_D = static_cast<double>(data.Dat_I);
        }
            break;
        case Type::Tdouble:
        {
            container.Dat_D = data.Dat_D;
        }
            break;
        case Type::Tbool:
        {
            container.Dat_D = (data.Dat_B? 1.0:0.0);
        }
            break;
        case Type::Tstring:
        {
            try
            {
                container.Dat_D = (std::stod(data.Dat_S));
            }
            catch(const std::invalid_argument & ia)
            {
                return false;
            }
        }
            break;  
        case Type::Tvoid:
        {
            container.Dat_D = 0.0;
        }
            break;
        default:
        {
            return false;
        }
            break;
        }
    }
        break;
    case Type::Tbool:
    {
            switch(type_id)
            {
            case Type::Tint:
            {
                container.Dat_B = static_cast<bool>(data.Dat_I);
            }
                break;
            case Type::Tdouble:
            {
                container.Dat_B = static_cast<bool>(data.Dat_D);
            }
                break;
            case Type::Tbool:
            {
                container.Dat_B = data.Dat_B;
            }
                break;
            case Type::Tstring:
            {
                container.Dat_B = data.Dat_S;
            }
                break;
            case Type::Tvoid:
            {
                container.Dat_B = false;
            }
                break;
            default:
            {
                return false;
            }
                break;
            }
    }
        break;
    case Type::Tstring:
    {
        container.Dat_S = nullptr;
        switch(type_id)
        {
        case Type::Tint:
        {
            container.Dat_S = getCharCopy(std::to_string(data.Dat_I).c_str());
        }
            break;
        case Type::Tdouble:
        {
            container.Dat_S = getCharCopy(std::to_string(data.Dat_D).c_str());
        }
            break;
        case Type::Tbool:
        {
            container.Dat_S = getCharCopy(data.Dat_B?"true":"false");
        }
            break;
        case Type::Tstring:
        {
            container.Dat_S = getCharCopy(data.Dat_S);
        }
            break;
        case Type::Tvoid:
        {
            container.Dat_S = getCharCopy(" ");
        }
            break;
        default:
        {
            return false;
        }
            break;
        }
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

//Get a data from the variable
char * VarObject_v2::getData() const
{
    currentData local_data;
    if(convert_to(Type::Tstring,local_data))
    {
        return local_data.Dat_S;
    }
    return nullptr;
}

//Get a type of the variable
Type VarObject_v2::getType() const
{
    return type_id;
}

//Get a char-name of type of the variable
char VarObject_v2::getCtype() const
{
    return static_cast<char>(type_id);
}

//overrided function
int VarObject_v2::objType() const
{
    return Tvariable;
}

//Get a vision level of the variable
Vision VarObject_v2::getVision() const
{
    return vision_id;
}

VarObject_v2 VarObject_v2::operator()(Type_v type) const
{
    switch (type) {
    case Type_v::Tint:
    {
        return std::move(VarObject_v2(static_cast<int>(*this)));
    }
        break;
    case Type_v::Tdouble:
    {
        return std::move(VarObject_v2(static_cast<double>(*this)));
    }
        break;
    case Type_v::Tstring:
    {
        return std::move(VarObject_v2(static_cast<char*>(*this)));
    }
        break;
    case Type_v::Tbool:
    {
        return std::move(VarObject_v2(static_cast<bool>(*this)));
    }
        break;
    default:
    {
        return VarObject_v2();
    }
        break;
    }
}
