#include "ScripterManager_v2.h"

/*
 * Manager creates a script core and a thread for it.
*/
ScripterManager_v2::ScripterManager_v2(ScripterCore_v2 *user_core, QObject *parent):QObject(parent)
{
    glob_ret_active=false;
    Nline = 0;
    Ntest=0;
    testerflags=0;
    if(user_core)
    {
        script_core = user_core;
    }
    else
    {
        script_core = new ScripterCore_v2();
    }

    script_core->moveToThread(&script_thread);

    connect(&script_thread,&QThread::finished,script_core,&QObject::deleteLater);


    connect(this,SIGNAL(__InTest_Load_Script(QString,QString,QString)),
            script_core,SLOT(__slot_load_script(QString,QString,QString)));     //load new/set script ' signal


    connect(this,SIGNAL(__InTest_Load_DLL(QString)),
            script_core,SLOT(__slot_load_library(QString)));                    //load new dll ' signal


    connect(this,SIGNAL(__InTest_Ask_for_global()),
            script_core,SLOT(__slot_ask_for_global_vars()));                    //signal asks the script core for globals


    connect(this,SIGNAL(__InTest_Load_Variable(QString,QVariant)),
            script_core,SLOT(__slot_load_variable(QString,QVariant)));          //load/update single variable ' signal


    connect(this,SIGNAL(__InTest_Load_Variables(QMap<QString,QVariant>)),
            script_core,SLOT(__slot_load_variables(QMap<QString,QVariant>)));   //load/update a group of variables ' signal


    connect(this,SIGNAL(__InTest_Run_next()),
            script_core,SLOT(__slot_start_script()));                           //initiating next step in script ' signal


    connect(this,SIGNAL(__InTest_Run_Sequance(QStringList)),
            script_core,SLOT(__slot_addit_code(QStringList)));                  //loading a code (not a file) ' signal


    connect(this,SIGNAL(__InTest_Break_Script()),
            script_core,SLOT(__slot_break_script()),
            Qt::BlockingQueuedConnection);                                      //breaking a script execution ' signal

    connect(this,SIGNAL(__InTest_Remove_Script(QString)),
            script_core,SLOT(__slot_clear_loading_script(QString)),
            Qt::BlockingQueuedConnection);

    //This connections changes script core mode
    connect(this,SIGNAL(__InTest_SetPause(bool)),
            script_core,SLOT(__slot_turn_pause(bool)));                         //enable step-by-step execution

    connect(this,SIGNAL(__InTest_ShowWarningMsg(bool)),
            script_core,SLOT(__slot_turn_warnings(bool)));                      //turns warning messages

    connect(this,SIGNAL(__InTest_ShowInfoMsg(bool)),
            script_core,SLOT(__slot_turn_info_msg(bool)));                      //turns info messages

    connect(this,SIGNAL(__InTest_ShowVariables(bool)),
            script_core,SLOT(__slot_turn_modify_vars(bool)));                   //turns exchange of variables after step


    //__________________________________________________________________________//
    //                          Signals from the core                           //

    connect(script_core,SIGNAL(__signal_script_endline(QMap<QString,QVariant>)),
            this,SLOT(__InTest_TakeInfo(QMap<QString,QVariant>)));

    connect(script_core,SIGNAL(__signal_global_vars(QMap<QString,QVariant>)),
            this,SLOT(__InTest_TakeInfo(QMap<QString,QVariant>)));

    connect(script_core,SIGNAL(__signal_script_end(QVariant)),
            this,SLOT(__InTest_Finished(QVariant)));

    connect(script_core,SIGNAL(__signal_script_ready(int)),
            this,SLOT(__InTest_Ready_to_engage(int)));

    connect(script_core,SIGNAL(__signal_script_message(QString, int)),
            this,SLOT(__InTest_CatchLogMsg(QString,int)));

    //!Obsolete
    //connect(script_core,SIGNAL(SendFileToLoad(QString,int)),this,SLOT(__InTest_InsertNewCode(QString,int)));
    //connect(script_core,SIGNAL(Script_addActualString(QVector<int>,int)),this,SLOT(__InTest_AppendinActualStr(QVector<int>,int)),Qt::BlockingQueuedConnection);
    //connect(this,SIGNAL(__InTest_Init_GPIB(QMap<QString,QString>)),script_core,SLOT(Initialise_inner_GPIB(QMap<QString,QString>)),Qt::BlockingQueuedConnection);

    script_thread.start();
}

