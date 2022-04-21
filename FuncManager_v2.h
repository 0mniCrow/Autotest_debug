#ifndef FUNCMANAGER_V2_H
#define FUNCMANAGER_V2_H
#include "SignalObject_v2.h"
#define func_nulltype "o"
#define OPERATOR_COUNT 5
class FuncManager_v2
{
public:
    enum OperFlags {
        leftAssoc = 0x1,
        mathVal =  0x2
    };

    FuncManager_v2();
    virtual ~FuncManager_v2();
    virtual bool loadAdittionalOperators();
    bool loadNewLibrary(const QString & name);
    bool freeLibrary(const QString & name);
    bool appendOperator(const QString  & name, int priority, int arg_count, char flags=0);
    bool removeOperator(const QString & name);
    bool contains(const QString & name);
    bool isOperator(const QString & name);
    data_ptr callFunction(const std::shared_ptr<FuncObject_v2> &function, QVector<var_ptr> &data);
    QString getError() const;
    QStringList readLog();
    bool changeTypePriority(char lesser, char less, char great, char greater);

    bool isLibLoad(const QString & libname);
    QString getFunctionInterface(const QString &libname, const QString & name);
    QList<QStringList> getAllInterfaces();
    QVariant manualFuncCall(const QString & libname, const QString & name, const QVector<QVariant> & data);

    char op_get_greater(char left, char right) const;
    int op_preced(const QString & op);
    bool op_left_assoc(const QString & op);
    bool op_is_math(const QString & op);
    int op_arg_count(const QString &op);

private:

    typedef void (__cdecl*INIT_DLL)();
    typedef int (__cdecl*FUNC_COUNT)();
    typedef char *(__cdecl*FUNC_NAMES)();
    typedef char *(__cdecl*GET_INTRFCE)(char *);
    typedef char **(__cdecl*CALL_FUNC)(const char *, int, char**);
    typedef void (__cdecl*RELEASE_DLL)();



    struct OriginalOperator
    {
        int priority;
        int operand_count;
        char flags;
        bool left_assoc;
    };

    struct OriginalFunction
    {
        QStringList signatures;
        char * f_name;
        bool is_Operator;
    };
    struct OriginalLibrary
    {
        QString original_name;
        QLibrary library;
        QMap<QString,OriginalFunction*> functions;
        INIT_DLL _initiateDLL;
        FUNC_COUNT _functionsCount;
        FUNC_NAMES _functionsNames;
        GET_INTRFCE _getFunctionInterface;
        CALL_FUNC _callFunction;
        RELEASE_DLL _releaseDll;
        int function_count;
    };

    struct local_foundry
    {
        OriginalLibrary * best_lib;
        QString best_signature;
        int best_count;
        bool function_is_found;
    };

    QMap<QString,OriginalLibrary*> libraries;
    QMap<QString,OriginalOperator*> local_operators;
    QMultiMap<QString,OriginalLibrary *> contained_functions;
    QVector<char> operation_order;
    QStringList log;
    QString last_error;
    bool is_ready;

    bool loadFunctions(OriginalLibrary * p_library);
    OriginalFunction * loadFunction(OriginalLibrary * p_library, const QString & name);
    bool removeFunctions(OriginalLibrary*p_library);
    bool removeFunction(const QString & name, OriginalLibrary * p_library = nullptr);
    char ** data_comp(const QString & func_name, const QVector<var_ptr> & data, local_foundry & selected_lib);
    data_ptr ret_analyse(char **ii_responce/*,const QString & func_name*/);
    QVariant ret_manual(data_ptr value);
    void message(const QString & msg);
    typedef QMap<QString,FuncManager_v2::OriginalLibrary*> lib_map;
    typedef QMap<QString,FuncManager_v2::OriginalFunction*> func_map;
    typedef QMap<QString,FuncManager_v2::OriginalOperator*> op_map;

};

#endif // FUNCMANAGER_V2_H
