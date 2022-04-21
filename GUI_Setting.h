#ifndef GUI_SETTING_H
#define GUI_SETTING_H

#include "GUI_load_window.h"

namespace Ui {
class GUI_Setting;
}

enum main_status
{
    load_dll_auto = 0x01,
    load_gpib_auto = 0x02,
    load_excel_auto = 0x04,
    show_info = 0x08,
    show_warning = 0x10,
    result_as_global = 0x20
};

enum excel_status
{
    excel_all_test,
    excel_range_test,
    excel_one_test,
};

struct setting_excel
{
    QString exepted_tests;
    QString paused_tests;
    int from_num;
    int for_num;
    int repeats;
    excel_status state;
};


class GUI_Setting : public QWidget
{
    Q_OBJECT

public:
    explicit GUI_Setting(QWidget *parent = 0);
    ~GUI_Setting();


    void load_excel_settings(setting_excel &);
    void load_flags(char flags);

signals:
    void load_dll_manual_btn();
    void load_dll_from_config_btn();
    void load_gpib_manual_btn();
    void load_gpib_from_config_btn();
    void load_excel_manual_btn();
    void load_excel_from_config_btn();
    void start_load_dll(bool);
    void start_load_gpib(bool);
    void start_load_excel(bool);

    void results_to_global_variables(bool);
    void show_info_messages(bool);
    void show_warning_messages(bool);
    void test_file_select();
    void delete_loaded_test(QString);

    void clear_excel_seq();
    void save_excel_settings(setting_excel);
    void start_excel_test();
    void main_status_changed(char);

private slots:

    void hide_excel_variants();
    void delete_tests();
    void excel_data_saved();
    void excel_start_test();
    void status_changed();

private:
    Ui::GUI_Setting *ui;
    setting_excel excel_settings;
    char autotest_flags;

};

#endif // GUI_SETTING_H