ScripterManager_v2::~ScripterManager_v2()
{

//    if(logistic_model)                    //!Obsolete
//    {
//        logistic_model->deleteLater();
//    }
    if(variable_model)
    {
        variable_model->deleteLater();
    }
    if(code_model)
    {
        code_model->deleteLater();
    }

    script_thread.quit();
    script_thread.wait();
    script_text.clear();
    variables.clear();
}


bool ScripterManager_v2::AUTOTEST_Load_Test(const QString & scr_file_addr,
                        const QString & setting_file_addr,
                        const QString & variables,
                        int test_num)
{
    if(testerflags&TEST_RUN)
    {
        //emit __InTest_ThrowLogMsg("Fail when load new test: previous test is not finished;",Ntest); //!Obsolete
        emit AUTOTEST_MESSGES("TEST N"+QString::number(Ntest)+": previous test is not finished!",scr_WARNING);
        //emit AUTOTEST_SCRIPT_END(QVariant("Error!"));             //!It can be used in future, if user wants it
        return false;
    }

    testerflags|=TEST_RUN;
    if(testerflags&TEST_PAUSE)
    {
        testerflags|=ISTESTWAS_PAUSED;
    }
    if(scr_file_addr.contains('$'))
    {
        Ntest=test_num;
        emit __InTest_Load_Script(scr_file_addr,
                                  setting_file_addr,
                                  variables);
        return true;
    }
    QFile file(scr_file_addr);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        emit AUTOTEST_SCRIPT_END(QVariant("Error!"));
        return false;
    }
    QTextStream reader(&file);
    while(!reader.atEnd())
    {
        if((testerflags&CODEVIEW_OK)&&(testerflags&TEST_PAUSE))
        {
            code_model->InfoToUpdate(CodeModel::INSERT_Begin);
        }
        script_text.append(reader.readLine());
        if((testerflags&CODEVIEW_OK)&&(testerflags&TEST_PAUSE))
        {
            code_model->InfoToUpdate(CodeModel::INSERT_End);
        }
    }
    file.close();
    Ntest=test_num;
    emit __InTest_Load_Script(scr_file_addr,
                              setting_file_addr,
                              variables);
    return true;
}

void ScripterManager_v2::AUTOTEST_Load_DLL(const QString & dll_addr)
{
    emit __InTest_Load_DLL(dll_addr);
}

bool ScripterManager_v2::AUTOTEST_Load_Code(const QStringList & text)   //!+++
{
    if((testerflags&TEST_RUN)||(testerflags&TEST_ACTIVE))
    {
        //emit __InTest_ThrowLogMsg(QString("Fail when load user code: previous test is not finished;"),Ntest); //Obsolete
        emit AUTOTEST_MESSGES("TEST N"+QString::number(Ntest)+": previous test is not finished!",scr_WARNING);
        return false;
    }
    else
    {
        testerflags|=TEST_RUN;
        for(int i = 0; i <text.size();i++)
        {
            if((testerflags&CODEVIEW_OK)&&(testerflags&TEST_PAUSE))
            {
                code_model->InfoToUpdate(CodeModel::INSERT_Begin);
            }
            script_text.append(text.at(i));
            if((testerflags&CODEVIEW_OK)&&(testerflags&TEST_PAUSE))
            {
                code_model->InfoToUpdate(CodeModel::INSERT_End);
            }
        }
        Ntest=-1;
        emit __InTest_Run_Sequance(text);
        return true;
    }
}

void ScripterManager_v2::AUTOTEST_Load_Variables(const QMap<QString, QVariant> & load_map)
{
   emit __InTest_Load_Variables(load_map);
}


void ScripterManager_v2::__InTest_Ready_to_engage(int number)
{
    testerflags|=TEST_ACTIVE;
    Nline = number;
    emit __InTest_NewLine(number);
    if(testerflags&TEST_PAUSE)
    {
        codeview->verticalScrollBar()->setValue(number);
    }
    emit AUTOTEST_SCRIPT_READY();
    return;
}


