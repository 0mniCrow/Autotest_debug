#ifndef EXCELAPP_H
#define EXCELAPP_H
#include "predefines.h"

#define MSG_INFO 0
#define MSG_WARNING 1
#define MSG_ERROR 2
#define EXCEL_LOG_ADDR "ExcelJournal"


class ExcelApp : public QObject//QWidget
{
    Q_OBJECT
private:

    enum ExlHeaders
    {
        HeadCol_num     = 0,
        HeadCol_cfg     = 1,
        HeadCol_test    = 2,
        HeadCol_var     = 3,
        HeadCol_result  = 4,
        HeadCol_min_range =5,
        HeadCol_max_range = 6,
        HeadCol_SIZE    = 7
    };

    struct AXblock
    {
        QAxObject * mainExcel;
        QAxObject * workbooks;
        QAxObject * active_workbook;
        QAxObject * worksheets;
        QAxObject * template_worksheet;
        QAxObject * active_worksheet;
        QAxObject * active_range;
    };

    struct TESTblock
    {
        QVariant _result;
        char _range_ok;
        QString _hex_fileAddr;
        QString _nts_fileAddr;
        QString _var_toLoad;
        QVariant _min_range;
        QVariant _max_range;
        int rownumber;
    };

    struct SEQblock
    {
        QLinkedList<int> _tests;
        QLinkedList<int>::iterator _cur_test;
        QSet<int> _paused;
        int _repeats;
    };

    AXblock AX;                                     //Main ActiveX components of Excel
    int columnNumbers[HeadCol_SIZE];
    SEQblock _sequance;
    QThread data_processing_thread;
    QMap<int, TESTblock> _test_map;
    QMap<int, TESTblock>::iterator _test_iter;
    QVector<int> _debug_list;
    QVector<int> _cell_nums;
    QStringList log;
    QTime timer;
    QString _script_dir;
    QString _conf_dir;
    QString _log_file_n;
    int row_count, column_count, current_test;
    char flags;
    char userflags;

public:
    ///Constructors
    enum ExApp
    {
        ExApp_Default       = 0x00,
        ExApp_QuickStart    = 0x01,
        ExApp_QuickResult   = 0x02,
        ExApp_AutoClearSeq  = 0x04,
        ExApp_WriteResultWhenBreak = 0x08,
        ExApp_ShowInfoMsg   = 0x10,
        ExApp_ShowWarnMsg   = 0x20,
        ExApp_NotShowExcWndw = 0x40,
        ExApp_BreakAfterMiss = 0x80
    };

    enum EFlags
    {
        EF_Null         = 0x00,
        EF_ExcelLoaded  = 0x01,
        EF_SheetLoaded  = 0x02,
        EF_DataLoaded   = 0x04,
        EF_SeqLoaded    = 0x08,
        EF_TestActive   = 0x10,
        EF_BreakActive  = 0x20,
    };

    explicit ExcelApp(/*QWidget*/QObject *parent = 0);
    ExcelApp(const QString &excAddr,
             const QString &templName,
             const QString &sheetName,
             const QString &confDir = QString(),
             const QString &scrDir = QString(),
             char flag = ExApp_Default,
             /*QWidget*/ QObject *parent=nullptr);
    ~ExcelApp();

public slots:
    ///Initialization
    void Excel_reupload_data(const QString & excAddr,
                             const QString & templName,
                             const QString & sheetName,
                             const QString & confDir,
                             const QString & scrDir,
                             char flag);
    void Excel_reupload_seq( QList<QPair<int,int>> * incl_seq,
                             QList<QPair<int,int>> * excl_seq = nullptr,
                             QList<int> * pauses=nullptr,
                             int repeats= 1);



    void Excel_slot_set_worksheet(const QString & sheetName);
    void Excel_slot_set_flags(char flags);
    void Excel_slot_remove_flags(char flags);
    void Excel_slot_clear();

    ///Sequence manipulation
    void Excel_slot_append_test_seq_range(int start_num =-1 , int finish_num=-1);
    void Excel_slot_remove_test_seq_range(int start_num, int finish_num);
    void Excel_slot_clear_test_seq();

    ///Pause manipulation
    void Excel_slot_append_debug_pause(int test_num);
    void Excel_slot_delete_debug_pause(int test_num = -1);
    void Excel_slot_set_repeat_number(int repeat);

    ///External communications
    void Excel_slot_start_tests();
    void Excel_slot_break_tests();
    void Excel_slot_test_complete(QVariant answer);

signals:
    void Excel_signal_next(QString scrAddr,QString confAddr,QString variables, int number);
    void Excel_signal_message(QString text, int status);
    void Excel_signal_setPause(bool status);
    void Excel_signal_state(char status);
    //void Excel_signal_ClearScriptBuffer();


private:

    bool __Excel_slot_initiate (  const QString & excAddr,
                                const QString & templName,
                                const QString & sheetName,
                                const QString & confDir,
                                const QString & scrDir);

    bool __Excel_slot_Excel_load(const QString & excAddr,
                            const QString & templName);
    bool __Excel_slot_Data_load();
    bool __Excel_slot_Parse_header(const QList<QVariant>& header);
    bool __Excel_slot_Parse_data(const QList<QVariant> & data);
    int __Excel_slot_Actual_test_number();
    QAxObject * __Excel_slot_Get_range(  int leftTopRow,
                                    int leftTopColumn,
                                    int rightBottomRow,
                                    int rightBottomColumn);

    bool __Excel_active_contunue();
    bool __Excel_write_quickHeader();
    bool __Excel_write_quickResult(const TESTblock & data);
    bool __Excel_write_Results();
    QAxObject * __Excel_slot_get_range(  int leftTopRow,
                                    int leftTopColumn,
                                    int rightBottomRow,
                                    int rightBottomColumn) const;

    void __Excel_active_finish();
    char __Excel_check_result(const QVariant & min, const QVariant & max, const QVariant & result);

    bool __Excel_check_FileAddr(const QString & fileaddr) const;

    int __Excel_parse_testnum(const QVariant & testnum) const;

    QString __Excel_parse_scriptAdr(const QVariant & addr);
    QString __Excel_parse_confAdr(const QVariant & addr);
    QString __Excel_parse_vars(const QVariant & vars);
    QVariant __Excel_parse_range(const QVariant & value);

    void __Excel_slot_Excel_clear();
    void __Excel_slot_Data_clear();
    void __Excel_slot_Seq_clear();
    void __Excel_slot_result_clear();
    void __Excel_slot_start_clear();

    void __message(const char * message, int status);
    void __message(const QString & message, int status);
    void __writelog();
    void __writelog(const QString & text);
    void __create_current_log_file();
    void __connect_toCatch(QAxObject * obj);
    void __disconnect_fromCatch(QAxObject * obj);
private slots:
    void __catch_AX(int code, const QString & source,
                    const QString & descript,
                    const QString & help);
};

#endif // EXCELAPP_H
