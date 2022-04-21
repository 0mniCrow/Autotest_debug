#include "GUI_Setting.h"
#include "ui_GUI_Setting.h"

GUI_Setting::GUI_Setting(QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::GUI_Setting)
{
    ui->setupUi(this);
    connect(ui->load_dll_man_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_dll_manual_btn()));
    connect(ui->load_dll_from_start_cbx,SIGNAL(clicked(bool)),this,SIGNAL(start_load_dll(bool)));
    connect(ui->load_dll_from_conf_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_dll_from_config_btn()));
    connect(ui->load_dll_from_start_cbx,SIGNAL(clicked(bool)),this,SLOT(status_changed()));

    connect(ui->load_gpib_man_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_gpib_manual_btn()));
    connect(ui->load_gpib_from_start_cbx,SIGNAL(clicked(bool)),this,SIGNAL(start_load_gpib(bool)));
    connect(ui->load_gpib_from_conf_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_gpib_from_config_btn()));
    connect(ui->load_gpib_from_start_cbx,SIGNAL(clicked(bool)),this,SLOT(status_changed()));

    connect(ui->load_excel_man_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_excel_manual_btn()));
    connect(ui->load_excel_from_start_cbx,SIGNAL(clicked(bool)),this,SIGNAL(start_load_excel(bool)));
    connect(ui->load_excel_from_conf_btn,SIGNAL(clicked(bool)),this,SIGNAL(load_excel_from_config_btn()));
    connect(ui->load_excel_from_start_cbx,SIGNAL(clicked(bool)),this,SLOT(status_changed()));

    connect(ui->results_to_globvar,SIGNAL(clicked(bool)),this,SIGNAL(results_to_global_variables(bool)));
    connect(ui->info_msg_cbx,SIGNAL(clicked(bool)),this,SIGNAL(show_info_messages(bool)));
    connect(ui->warning_msg_cbx,SIGNAL(clicked(bool)),this,SIGNAL(show_warning_messages(bool)));
    connect(ui->select_file_test_btn,SIGNAL(clicked(bool)),this,SIGNAL(test_file_select()));
    connect(ui->delete_test_btn,SIGNAL(clicked(bool)),this,SLOT(delete_tests()));
    connect(ui->excel_save_settings_btn,SIGNAL(clicked(bool)),this,SLOT(excel_data_saved()));
    connect(ui->excel_start_btn,SIGNAL(clicked(bool)),this,SLOT(excel_start_test()));

    connect(ui->results_to_globvar,SIGNAL(clicked(bool)),this,SLOT(status_changed()));
    connect(ui->info_msg_cbx,SIGNAL(clicked(bool)),this,SLOT(status_changed()));
    connect(ui->warning_msg_cbx,SIGNAL(clicked(bool)),this,SLOT(status_changed()));

    connect(ui->excel_all_tests,SIGNAL(clicked(bool)),this,SLOT(hide_excel_variants()));
    connect(ui->excel_between_tests,SIGNAL(clicked(bool)),this,SLOT(hide_excel_variants()));
    connect(ui->excel_one_test,SIGNAL(clicked(bool)),this,SLOT(hide_excel_variants()));
    connect(ui->excel_repeat_cbx,SIGNAL(clicked(bool)),ui->excel_repeat_spin,SLOT(setEnabled(bool)));
    connect(ui->excel_clear_seq_btn,SIGNAL(clicked(bool)),this,SIGNAL(clear_excel_seq()));
}

GUI_Setting::~GUI_Setting()
{
    delete ui;
}

void GUI_Setting::load_excel_settings(setting_excel & excel_dat)
{
    std::swap(excel_settings, excel_dat);
    ui->excel_pause_items->setText(excel_settings.paused_tests);
    ui->excel_exclude_test_line->setText(excel_settings.exepted_tests);
    switch(excel_settings.state)
    {
    case excel_all_test:
    {
        ui->excel_all_tests->setChecked(true);   
        excel_settings.for_num=0;
        excel_settings.from_num=0;
    }
        break;
    case excel_range_test:
    {
        ui->excel_between_tests->setChecked(true);
        ui->excel_test_from_line->setText(QString::number(excel_settings.from_num));
        ui->excel_test_to_line->setText(QString::number(excel_settings.for_num));
    }
        break;
    case excel_one_test:
    {
        ui->excel_one_test->setChecked(true);
        ui->excel_test_num_line->setText(QString::number(excel_settings.from_num));
        excel_settings.for_num=0;
        excel_settings.exepted_tests.clear();
    }
        break;
    default:
    {
        ui->excel_all_tests->setChecked(true);
    }
        break;
    }
    if(excel_settings.repeats>1)
    {
        ui->excel_repeat_cbx->setChecked(true);
        ui->excel_repeat_spin->setEnabled(true);
        ui->excel_repeat_spin->setValue(excel_settings.repeats);
    }
    hide_excel_variants();
}