void ScripterManager_v2::__InTest_Finished(QVariant line)
{
    testerflags&=~TEST_ACTIVE;
    variables.clear();
    if((testerflags&CODEVIEW_OK)&&((testerflags&TEST_PAUSE)||
                                   (testerflags&ISTESTWAS_PAUSED)))
    {
        code_model->InfoToUpdate(CodeModel::REMOVE_Begin);
    }
    script_text.clear();
    if((testerflags&CODEVIEW_OK)&&((testerflags&TEST_PAUSE)||
                                   (testerflags&ISTESTWAS_PAUSED)))
    {
        code_model->InfoToUpdate(CodeModel::REMOVE_End);
    }
    //emit __InTest_ThrowLogMsg("RESULT:"+(line.toString())+";",Ntest);  //!Obsolete
    emit AUTOTEST_MESSGES("TEST N"+QString::number(Ntest)+": RESULT ["+line.toString()+"]",scr_INFO);
    testerflags&=~ISTESTWAS_PAUSED;

    //append return value as variable
    if(glob_ret_active)
    {
        QMap<QString,QVariant> send;
        QString name_val = "test_"+QString::number(Ntest);
        send.insert(name_val,line);
        AUTOTEST_Load_Variables(send);
    }

    Nline=0;
    Ntest=0;
    testerflags&=~TEST_RUN;
    emit AUTOTEST_SCRIPT_END(line);
}


void ScripterManager_v2::__InTest_TakeInfo(QMap<QString, QVariant> data)
{
    QMap<QString,QVariant>::iterator step = variables.begin();
    while(step!=variables.end())
    {
        QMap<QString,QVariant>::iterator understep = data.find(step.key());
        if(understep==data.end())
        {
            QString t_name = step.key();
            step=variables.erase(step);
            emit __InTest_VarChange(t_name);
        }
        else
        {
            step++;
        }
    }

    //Checking for new variables
    for(step=data.begin();step!=data.end();step++)
    {
        QMap<QString,QVariant>::iterator understep = variables.find(step.key());
        if(understep!=variables.end())
        {
            understep.value()=step.value();
        }
        else
        {
            variables.insert(step.key(),step.value());
        }
        emit __InTest_VarChange(step.key());
    }
}


void ScripterManager_v2::AUTOTEST_By_Step_mode(bool logic)
{
    AUTOTEST_Change_mode(logic,AUTOTEST_STEP_BY_STEP);
}

void ScripterManager_v2::AUTOTEST_Show_warning_msg(bool logic)
{
    AUTOTEST_Change_mode(logic,AUTOTEST_WARNING_MSG);
}

void ScripterManager_v2::AUTOTEST_Show_info_msg(bool logic)
{
    AUTOTEST_Change_mode(logic,AUTOTEST_INFO_MSG);
}

void ScripterManager_v2::AUTOTEST_Load_ret_val_as_global(bool logic)
{
    AUTOTEST_Change_mode(logic,AUTOTEST_LOAD_RETURN_VAL_AS_GLOBAL);
}

void ScripterManager_v2::AUTOTEST_Change_mode(bool logic, char mods)
{
    if(mods&AUTOTEST_STEP_BY_STEP)
    {
    if(logic)
    {
        testerflags|=TEST_PAUSE;
        testerflags|=ISTESTWAS_PAUSED;
    }
    else
    {
        testerflags&=~TEST_PAUSE;
    }
    emit __InTest_SetPause(logic);
    }
    if(mods&AUTOTEST_VARIABLES_EXCHANGE)
    {
        emit __InTest_ShowVariables(logic);
    }
    if(mods&AUTOTEST_WARNING_MSG)
    {
        emit __InTest_ShowWarningMsg(logic);
    }
    if(mods&AUTOTEST_INFO_MSG)
    {
        emit __InTest_ShowInfoMsg(logic);
    }
    if(mods&AUTOTEST_LOAD_RETURN_VAL_AS_GLOBAL)
    {
        glob_ret_active=logic;
    }
    return;
}


void ScripterManager_v2::AUTOTEST_Append_Variable(QString name, QVariant data)
{
    if(variables.contains(name))
    {
        variables.find(name).value()=data;
    }
    else
    {
        variables.insert(name,data);
    }
    emit __InTest_VarChange(name);
    emit __InTest_Load_Variable(name,data);
}


