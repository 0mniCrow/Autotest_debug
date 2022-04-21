#include "BlockObject_v2.h"

BlockObject_v2::BlockObject_v2(const QString & name):DataObject(name)
{

}

BlockObject_v2::~BlockObject_v2()
{
    block_body.clear();

    local_variables.clear();
}

bool BlockObject_v2::appendString(int num, QLinkedList<data_ptr> & string)
{
    BlockSequance temp;
    temp.block_line=string;
    temp.sence_num=num;
    block_body.append(temp);
    return true;
}

bool BlockObject_v2::appendString(int num, QLinkedList<data_ptr> && string)
{
    BlockSequance temp;
    temp.block_line=std::move(string);
    temp.sence_num=num;
    block_body.append(temp);
    return true;
}

bool BlockObject_v2::appendLocalVariable(const var_ptr & variable)
{
    if(variable->getVision()!=Vision::Vlocal)
    {
        return false;
    }
    if(!local_variables.contains(variable->getName()))
    {
        local_variables.insert(variable->getName(),variable);
    }
    return true;
}

bool BlockObject_v2::appendLocalVariables(var_map & vars)
{
    var_map::iterator iter = vars.begin();
    while(iter!=vars.end())
    {
        if(appendLocalVariable(iter.value()))
        {
            iter++;
        }
        else
        {
            break;
        }
    }
    return iter==vars.end();
}

bool BlockObject_v2::setParentVariables(const var_map & vars)
{
    if(!local_variables.isEmpty())
    {
        return false;
    }
    local_variables=vars;
    return true;
}

bool BlockObject_v2::setParentVariables(var_map && vars)
{
    bool aok = local_variables.isEmpty();
    local_variables.clear();
    local_variables=std::move(vars);
    return aok;
}

QLinkedList<data_ptr> & BlockObject_v2::getCurrentLine()
{
    return current_line->block_line;
}

var_map BlockObject_v2::getLocalVariables()
{
    return local_variables;
}

/*
var_map BlockObject_v2::getParentVariables()
{
    var_map temp = parent_variables;
    temp.unite(local_variables);
    return temp;
}
*/
/*
void BlockObject_v2::setLocalVariables(const QMap<QString, VarObject_v2 *> &variables)
{
    local_variables = variables;
}
*/

bool BlockObject_v2::isEmpty() const
{
    return block_body.isEmpty();
}
bool BlockObject_v2::isBlockEnd() const
{
    return current_line==block_body.end();
}
bool BlockObject_v2::isBlockBegin() const
{
    return current_line==block_body.begin();
}
void BlockObject_v2::toBlockBegin()
{
    current_line=block_body.begin();
}

bool BlockObject_v2::nextString()
{
    if(current_line==block_body.end())
    {
        return false;
    }
    current_line++;
    return true;
}

bool BlockObject_v2::prevString()
{
    if(current_line==block_body.begin())
    {
        return false;
    }
    current_line--;
    return true;
}

int BlockObject_v2::curStrNum() const
{
    return current_line->sence_num;
}

void BlockObject_v2::refresh(var_map upper_variables)
{
    for(var_map::iterator iter = local_variables.begin(); iter!=local_variables.end();iter++)
    {
        if(!upper_variables.contains(iter.key()))
        {
            if(iter.value()->getVision()!=Vision::Vglobal)
            {
                iter.value()->clear();
            }
        }
    }
}
