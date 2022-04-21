#ifndef SCRIPTERCORE_V2_H
#define SCRIPTERCORE_V2_H
#include "LogicManager_v2.h"
#define SCRIPTER_LOG_ADDR "ScriptJournal"

class ScripterCore_v2:public QObject
{
    Q_OBJECT
public:

    ScripterCore_v2(FuncManager_v2 * func_manager = nullptr);
    ~ScripterCore_v2();

    enum states{
        IsFalse = 0,
        IsLoading = 1,
        IsReady = 2,
        IsRun = 3,
        IsFree = 4,
        IsBreak = 5,
        IsError = 6,
        IsSuccess = 7
    };
    enum flags{
        StepModeOn = 0x01,
        SendWarningMessages = 0x02,
        SendInfoMessages = 0x04,
        ModifyVars = 0x08,
        MissOneStep = 0x10,
        LogFileIsSet = 0x20
    };

public slots:
    void __slot_load_script(const QString & address,
                       const QString & settings,
                       const QString & variables);
    void __slot_start_script();
    void __slot_load_library(const QString & address);
    void __slot_set_function_core(FuncManager_v2 * newcore);
    void __slot_turn_pause(bool logic);
    void __slot_turn_warnings(bool logic);
    void __slot_turn_info_msg(bool logic);
    void __slot_turn_modify_vars(bool logic);
    void __slot_ask_for_global_vars();
    void __slot_break_script();
    void __slot_addit_code(const QStringList & data);
    void __slot_load_variables(const QMap<QString,QVariant> & data);
    void __slot_load_variable(const QString & name, const QVariant &data);
    void __slot_clear_loading_script(const QString & name = QString());
    void __slot_is_library_load(const QString & libname);
    void __slot_get_function(const QString &libname, const QString & name);
    void __slot_get_all_functions();
    void __slot_call_function(const QString & libname, const QString & name, const QVector<QVariant> & data);

signals:
    void __signal_script_ready(int NLine);
    void __signal_script_endline(QMap<QString,QVariant> data);
    void __signal_script_end(QVariant data);
    void __signal_script_message(QString message, int importancy);
    void __signal_global_vars(QMap<QString,QVariant> data);
    void __signal_get_function(QString interface);
    void __signal_is_library_load(bool logic);
    void __signal_get_all_functions(QList<QStringList> interfaces);
    void __signal_call_function(QVariant result);

private:
    enum inner_types{
        token_uncorrect = 0,
        t_variable = 1,
        t_signal = 2,
        t_function = 3,
        t_block = 4,
        t_void = 5,
        t_int = 6,
        t_double = 7,
        t_string = 8,
        t_bool = 9,
        t_func=10,
        t_match=11
    };
    QTime script_timer;
    LogicManager scriptManager;
    QMap<inner_types,QRegExp> scanner;                //                Contains regular expressions
    QMap<QString, block_ptr> scrPool;                 //    Contains downloaded and analysed scripts
    QStack<block_ptr> runSeq;                         //   Contains a current script(level of blocks)
    QStack<block_ptr> add_on_blocks;                  //             Contains user-downloaded blocks
    var_map globVars;                                 //                   Contains global variables
    FuncManager_v2 * functor;                         //          This object uses to call functions
    int curLine;                                      //           Number of the for-user-logic line
    unsigned char scrFlags;                           //                                Object flags
    states scrState;
    QVector<var_ptr> load_vars;                       //      Variables, that comes with script call
    QString config_file;                              //         String contains config file to load
    QStringList scripter_log;                         //           File, that contains state journal
    QVariant return_value;
    QFile inlogfile;
    QTextStream inlogstream;

    //Script loading methods:
    void message(const QString & text, int importancy=scr_INFO);
    void message(const char * text, int inportancy=scr_INFO);
    void startLog();
    void finishLog();
    void loadMessages(const QStringList & msgs);




    char **             __inScr__appendstring(char ** old_char, int size,
                                                char * new_char) const;
    char *              __inScr__partofstring(const char * string,
                                                int position, int size) const;
    int                 __inScr__code_splitter(const QStringList & script_text,
                                                QLinkedList<QStringList*> & container);
    block_ptr           __inScr__code_analyser(const QLinkedList<QStringList *> & data,
                                                block_ptr * parent = nullptr);
    bool                __inScr__token_analyser(const QStringList & list,
                                                QLinkedList<data_ptr> & container,
                                                var_map & local_variables,
                                                block_ptr & contain_block, int & block_pos);
    QMap<inner_types,
    QRegExp>::iterator  __inScr__get_token_type(const QString & data);
    bool                __inScr__code_sort(QLinkedList<data_ptr> & code_line);

    //Script execution methods:
    data_ptr            __inScr__calculate_step(block_ptr & data);
    bool                __inScr__change_current_state(signal_ptr & func_answer,
                                                      QVector<var_ptr> *data);
    virtual bool        changeAdditionalState(signal_ptr & func_answer,
                                              QVector<var_ptr> *data);
    bool                __inScr__command_append_block(const QString & name);

    //Script manage methods:
    QStringList         __inScr__open_file(const QString & file_name);
    block_ptr           __inScr__code_disposer(const QString & address, block_ptr * parent=nullptr);
    bool                __inScr__set_current_script(const QString & address);
    bool                __inScr__script_start();
    block_ptr           __inScr__script_get_actual_block(bool secret=false);
    block_ptr           __inScr__script_eject_current_block();
    bool                __inScr__scan_calculator_answer(data_ptr & answer);
    bool                __inScr__script_continue(int line);
    bool                __inScr__check_end();
    bool                __inScr__script_end();
    bool                __inScr__append_extern_block(const QString & addr);
    void                __inScr__set_state(states new_state);
    void                __inScr__set_flag(flags new_flag);
    void                __inScr__remove_flag(flags rem_flag);
    QVariant            __inScr__get_value(var_ptr & variable);
    var_ptr             __inScr__get_variable(const QString & info);
    void                __inScr__flush_log_in_file();
    var_map             __inScr__actual_variables(var_map & local_map);

    //Deleted consturctors:
                        ScripterCore_v2(const ScripterCore_v2 & other) = delete;
    void                operator = (const ScripterCore_v2 & other) = delete;



};

#endif // SCRIPTERCORE_V2_H
