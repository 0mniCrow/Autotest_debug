#ifndef LOGICMANAGER_H
#define LOGICMANAGER_H
#include "BlockObject_v2.h"


class LogicManager
{
private:
    bool if_state;              //состояние текущего условия в операторе if
    bool while_state;           //состояние текущего условия в операторе while
    bool if_else_thow;          //состояние предыдущих условий для оператора elseif
    bool is_for_active;         //!Будет использоваться при возврате скрипта к голове цикла.
    int if_levels;              //уровень вложенности операторов if
    int while_levels;           //уровень вложенности операторов while
    struct for_node
    {
        var_ptr counter_l;
        int increment_l;
    };

    enum level_type
    {
        if_floor, while_floor
    };

    QStack<level_type> top_type;
    QStack<bool> cycle_type;
    QStack<for_node> for_layers; //!Будет использоваться для проверки текущего цикла на оператор for
public:
    LogicManager();
    void StartCycle(bool condition);       //Сообщение о начале цикла
    void StartForCycle(const var_ptr &counter, bool condition, int increment);
    void ContinueCycle();                               //Сообщение о пропуске шага цикла
    void BreakCycle();                                  //Сообщение о прерывании цикла
    bool EndCycle();                                    //Сообщение об окончании цикла
    void StartCondition(bool condition);                //Сообщение о начале усл.оператора
    void NextCondition(bool condition);                 //Сообщение о следующем условии тек. усл.оператора
    bool EndCondition();                                //Сообщение об окончании усл.оператора
    bool ScriptLine_active() const;                     //Запрос общего состояния у менеджера
    void toDefault();                                   //Сброс семафор

    bool CycleActive() const {return while_levels;}             //Проверка активности цикла
    bool ConditionActive() const {return if_levels;}            //Проверка активности усл.оператора
    bool CycleValid() const {return while_state;}               //Проверка состояния условия цикла
    bool ConditionValid() const {return if_state;}              //Проверка состояния условия усл.оператора
    bool CycleOk() const { return CycleActive()?CycleValid():true; }                    //Проверка пропускного флага цикла
    bool ConditionOk() const { return ConditionActive()?ConditionValid():true; }        //Проверка пропускного флана усл.оператора
    bool ForState() const { return is_for_active; }
    bool IsCycleOnTop() const { return top_type.isEmpty()?false:(top_type.top()==while_floor); }
    bool IsConditiOnTop() const { return top_type.isEmpty()?false:(top_type.top()==if_floor); }

};

#endif // LOGICMANAGER_H
