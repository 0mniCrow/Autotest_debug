#ifndef BLOCKOBJECT_V2_H
#define BLOCKOBJECT_V2_H
#include "FuncManager_v2.h"

/*
 * Object equial to a level of hierarchy in the
 * script. Can contain all other objects, including
 * other block objects. Contain strings of the current
 * level of the script, local variables, include
 * parent local variables. Contain a pointer to
 * the current string of the script. It can return a
 * current string of the script, local variables for
 * a current level of this script.
   */

class BlockObject_v2:public DataObject
{
private:
    struct BlockSequance
    {
        QLinkedList<data_ptr> block_line;                       //Script string
        int sence_num;                                          //real number of the string
    };
    QLinkedList<BlockSequance> block_body;                      //List of script strings
    QLinkedList<BlockSequance>::iterator current_line;          //Current string iterator
    var_map local_variables;                                    //Local variables
    //var_map parent_variables;

    BlockObject_v2(const BlockObject_v2 & other) = delete;
    BlockObject_v2 & operator =(const BlockObject_v2 & other) = delete;
public:
    BlockObject_v2(const QString & name = QString());
    ~BlockObject_v2();
    bool appendString(int num, QLinkedList<data_ptr> & string);
    bool appendString(int num, QLinkedList<data_ptr> && string);
    bool setParentVariables(const var_map & vars);
    bool setParentVariables(var_map && vars);
    bool appendLocalVariable(const var_ptr & variable);
    bool appendLocalVariables(var_map & vars);

    bool isEmpty() const;
    bool isBlockEnd() const;
    bool isBlockBegin() const;
    void toBlockBegin();
    QLinkedList<data_ptr> & getCurrentLine();
    var_map getLocalVariables();
    //var_map getParentVariables();


    bool nextString();
    bool prevString();

    int curStrNum() const;
    void refresh(var_map upper_variables);

    int objType() const override { return Tblock; }
};

typedef std::shared_ptr<BlockObject_v2> block_ptr;
#endif // BLOCKOBJECT_V2_H
