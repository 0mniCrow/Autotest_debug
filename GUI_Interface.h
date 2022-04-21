#ifndef GUI_INTERFACE_H
#define GUI_INTERFACE_H

#include "ScripterManager_v2.h"
#include "ConfigEditor_v2.h"
#include "GUI_Setting.h"
#include "GUI_load_window.h"
//#include "GUI_load_Excel.h"
#define EXCEL_LOG_WINDOW 0
#define SCRIPT_LOG_WINDOW 1

namespace Ui {
class GUI_Interface;
}

class GUI_Interface : public QMainWindow
{
    Q_OBJECT
signals:
    void GUI__signal_start_test(QString address,
                                QString config,
                                QString variables,
                                int num);
    void GUI__signal_test_end(QVariant data);
    void GUI__command_start();
    void GUI__command_load_excelApp(QString addr, QString templ,
                                    QString sheet, QString conf,
                                    QString script, char flags);
    void GUI__command_load_sequances(QList<QPair<int,int>> *,
                                     QList<QPair<int,int>> *,
                                     QList<int> *,
                                     int);
    void GUI__command_append_seqence(int from, int to);
    void GUI__command_append_seqence();
    void GUI__command_remove_seqence(int from, int to);
    void GUI__command_remove_seqences();
    void GUI__command_append_pause(int num);
    void GUI__command_remove_pause(int num);
    void GUI__command_set_flags(char flags);
    void GUI__command_remove_flags(char flags);

public:
    explicit GUI_Interface(const QStringList & keys, QWidget *parent = 0);
    ~GUI_Interface();

public slots:
    void GUI__slot_catch_return_value(QVariant data);
    void GUI__slot_ready_to_engage();
    void GUI__slot_get_update_status(char stat);

private slots:
    void on_start_or_continue_clicked(bool checked);
    void on_InsertItemBtn_clicked(bool checked);
    void on_DeleteItemBtn_clicked(bool checked);
    void on_load_variables_clicked(bool checked);

    void GUI__slot_open_settings_window();

    void GUI__slot_load_info(QMap<QString,QString> list, int data_type);
    void GUI__slot_load_dll_from_config();
    void GUI__slot_load_dll_manualy();
    void GUI__slot_load_gpib_from_config();
    void GUI__slot_load_gpib_manualy();
    void GUI__slot_load_excel_from_config();
    void GUI__slot_load_excel_manualy();
    void GUI__slot_load_script_file();
    void GUI__slot_load_excel_from_settings();
    void GUI__slot_load_flags();
    void GUI__slot_start_excel_test();
    void GUI__slot_save_flags(char flag);
    void GUI__slot_save_excel_settings(setting_excel excel_dat);
    void GUI__slot_initiate_command();

    void GUI__slot_catch_message(QString message, int type);
    void GUI__slot_catch_script(QString message, int type);
    void GUI__slot_infomessage(bool);
    void GUI__slot_load_Excel_object(const QString & excAddr,
                               const QString & templName,
                               const QString & sheetName,
                               const QString & confDir= QString(),
                               const QString & scrDir = QString(),
                               char flag = 0);
    void GUI__slot_load_Excel_seq();


private:
    QString GUI_message_colorize(const QString & msg, int type);
    void GUI_excel_connections();
    void GUI_autotest_connections();
    void GUI_setting_window_connections();
    void GUI_config_connections();
    void GUI_cross_connections();
    void GUI_load_DLL(QMap<QString,QString> & data);
    void GUI_load_Variables(QMap<QString,QString> & data);
    void GUI_load_settings(const QStringList & keys);

    void GUI_wr_sect(const QString & section, const QMap<QString,QVariant> & container);
    void GUI_rd_sect(const QString & section, QMap<QString,QVariant> & container);
    void GUI_wr_conf(const QString & section, const QString & name, const QVariant & value);
    QVariant GUI_rd_conf(const QString & section, const QString & name);
    void GUI_clr_conf(const QString & section, const QString & name = QString());


    Ui::GUI_Interface *ui;
    ScripterManager_v2 interpritator;
    ExcelApp * excelAutomat;
    QSettings * additionalSettings;
    bool ready;
    GUI_Setting * setting_window;
    GUI_load_lib * load_window;
    GUI_load_Excel * load_excel;
};

#endif // GUI_INTERFACE_H