bool ScripterManager_v2::AUTOTEST_IsPaused()
{
    return testerflags&TEST_PAUSE;
}


void ScripterManager_v2::AUTOTEST_Start_test()
{
    if(testerflags&TEST_ACTIVE)
    {
        testerflags&=~TEST_ACTIVE;
        emit __InTest_Run_next();
    }
}


void ScripterManager_v2::__InTest_CatchLogMsg(QString msg,int importancy)
{
    //emit __InTest_ThrowLogMsg(msg,Ntest);                         //Obsolete
    emit AUTOTEST_MESSGES(QString::number(Ntest)+": "+msg,importancy);
}


bool ScripterManager_v2::AUTOTEST_CONNECT_CodeVIEW(QTableView *view)
{
    if(view)
    {
        codeview=view;
        QItemSelectionModel * m =codeview->selectionModel();
        code_model = new CodeModel(&script_text);
        codeview->setModel(code_model);
        if(m)
        {
            delete m;
        }
        codeview->setColumnWidth(_c_NUMBERPOSITION,40);
        codeview->setColumnWidth(_c_INFOPOSITION,600);
        connect(this,SIGNAL(__InTest_SetPause(bool)),code_model,SLOT(Walk_mode_switch(bool)));
        connect(this,SIGNAL(__InTest_NewLine(int)),code_model,SLOT(RowChanged(int)));
        testerflags|=CODEVIEW_OK;
        return true;
    }
    return false;
}

//!Obsolete
/*
bool ScripterManager_v2::AUTOTEST_CONNECT_LogVIEW(QTableView *view)
{
    if(view)
    {
        tableforlog=view;
        QItemSelectionModel * m = tableforlog->selectionModel();
        logistic_model = new LogModel();
        tableforlog->setModel(logistic_model);
        if(m)
        {
            delete m;
        }
        tableforlog->setColumnWidth(_c_NUMBERPOSITION,60);
        tableforlog->setColumnWidth(_c_INFOPOSITION,500);
        connect(this,SIGNAL(__InTest_ThrowLogMsg(QString,int,int)),logistic_model,SLOT(IncomesData(QString,int)));
        testerflags|=LOGVIEW_OK;
        return true;
    }
    return false;
}
*/


bool ScripterManager_v2::AUTOTEST_CONNECT_VariablesVIEW(QTableView *view)
{
    if(view)
    {
        variable_show=view;
        QItemSelectionModel * m = variable_show->selectionModel();
        variable_model = new VarModel(&variables);
        variable_show->setModel(variable_model);
        if(m)
        {
            delete m;
        }
        variable_show->setColumnWidth(_c_NUMBERPOSITION,200);
        variable_show->setColumnWidth(_c_INFOPOSITION,400);
        connect(this,SIGNAL(__InTest_VarChange(QString)),variable_model,SLOT(IncomesData(QString)));
        connect(this,SIGNAL(__InTest_SetPause(bool)),variable_model,SLOT(Walk_mode_switch(bool)));
        connect(variable_model,SIGNAL(DatChange(QString)),this,SLOT(__InTest_Variable_ToChange(QString)));
        connect(this,SIGNAL(AUTOTEST_SCRIPT_END(QVariant)),variable_model,SLOT(cleartable()));
        testerflags|=VARVIEW_OK;
        return true;
    }
    return false;
}

//Slot catches variable's name and informs view about change
void ScripterManager_v2::__InTest_Variable_ToChange(QString name)
{
    auto it = variables.find(name);
    emit __InTest_Load_Variable(it.key(),it.value());
}

/*
void ScripterManager_v2::AUTOTEST_Clear_Journal() //!Obsolete
{
     if(testerflags&LOGVIEW_OK)
     {
         logistic_model->cleartable();
     }
}
*/

void ScripterManager_v2::AUTOTEST_Break_Sequance()
{
    if((testerflags&TEST_PAUSE)&&(testerflags&TEST_ACTIVE))
    {
        emit __InTest_Break_Script();
        //__InTest_Finished(QString("Interrupted by user;//"));
    }
}


bool ScripterManager_v2::AUTOTEST_IsFree()
{
    return !(testerflags&TEST_RUN);
}

void ScripterManager_v2::AUTOTEST_Remove_script(QString name)
{
    emit __InTest_Remove_Script(name);
}
