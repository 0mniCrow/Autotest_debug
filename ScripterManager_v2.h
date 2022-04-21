#ifndef SCRIPTERMANAGER_V2_H
#define SCRIPTERMANAGER_V2_H
#include "DataModels_v2.h"

class ScripterManager_v2:public QObject
{
    Q_OBJECT
private:
    enum                                    //Inner states of the script manager
    {
        TEST_PAUSE=0x01,
        CODEVIEW_OK=0x02,
        VARVIEW_OK=0x04,
        LOGVIEW_OK=0x08,
        TEST_ACTIVE=0x10,
        TEST_RUN=0x20,
        ISTESTWAS_PAUSED=0x40               //This flag needs only to clear script text in debug mode
    };

    char testerflags;
    bool glob_ret_active;
    int Nline;                              //Number of an executed script line
    int Ntest;                              //Number of the test
    //QTableView * tableforlog;             //!Obsolete
    QTableView * codeview;
    QTableView * variable_show;
    //LogModel * logistic_model;            //!Obsolete
    VarModel * variable_model;
    CodeModel * code_model;
    ScripterCore_v2 * script_core;
    QMap<QString,QVariant> variables;       //Conterner is used for the variable model
    QVector<QString> script_text;
    QThread script_thread;

public:
    enum logs
    {
        AUTOTEST_INFO_LOG,
        AUTOTEST_CODE_LOG,
        AUTOTEST_VARIABLE_LOG
    };
    enum mods
    {
        AUTOTEST_WARNING_MSG=0x01,
        AUTOTEST_INFO_MSG=0x02,
        AUTOTEST_STEP_BY_STEP=0x04,
        AUTOTEST_VARIABLES_EXCHANGE=0x08,
        AUTOTEST_LOAD_RETURN_VAL_AS_GLOBAL = 0x10
    };
    ScripterManager_v2(ScripterCore_v2 * user_core = nullptr, QObject * parent = nullptr);
    void AUTOTEST_Change_mode(bool logic, char mods = AUTOTEST_STEP_BY_STEP);
    bool AUTOTEST_IsFree();
    bool AUTOTEST_IsPaused();
    //bool AUTOTEST_CONNECT_LogVIEW(QTableView * view); //!Obsolete
    bool AUTOTEST_CONNECT_CodeVIEW(QTableView * view);
    bool AUTOTEST_CONNECT_VariablesVIEW(QTableView * view);
    ~ScripterManager_v2();
//Internal communications for working with the scripter core.
signals:
    void __InTest_Load_Script(QString scr_addr, QString set_addr,QString vars);
    void __InTest_Load_DLL(QString dll_addr);
    void __InTest_Ask_for_global();
    void __InTest_Load_Variable(QString name,QVariant data);
    void __InTest_ClearLog(int log);
    void __InTest_Run_Sequance(QStringList text);
    void __InTest_NewLine(int line);                                //Manager informs about of a changed executed string
    void __InTest_Run_next();
    void __InTest_SetPause(bool logic);
    void __InTest_ShowWarningMsg(bool logic);
    void __InTest_ShowInfoMsg(bool logic);
    void __InTest_ShowVariables(bool logic);
    //void __InTest_ThrowLogMsg(QString line, int num);             //!Obsolete
    void __InTest_VarChange(QString name);
    void __InTest_Break_Script();
    void __InTest_Load_Variables(QMap<QString,QVariant> data);
    void __InTest_Remove_Script(QString name);
private slots:
    void __InTest_Ready_to_engage(int NLine);
    void __InTest_Variable_ToChange(QString name);                  //Core informs the manager about an updated variable
    void __InTest_Finished(QVariant line);
    void __InTest_CatchLogMsg(QString msg, int importancy);
    void __InTest_TakeInfo(QMap<QString,QVariant> data);            //Core sends current variables to the manager

//External interface
public slots:
    bool AUTOTEST_Load_Test(const QString & scr_file_addr,
                            const QString & setting_file_addr,
                            const QString & variables,
                            int test_num=-1);
    void AUTOTEST_Load_DLL(const QString & dll_addr);
    bool AUTOTEST_Load_Code(const QStringList & text);
    void AUTOTEST_Load_Variables(const QMap<QString, QVariant> & load_map);
    void AUTOTEST_Start_test();
    void AUTOTEST_Append_Variable(QString name, QVariant data);
    //void AUTOTEST_Clear_Journal();                                //!Obsolete
    void AUTOTEST_Break_Sequance();
    void AUTOTEST_By_Step_mode(bool logic);
    void AUTOTEST_Show_warning_msg(bool logic);
    void AUTOTEST_Show_info_msg(bool logic);
    void AUTOTEST_Load_ret_val_as_global(bool logic);
    void AUTOTEST_Remove_script(QString name);

signals:
    void AUTOTEST_SCRIPT_READY();
    void AUTOTEST_SCRIPT_END(QVariant line);
    void AUTOTEST_MESSGES(QString line, int importancy);
};

#endif // SCRIPTERMANAGER_V2_H
