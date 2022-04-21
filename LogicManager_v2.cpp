#include "LogicManager_v2.h"

//Конструктор семафорного класса, инициализирует семафоры для скриптера.
LogicManager::LogicManager()
{
    if_state=false;
    while_state=false;
    if_else_thow=false;
    is_for_active=false;
    if_levels=0;
    while_levels=0;
}

//Метод обработки семафорами события старта условного оператора
void LogicManager::StartCondition(bool condition)
{
        if_else_thow=if_state=condition;
        if_levels++;
        top_type.push(if_floor);
}

//Метод обработки семафорами следующей конструкции условного оператора(elseif/else)
void LogicManager::NextCondition(bool condition)
{
    if(if_else_thow)
        if_state=false;
    else
        if_else_thow=if_state=condition;
}
//Метод обработки семафорами завершения условного оператора
bool LogicManager::EndCondition()
{
    if(if_levels==1)
    {
        if_state=false;
        if_else_thow=false;
        if_levels=0;
        top_type.pop();
        return true;
    }
    else if(if_levels>1)
    {
        if_state=true;
        if_else_thow=true;
        if_levels--;
        top_type.pop();
        return true;
    }
    else
        return false;
}

//Метод обработки семафорами события начала цикла
void LogicManager::StartCycle(bool condition)
{
    while_state=condition;
    while_levels++;
    is_for_active=false;
    cycle_type.push(false);
    top_type.push(while_floor);
}

void LogicManager::StartForCycle(const var_ptr &counter, bool condition, int increment)
{
    while_state=condition;
    while_levels++;
    is_for_active=true;
    for_node temp;
    temp.counter_l=counter;
    temp.increment_l=increment;
    for_layers.push(temp);
    cycle_type.push(true);
    top_type.push(while_floor);
}

//Метод обработски семафорами события сброса до следующей итерации цикла
void LogicManager::ContinueCycle()
{
    while_state=true;
    //!TODO: needs to make a variable or flags that directs main scripter to continue (may be better outside manager)
}

//Метод обработки семафорами события прерывания цикла
void LogicManager::BreakCycle()
{
    while_state=false;
    //EndCycle();
    //!TODO: needs to make a variable or flags that directs main scripter to continue (may be better outside manager)
}

//Метод обработки семафорами события окончания цикла
bool LogicManager::EndCycle()
{
    if(cycle_type.isEmpty())
    {
        return false;
    }

    if(cycle_type.pop())
    {
        VarObject_v2 * info = for_layers.top().counter_l.get();
        int incr = for_layers.top().increment_l;
        *info = (static_cast<int>(*info) + incr);
        for_layers.pop();
    }

    if(while_levels==1)
    {
        while_levels=0;
        while_state = false;
        top_type.pop();
        return true;
    }
    else if(while_levels>1)
    {
        while_levels--;
        while_state=true;
        top_type.pop();
        return true;
    }
    else
        return false;
}

//Метод возвращает true, если семафор зеленого цвета
bool LogicManager::ScriptLine_active() const
{
    if(CycleOk()&&ConditionOk())
        return true;
    else
        return false;
}

//Метод сбрасывает семафор до условия по-умолчанию
void LogicManager::toDefault()
{
    if_state=false;
    while_state=false;
    if_else_thow=false;
    is_for_active=false;
    if_levels=0;
    while_levels=0;
    cycle_type.clear();
    for_layers.clear();

}