void GUI_Setting::load_flags(char flags)
{
    autotest_flags = flags;
    if(autotest_flags&load_dll_auto)
    {
        ui->load_dll_from_start_cbx->setChecked(true);
    }
    if(autotest_flags&load_gpib_auto)
    {
        ui->load_gpib_from_start_cbx->setChecked(true);
    }
    if(autotest_flags&load_excel_auto)
    {
        ui->load_excel_from_start_cbx->setChecked(true);
    }
    if(autotest_flags&show_info)
    {
        ui->info_msg_cbx->setChecked(true);
    }
    if(autotest_flags&show_warning)
    {
        ui->warning_msg_cbx->setChecked(true);
    }
    if(autotest_flags&result_as_global)
    {
        ui->results_to_globvar->setChecked(true);
    }
}

void GUI_Setting::delete_tests()
{
    emit delete_loaded_test(ui->test_for_delete_line->text());
}

void GUI_Setting::excel_data_saved()
{
    QString exclude(ui->excel_exclude_test_line->text());
    bool aok = true;
    foreach(QChar ch, exclude)
    {
        if(ch.isDigit()||(ch.toLatin1()=='-')||(ch.toLatin1()==' '))
        {
            continue;
        }
        else
        {
            aok=false;
        }
    }
    if(!aok)
    {
        ui->excel_exclude_test_line->setStyleSheet("background: red");
        return;
    }
    else
    {
        ui->excel_exclude_test_line->setStyleSheet("background: white");
    }

    excel_settings.exepted_tests = exclude;

    exclude = ui->excel_pause_items->text();
    foreach(QChar ch, exclude)
    {
        if(ch.isDigit()||(ch.toLatin1()==' '))
        {
            continue;
        }
        else
        {
            aok=false;
        }
    }
    if(!aok)
    {
        ui->excel_pause_items->setStyleSheet("background: red");
        return;
    }
    else
    {
        ui->excel_pause_items->setStyleSheet("background: white");
    }

    excel_settings.paused_tests = ui->excel_pause_items->text();
    if(ui->excel_all_tests->isChecked())
    {
        excel_settings.state = excel_all_test;
        excel_settings.for_num = 0;
        excel_settings.from_num = 0;
        excel_settings.exepted_tests = ui->excel_exclude_test_line->text();
    }
    else if(ui->excel_between_tests->isChecked())
    {
        excel_settings.state = excel_range_test;
        bool aok;
        excel_settings.from_num = ui->excel_test_from_line->text().toInt(&aok,0);
        excel_settings.for_num = ui->excel_test_to_line->text().toInt(&aok,0);
        excel_settings.exepted_tests = ui->excel_exclude_test_line->text();
    }
    else if(ui->excel_one_test->isChecked())
    {
        bool aok;
        excel_settings.state = excel_one_test;
        excel_settings.from_num = ui->excel_test_num_line->text().toInt(&aok,0);
        excel_settings.for_num = 0;
    }
    if(ui->excel_repeat_cbx->isChecked())
    {
        excel_settings.repeats = ui->excel_repeat_spin->value();
    }
    else
    {
        excel_settings.repeats = 0;
    }
    emit save_excel_settings(excel_settings);
}

void GUI_Setting::excel_start_test()
{
    emit start_excel_test();
}

void GUI_Setting::hide_excel_variants()
{
    if(ui->excel_all_tests->isChecked())
    {
        ui->excel_exclude_test_line->setEnabled(true);
        ui->excel_test_from_line->setEnabled(false);
        ui->excel_test_num_line->setEnabled(false);
        ui->excel_test_to_line->setEnabled(false);
    }
    else if(ui->excel_between_tests->isChecked())
    {
        ui->excel_exclude_test_line->setEnabled(true);
        ui->excel_test_from_line->setEnabled(true);
        ui->excel_test_num_line->setEnabled(false);
        ui->excel_test_to_line->setEnabled(true);
    }
    else if(ui->excel_one_test->isChecked())
    {
        ui->excel_exclude_test_line->setEnabled(false);
        ui->excel_test_from_line->setEnabled(false);
        ui->excel_test_num_line->setEnabled(true);
        ui->excel_test_to_line->setEnabled(false);
    }
}

void GUI_Setting::status_changed()
{
    QObject * obj = this->sender();
    QString objName = obj->objectName();
    char flag =0;
    if(objName==ui->info_msg_cbx->objectName())
    {
        flag = show_info;
        ui->info_msg_cbx->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    else if(objName==ui->warning_msg_cbx->objectName())
    {
        flag = show_warning;
        ui->warning_msg_cbx->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    else if(objName==ui->results_to_globvar->objectName())
    {
        flag = result_as_global;
        ui->results_to_globvar->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    else if(objName==ui->load_dll_from_start_cbx->objectName())
    {
        flag = load_dll_auto;
        ui->load_dll_from_start_cbx->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    else if(objName==ui->load_excel_from_start_cbx->objectName())
    {
        flag = load_excel_auto;
        ui->load_excel_from_start_cbx->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    else if(objName==ui->load_gpib_from_start_cbx->objectName())
    {
        flag = load_gpib_auto;
        ui->load_gpib_from_start_cbx->isChecked() ? autotest_flags|=flag : autotest_flags&=~flag;
    }
    emit main_status_changed(autotest_flags);
}
