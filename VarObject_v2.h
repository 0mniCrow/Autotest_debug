#ifndef VAROBJECT_V2_H
#define VAROBJECT_V2_H
#include "DataObject.h"

class VarObject_v2:public DataObject
{
public:
    enum class Type_v : char              //Enumination of the variable types.
    {
        Tuncorrect = 'e',
        Tvoid = 'v',
        Tint = 'i',
        Tdouble = 'd',
        Tstring = 's',
        Tbool = 'b'
    };

    enum Visibility_v            //Enumination of the vision levels.
    {
        Vtemp,                  //temporary variables delete after line cycle
        Vline,                  //inline variables are parts of script line. They are clear after line cycle
        Vlocal,                 //local variables are user-set variables. They are clear after script finish
        Vglobal                 //global variables are user-set and user-pronounced.They don't clear after script finish.
    };

    VarObject_v2(const QString &name = QString());
    VarObject_v2(const QString &name, Type_v type,             //Constructor for local variables with-
                       Visibility_v vis);                      //-mandatory name, type and visibility.
    VarObject_v2(const VarObject_v2 & other);
    VarObject_v2(VarObject_v2 && other);
    VarObject_v2(int var);
    VarObject_v2(double var);
    VarObject_v2(const char * var);
    VarObject_v2(bool var);
    VarObject_v2 & operator= (const VarObject_v2 & other);
    VarObject_v2 & operator= (VarObject_v2 && other);
    VarObject_v2 & operator= (int var);
    VarObject_v2 & operator= (double var);
    VarObject_v2 & operator= (bool var);
    VarObject_v2 & operator= (const char * var);
    VarObject_v2 & operator= (const QVariant & var);
    operator int() const;                                      //Cast operator returns int (0 if fail).
    operator double() const;                                   //Cast operator ret double (0.0 if fail).
    operator char * () const;                                  //Cast operator ret char* (null if fail).
    operator bool () const;                                    //Cast operator ret bool (false if fail).
    VarObject_v2 operator()(Type_v type) const;

    ~VarObject_v2();

    Visibility_v    setVision(Visibility_v new_vis);          //Method that changes a var vis level.
    char *          getData() const;                          //Method returns a variable value.
    Type_v          getType() const;
    char            getCtype() const;
    int             objType() const override;                 //Override method returns token type.
    Visibility_v    getVision() const;                        //Method returns a var vision level.
    void            clear();                                  //Method to clear variable value (only for local/global types).

private:

    union currentData                                         //A union for the value of the
    {                                                         //VarObject data.
        int     Dat_I;
        double  Dat_D;
        bool    Dat_B;
        char *  Dat_S;
    }           data;
    Type_v type_id;                                           //Type of the variable
    Visibility_v vision_id;                                   //Visibility level of the variable

    void setData(const currentData & info,                    //Method sets new data into variable. The way-
                 Type_v type, bool setApprop);                //- to set it depends of appropriation flag
    static char * getString(std::size_t len);                 //Creates empty string with length len
    static char * getCharArr(const std::string & sdata);      //Creates new string with data copied from sdata
    static char * getCharCopy(const char * data);             //Creates a copy of C-string
    void deleteIfString();                                    //Delete C-string data if it contains
    bool convert_to(Type_v type,                              //Method converse data to elemental type and put-
                    currentData & container) const;           //it to the container. Return success of the conversion

};

typedef VarObject_v2::Type_v Type;
typedef VarObject_v2::Visibility_v Vision;

#endif // VAROBJECT_V2_H
