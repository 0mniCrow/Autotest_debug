#include "GUI_Interface.h"
#include "ui_GUI_Interface.h"

GUI_Interface::GUI_Interface(const QStringList &keys, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GUI_Interface)
{
    ui->setupUi(this);
    QString path = QDir::currentPath()+"/configs";
    additionalSettings = new QSettings(path+"/config.ini",QSettings::IniFormat);
    excelAutomat= new ExcelApp(this);

    setting_window = new GUI_Setting(this);
    load_window = new GUI_load_lib(this);
    load_excel = new GUI_load_Excel(this);
    ready = false;
    ui->setupUi(this);
    ui->start_or_continue->setEnabled(ready);
    ui->textedit_Excel_log->setHtml("<h1 style=\"color:powderblue;\">Excel log start</h1>");
    ui->ScriptJournalTextEdit->setHtml("<h1 style=\"color:powderblue;\">Script log start</h1>");
    GUI_autotest_connections();

    GUI_setting_window_connections();

    //connect(ui->File_select,SIGNAL(clicked(bool)),this,SLOT(GUI__slot_load_script_file()));
    //connect(ui->remove_scripts,SIGNAL(clicked(bool)),this,SLOT(on_remove_scripts_clicked(bool)));
    //connect(ui->dll_load,SIGNAL(clicked(bool)),this,SLOT(GUI__slot_load_dll_manualy()));

    //Кнопка инициализации строковых команд
    connect(ui->initiate_command,SIGNAL(clicked(bool)),this,SLOT(GUI__slot_initiate_command()));
    //Окно настроек автотеста
    connect(ui->setting_btn,SIGNAL(clicked(bool)),this,SLOT(GUI__slot_open_settings_window()));

    connect(load_excel,SIGNAL(Load_excel(QString,QString,QString,QString,QString,char)),
            this,SLOT(GUI__slot_load_Excel_object(QString,QString,QString,QString,QString,char)));

    connect(this,SIGNAL(destroyed(QObject*)),load_excel,SLOT(deleteLater()));

    GUI_config_connections();

    GUI_excel_connections();

    GUI_cross_connections();

    interpritator.AUTOTEST_CONNECT_CodeVIEW(ui->code_view);
    // interpritator.AUTOTEST_CONNECT_LogVIEW(ui->text_journal);
    interpritator.AUTOTEST_CONNECT_VariablesVIEW(ui->variable_view);
    QStringList headers{"Identificator","Value"};
    SettingModel * model = new SettingModel(headers,additionalSettings);
    ui->SettingsTree->setModel(model);
    GUI_load_settings(keys);


}

void GUI_Interface::GUI_autotest_connections()
{
    //Сообщение от автотеста - завершение исполнения
    connect(&interpritator,SIGNAL(AUTOTEST_SCRIPT_END(QVariant)),this,SLOT(GUI__slot_catch_return_value(QVariant)));
    //Сообщение от автотеста - следующая итерация пошагового исполнения готова
    connect(&interpritator,SIGNAL(AUTOTEST_SCRIPT_READY()),this,SLOT(GUI__slot_ready_to_engage()));

    //Сообщение автотесту - запустить следующую итерацию
    connect(ui->start_or_continue,SIGNAL(clicked(bool)),&interpritator,SLOT(AUTOTEST_Start_test()));
    //Сообщение автотесту - прервать тест
    connect(ui->break_test,SIGNAL(clicked(bool)),&interpritator,SLOT(AUTOTEST_Break_Sequance()));
    //Сообщение автотесту - загрузить скрипт с командами
    connect(this,SIGNAL(GUI__signal_start_test(QString,QString,QString,int)),&interpritator,SLOT(AUTOTEST_Load_Test(QString,QString,QString,int)));
    //Сообщение автотесту - изменение состояния флага пошагового исполнения
    connect(ui->step_by_step_flag,SIGNAL(clicked(bool)),&interpritator,SLOT(AUTOTEST_By_Step_mode(bool)));
    connect(&interpritator,SIGNAL(AUTOTEST_MESSGES(QString,int)),this,SLOT(GUI__slot_catch_script(QString,int)));
}

void GUI_Interface::GUI_setting_window_connections()
{
    //Сообщение из окна настроек автотесту - показывать предупреждающие сообщения
    connect(setting_window,SIGNAL(show_warning_messages(bool)),&interpritator,SLOT(AUTOTEST_Show_warning_msg(bool)));
    //Сообщение из окна настроек автотесту - показывать информационные сообщения
    connect(setting_window,SIGNAL(show_info_messages(bool)),&interpritator,SLOT(AUTOTEST_Show_info_msg(bool)));

    //Сообщение из окна настроек - ручаня загрузка библиотеки функций
    connect(setting_window,SIGNAL(load_dll_manual_btn()),this,SLOT(GUI__slot_load_dll_manualy()));
    //Сообщение из окна настроек - загрузить библиотеки из файла конфигураций
    connect(setting_window,SIGNAL(load_dll_from_config_btn()),this,SLOT(GUI__slot_load_dll_from_config()));
    //Сообщение из окна настроек  - ручная загрузка переменных из файла конфигураций
    connect(setting_window,SIGNAL(load_gpib_manual_btn()),this,SLOT(GUI__slot_load_gpib_manualy()));
    //Сообщение из окна настроек - загрузить все переменные из файла конфигураций
    connect(setting_window,SIGNAL(load_gpib_from_config_btn()),this,SLOT(GUI__slot_load_gpib_from_config()));
    //Сообщение из окна настроек - ручная загрузка файла Эксель
    connect(setting_window,SIGNAL(load_excel_manual_btn()),this,SLOT(GUI__slot_load_excel_manualy()));
    //Сообщение из окна настроек - загрузить файл Эксель из файла конфигураций
    connect(setting_window,SIGNAL(load_excel_from_config_btn()),this,SLOT(GUI__slot_load_excel_from_config()));

    //Сообщение из окна настроек автотесту - сохранять результаты тестов как глобальные переменные
    connect(setting_window,SIGNAL(results_to_global_variables(bool)),&interpritator,SLOT(AUTOTEST_Load_ret_val_as_global(bool)));
    //Сообщение из окна настроек автотесту - удалить указанный сохранённый скрипт из памяти автотеста
    connect(setting_window,SIGNAL(delete_loaded_test(QString)),&interpritator,SLOT(AUTOTEST_Remove_script(QString)));

    //Сообщение из окна настроек - ручная загрузка файла теста
    connect(setting_window,SIGNAL(test_file_select()),this,SLOT(GUI__slot_load_script_file()));
    //Сообщение из окна настроек - обновить данные по настройкам тестирования Эксель
    connect(setting_window,SIGNAL(save_excel_settings(setting_excel)),this,SLOT(GUI__slot_save_excel_settings(setting_excel)));
    //Сообщение из окна настроек - запустить тестирование по файлу Эксель
    connect(setting_window,SIGNAL(start_excel_test()),this,SLOT(GUI__slot_start_excel_test()));
    //Сообщение из окна настроек - изменение состояния настроек автотеста
    connect(setting_window,SIGNAL(main_status_changed(char)),this,SLOT(GUI__slot_save_flags(char)));
    connect(setting_window,SIGNAL(clear_excel_seq()),excelAutomat,SLOT(Excel_slot_clear_test_seq()));

    //!
    connect(this,SIGNAL(destroyed(QObject*)),setting_window,SLOT(deleteLater()));
}

void GUI_Interface::GUI_config_connections()
{
    //Работа с файлом конфигураций - добавить элемент/вставить элемент в выделенную область
    connect(ui->InsertItemBtn,SIGNAL(clicked(bool)),this,SLOT(on_InsertItemBtn_clicked(bool)));
    //Работа с файлом конфигураций - удалить элемент/группу элементов
    connect(ui->DeleteItemBtn,SIGNAL(clicked(bool)),this,SLOT(on_DeleteItemBtn_clicked(bool)));
    //Работа с файлом конфигураций - загрузить выделенный элемент как глобальную переменную
    connect(ui->load_variables,SIGNAL(clicked(bool)),this,SLOT(on_load_variables_clicked(bool)));
    //Сообщение из окна загрузок - загрузка библиотек избранных пользователем
    connect(load_window,SIGNAL(__signal_adress_list(QMap<QString,QString>,int)),this,SLOT(GUI__slot_load_info(QMap<QString,QString>,int)));
    //!
    connect(this,SIGNAL(destroyed(QObject*)),load_window,SLOT(deleteLater()));
}

void GUI_Interface::GUI_excel_connections()
{

    connect(this,SIGNAL(GUI__command_append_pause(int)),excelAutomat,SLOT(Excel_slot_append_debug_pause(int)));
    connect(this,SIGNAL(GUI__command_append_seqence(int,int)),excelAutomat,SLOT(Excel_slot_append_test_seq_range(int,int)));
    connect(this,SIGNAL(GUI__command_append_seqence()),excelAutomat,SLOT(Excel_slot_append_test_seq_range()));
    connect(this,SIGNAL(GUI__command_load_excelApp(QString,QString,QString,QString,QString,char)),
            excelAutomat,SLOT(Excel_reupload_data(QString,QString,QString,QString,QString,char)));
    connect(this,SIGNAL(GUI__command_load_sequances(QList<QPair<int,int> >*,QList<QPair<int,int> >*,QList<int>*,int)),
            excelAutomat,SLOT(Excel_reupload_seq(QList<QPair<int,int> >*,QList<QPair<int,int> >*,QList<int>*,int)));
    connect(this,SIGNAL(GUI__command_remove_pause(int)),excelAutomat,SLOT(Excel_slot_delete_debug_pause(int)));
    connect(this,SIGNAL(GUI__command_remove_seqence(int,int)),excelAutomat,SLOT(Excel_slot_remove_test_seq_range(int,int)));
    connect(this,SIGNAL(GUI__command_remove_seqences()),excelAutomat,SLOT(Excel_slot_clear_test_seq()));
    connect(this,SIGNAL(GUI__command_start()),excelAutomat,SLOT(Excel_slot_start_tests()));
    connect(this,SIGNAL(GUI__command_remove_flags(char)),excelAutomat,SLOT(Excel_slot_remove_flags(char)));
    connect(this,SIGNAL(GUI__command_set_flags(char)),excelAutomat,SLOT(Excel_slot_set_flags(char)));
    connect(excelAutomat,SIGNAL(Excel_signal_message(QString,int)),this,SLOT(GUI__slot_catch_message(QString,int)));
    connect(ui->break_test,SIGNAL(clicked(bool)),excelAutomat,SLOT(Excel_slot_break_tests()));

    connect(&interpritator,SIGNAL(AUTOTEST_SCRIPT_END(QVariant)),excelAutomat,SLOT(Excel_slot_test_complete(QVariant)));
    connect(excelAutomat,SIGNAL(Excel_signal_next(QString,QString,QString,int)),&interpritator,SLOT(AUTOTEST_Load_Test(QString,QString,QString,int)));
    connect(excelAutomat,SIGNAL(Excel_signal_setPause(bool)),&interpritator,SLOT(AUTOTEST_By_Step_mode(bool)));
    connect(excelAutomat,SIGNAL(Excel_signal_setPause(bool)),ui->step_by_step_flag,SLOT(setChecked(bool)));
    connect(excelAutomat,SIGNAL(Excel_signal_state(char)),this,SLOT(GUI__slot_get_update_status(char)));

    connect(this,SIGNAL(destroyed(QObject*)),excelAutomat,SLOT(deleteLater()));
}

void GUI_Interface::GUI_cross_connections()
{

}

GUI_Interface::~GUI_Interface()
{
    delete ui;
    delete additionalSettings;
}

void GUI_Interface::GUI_load_DLL(QMap<QString,QString> & data)
{
    if(!data.isEmpty())
    {
        if(additionalSettings)
        {
            additionalSettings->beginGroup("DLL");
            additionalSettings->remove("");
            additionalSettings->endGroup();
            additionalSettings->beginGroup("DLL");
        }

        for(QMap<QString,QString>::iterator it = data.begin();it!=data.end();it++)
        {
            if(additionalSettings)
            {
                additionalSettings->setValue(it.key(),it.value());
            }
            interpritator.AUTOTEST_Load_DLL(it.value());
        }
        if(additionalSettings)
        {
            additionalSettings->endGroup();
        }
    }
}

void GUI_Interface::GUI_load_Variables(QMap<QString,QString> & data)
{

        QMap<QString,QVariant> for_send;
        for(QMap<QString,QString>::iterator it = data.begin();it!=data.end();it++)
        {
            QVariant value(it.value());
            for_send.insert(it.key(),value);
        }
        if(!for_send.isEmpty())
        {
            interpritator.AUTOTEST_Load_Variables(for_send);
        }
}

void GUI_Interface::GUI__slot_load_Excel_object(const QString & excAddr,
                           const QString & templName,
                           const QString & sheetName,
                           const QString & confDir,
                           const QString & scrDir,
                           char flag)
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("EXCEL");
        additionalSettings->setValue("address",excAddr);
        additionalSettings->setValue("template",templName);
        additionalSettings->setValue("sheet",sheetName);
        additionalSettings->setValue("script",scrDir);
        additionalSettings->setValue("config",confDir);
        additionalSettings->setValue("flags",static_cast<int>(flag));
        additionalSettings->endGroup();
        additionalSettings->beginGroup("MAIN");
        if(additionalSettings->value("SHOW_WARNING").toBool())
        {
            flag|=ExcelApp::ExApp_ShowWarnMsg;
        }
        if(additionalSettings->value("SHOW_INFO").toBool())
        {
            flag|=ExcelApp::ExApp_ShowInfoMsg;
        }
        additionalSettings->endGroup();
    }
    emit GUI__command_load_excelApp(excAddr,templName,sheetName,
                                    confDir, scrDir, flag);

}


//Catch signal from a script manager informs that script is finished
void GUI_Interface::GUI__slot_catch_return_value(QVariant value)
{
    ui->command_line->setText(value.toString());
}

//Catch ignal from a script manager ingorms that script is ready
//for a next operation
void GUI_Interface::GUI__slot_ready_to_engage()
{
    ready = true;
    ui->start_or_continue->setEnabled(ready);
}

//Manual loading of the script from file system. An additional data
//not supported
void GUI_Interface::GUI__slot_load_script_file()
{
    QString address = QFileDialog::getOpenFileName(this,("Open Script"),
                                                   QDir::currentPath()+"/tests",
                                                   ("Text files (*.nts *.ntf *.txt)"),
                                                   nullptr,
                                                   QFileDialog::DontUseNativeDialog);
    ui->File_address->setText(address);
    if(!address.isEmpty())
    {
        QString term("");
        emit GUI__signal_start_test(address,term,term,0);
    }
}

//Slot catch signal from "next" button
void GUI_Interface::on_start_or_continue_clicked(bool checked)
{
    Q_UNUSED(checked)
    ready=false;
    ui->start_or_continue->setEnabled(ready);
}

//Config interface - allows to insert a item in the config file
void GUI_Interface::on_InsertItemBtn_clicked(bool checked)
{
    Q_UNUSED(checked)
    QAbstractItemModel * model = ui->SettingsTree->model();
    if(!model)
        return;
    const QModelIndex index = ui->SettingsTree->selectionModel()->currentIndex();
    if(!model->insertRow(0,index))
        return;
    ui->SettingsTree->selectionModel()->setCurrentIndex(model->index(0,0,index),QItemSelectionModel::ClearAndSelect);
}

//Config interface - allows to delete a item from the config file
void GUI_Interface::on_DeleteItemBtn_clicked(bool checked)
{
    Q_UNUSED(checked)
    QAbstractItemModel * model = ui->SettingsTree->model();
    if(!model)
        return;
    const QModelIndex index = ui->SettingsTree->selectionModel()->currentIndex();
    model->removeRow(index.row(),index.parent());
}

//Config interface - allows to load record from config file as global variable
void GUI_Interface::on_load_variables_clicked(bool checked)
{
    Q_UNUSED(checked)
    SettingModel * model = static_cast<SettingModel*>(ui->SettingsTree->model());
    if(!model)
        return;
    const QModelIndex index = ui->SettingsTree->selectionModel()->currentIndex();
    QMap<QString,QVariant> vardata = model->getvariablemap(index);
    for(QMap<QString,QVariant>::iterator step = vardata.begin();step!=vardata.end();step++)
    {
        interpritator.AUTOTEST_Append_Variable(step.key(),step.value());
    }
    return;
}

//Command line (can be used to write commands)
void GUI_Interface::GUI__slot_initiate_command()
{

}

void GUI_Interface::GUI__slot_open_settings_window()
{
    setting_window->show();
}

//Slot catch data from load form to transfer it between autotester and config file
void GUI_Interface::GUI__slot_load_info(QMap<QString,QString> list, int data_type)
{
    switch(data_type)
    {
    case LoadDLL:
    {
        GUI_load_DLL(list);
    }
        break;
    case LoadGPIB:
    {
        GUI_load_Variables(list);
    }
        break;
//    case LoadExcel:
//    {
//        GUI_load_Excel(list);
//    }
//        break;
    default:
        break;
    }
    load_window->hide();
}

void GUI_Interface::GUI__slot_load_dll_from_config()
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("DLL");
        QStringList data = additionalSettings->allKeys();
        for(QStringList::iterator it = data.begin();it!=data.end();it++)
        {
            interpritator.AUTOTEST_Load_DLL(additionalSettings->value(*it).toString());
        }
        additionalSettings->endGroup();
    }
}

void GUI_Interface::GUI__slot_load_dll_manualy()
{
    QMap<QString,QString> for_send;
    if(additionalSettings)
    {
        additionalSettings->beginGroup("DLL");
        QStringList data = additionalSettings->allKeys();
        for(QStringList::iterator it = data.begin();it!=data.end();it++)
        {
            for_send.insert(*it,additionalSettings->value(*it).toString());
        }
        additionalSettings->endGroup();
    }
    load_window->__download_get_data(for_send,LoadDLL);
    load_window->show();
}

void GUI_Interface::GUI__slot_load_gpib_manualy()
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("GPIB");
        QStringList data = additionalSettings->allKeys();
        QMap<QString,QString> for_send;
        for(QStringList::iterator it = data.begin();it!=data.end();it++)
        {
            for_send.insert(*it, additionalSettings->value(*it).toString());
        }

        additionalSettings->endGroup();

        if(!for_send.isEmpty())
        {
            load_window->__download_get_data(for_send,LoadGPIB);
        }
        load_window->show();
    }
}

void GUI_Interface::GUI__slot_load_gpib_from_config()
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("GPIB");
        QStringList data = additionalSettings->allKeys();
        QMap<QString,QVariant> for_send;
        for(QStringList::iterator it = data.begin();it!=data.end();it++)
        {
            for_send.insert(*it, additionalSettings->value(*it).toString());
        }

        additionalSettings->endGroup();

        if(!for_send.isEmpty())
        {
            interpritator.AUTOTEST_Load_Variables(for_send);
        }
    }
}

void GUI_Interface::GUI__slot_load_excel_from_config()
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("EXCEL");
        QString address = additionalSettings->value("address").toString();
        QString templ = additionalSettings->value("template").toString();
        QString sheet = additionalSettings->value("sheet").toString();
        QString script = additionalSettings->value("script").toString();
        QString config = additionalSettings->value("config").toString();
        char flags = static_cast<char>(additionalSettings->value("flags").toInt());
        additionalSettings->endGroup();
        additionalSettings->beginGroup("MAIN");
        if(additionalSettings->value("SHOW_WARNING").toBool())
        {
            flags|=ExcelApp::ExApp_ShowWarnMsg;
        }
        if(additionalSettings->value("SHOW_INFO").toBool())
        {
            flags|=ExcelApp::ExApp_ShowInfoMsg;
        }
        additionalSettings->endGroup();
        flags|=ExcelApp::ExApp_QuickStart;
        emit GUI__command_load_excelApp(address,templ,sheet,
                                        config, script, flags);
    }
}

void GUI_Interface::GUI__slot_load_excel_manualy()
{
    if(load_excel)
    {
        if(additionalSettings)
        {
            additionalSettings->beginGroup("EXCEL");
            QString address = additionalSettings->value("address").toString();
            QString templ = additionalSettings->value("template").toString();
            QString sheet = additionalSettings->value("sheet").toString();
            QString script = additionalSettings->value("script").toString();
            QString config = additionalSettings->value("config").toString();
            char flags = static_cast<char>(additionalSettings->value("flags").toInt());
            additionalSettings->endGroup();

            load_excel->local_load(address,templ,sheet,config,script,flags);
            //load_excel->setAttribute(Qt::WA_DeleteOnClose,true);

        }

        load_excel->show();
    }
}

void GUI_Interface::GUI__slot_save_excel_settings(setting_excel excel_dat)
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("EXCEL");

        switch(excel_dat.state)
        {
        case excel_all_test:
        {
            additionalSettings->setValue("state","ALL");
            additionalSettings->setValue("excluded",excel_dat.exepted_tests);
        }
            break;
        case excel_range_test:
        {
            additionalSettings->setValue("state","RANGE");
            additionalSettings->setValue("from",excel_dat.from_num);
            additionalSettings->setValue("to",excel_dat.for_num);
            additionalSettings->setValue("excluded",excel_dat.exepted_tests);
        }
            break;
        case excel_one_test:
        {
            additionalSettings->setValue("state","ONE");
            additionalSettings->setValue("num",excel_dat.from_num);
        }
            break;
        }

        additionalSettings->setValue("repeats",excel_dat.repeats);
        additionalSettings->setValue("pauses",excel_dat.paused_tests);
        additionalSettings->endGroup();
    }
    GUI__command_remove_flags(ExcelApp::ExApp_QuickStart);
    GUI__slot_load_Excel_seq();
}

void GUI_Interface::GUI__slot_load_Excel_seq()
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("EXCEL");
        QString state = additionalSettings->value("state").toString();
        int start_num = 0;
        int end_num=0;
        if(state=="ALL")
        {
            start_num=end_num=-1;
        }
        else if(state=="RANGE")
        {
            start_num = additionalSettings->value("from").toInt();
            end_num = additionalSettings->value("to").toInt();
        }
        else if(state=="ONE")
        {
            start_num = end_num = additionalSettings->value("num").toInt();
        }
        int repeats = additionalSettings->value("repeats").toInt();
        QStringList excluded = additionalSettings->value("excluded").toString().split(' ',QString::SkipEmptyParts);
        QStringList pauses = additionalSettings->value("pauses").toString().split(' ',QString::SkipEmptyParts);
        additionalSettings->endGroup();
        QList<QPair<int,int>> * list_sequances = new QList<QPair<int,int>>();
        QPair<int,int> tempp(start_num,end_num);
        list_sequances->append(tempp);
        QList<QPair<int,int>> * list_excluded = new QList<QPair<int,int>>();
        for(QStringList::iterator it = excluded.begin(); it!=excluded.end();it++)
        {
            QString current = *it;
            if(current.contains("-"))
            {
                QStringList temp = current.split('-',QString::SkipEmptyParts);
                QPair<int,int> ftemp(temp.at(0).toInt(),temp.at(1).toInt());
                list_excluded->append(ftemp);
            }
            else
            {
                int val = current.toInt();
                QPair<int,int> ftemp(val,val);
                list_excluded->append(ftemp);
            }
        }
        QList<int> * list_pauses = new QList<int>();
        for(QStringList::iterator it = pauses.begin(); it!=pauses.end();it++)
        {
            list_pauses->append((*it).toInt());
        }

        emit GUI__command_load_sequances(list_sequances,list_excluded,list_pauses,repeats);
    }
}

void GUI_Interface::GUI__slot_get_update_status(char stat)
{
    if(stat==ExcelApp::EF_DataLoaded)
    {
        GUI__slot_load_Excel_seq();
    }
    else if(stat == ExcelApp::EF_SeqLoaded)
    {
        GUI__slot_start_excel_test();
    }
}

QString GUI_Interface::GUI_message_colorize(const QString & msg, int type)
{
    QString text("<p style=\"background-color:white; color:");
    switch(type)
    {
    case scr_INFO:
    {
        text+="blue";
    }
        break;
    case scr_WARNING:
    {
        text+="orange";
    }
        break;
    case scr_ERROR:
    {
        text+="red";
    }
        break;
    case scr_MESSAGE:
    {
        text+="green";
    }
        break;
    }
    text+=";\">"+msg+"</p>";
    return text;
}

void GUI_Interface::GUI__slot_catch_message(QString message, int type)
{
    ui->textedit_Excel_log->append(GUI_message_colorize(message,type));
    ui->textedit_Excel_log->verticalScrollBar()->setValue(ui->textedit_Excel_log->verticalScrollBar()->maximum());
    if(ui->DialogOutput->currentIndex()!=EXCEL_LOG_WINDOW)
    {
        ui->DialogOutput->setCurrentIndex(EXCEL_LOG_WINDOW);
    }
}

void GUI_Interface::GUI__slot_catch_script(QString message, int type)
{
    ui->ScriptJournalTextEdit->append(GUI_message_colorize(message,type));
    ui->ScriptJournalTextEdit->verticalScrollBar()->setValue(ui->ScriptJournalTextEdit->verticalScrollBar()->maximum());
    if(ui->DialogOutput->currentIndex()!=SCRIPT_LOG_WINDOW)
    {
        ui->DialogOutput->setCurrentIndex(SCRIPT_LOG_WINDOW);
    }
}

void GUI_Interface::GUI__slot_infomessage(bool logic)
{
    QObject * ptr = sender();
    if(ptr->objectName()=="info_msg_cbx")
    {
        if(logic)
        {
            emit GUI__command_set_flags(ExcelApp::ExApp_ShowInfoMsg);
        }
        else
        {
            emit GUI__command_remove_flags(ExcelApp::ExApp_ShowInfoMsg);
        }
    }
    else if(ptr->objectName()=="warning_msg_cbx")
    {
        if(logic)
        {
            emit GUI__command_set_flags(ExcelApp::ExApp_ShowWarnMsg);
        }
        else
        {
            emit GUI__command_remove_flags(ExcelApp::ExApp_ShowWarnMsg);
        }
    }
}

void GUI_Interface::GUI__slot_load_excel_from_settings()
{
    if(additionalSettings)
    {
        setting_excel excel_dat;
        excel_dat.exepted_tests.clear();
        excel_dat.for_num=0;
        excel_dat.from_num=0;
        excel_dat.repeats=0;

        additionalSettings->beginGroup("EXCEL");
        QString state = additionalSettings->value("state").toString();
        excel_dat.exepted_tests = additionalSettings->value("excluded").toString();
        excel_dat.paused_tests = additionalSettings->value("pauses").toString();
        excel_dat.repeats = additionalSettings->value("repeats").toInt();
        if(!state.isEmpty())
        {
            if(state == "ALL")
            {
                excel_dat.state = excel_all_test;

            }
            else if(state == "RANGE")
            {
                excel_dat.state = excel_range_test;
                excel_dat.from_num = additionalSettings->value("from").toInt();
                excel_dat.for_num = additionalSettings->value("to").toInt();

            }
            else if(state == "ONE")
            {
                excel_dat.state = excel_one_test;
                excel_dat.from_num = additionalSettings->value("num").toInt();
                excel_dat.for_num=excel_dat.from_num;
            }
        }
        additionalSettings->endGroup();
        setting_window->load_excel_settings(excel_dat);
    }
}

void GUI_Interface::GUI__slot_start_excel_test()
{
    GUI__command_start();
}

void GUI_Interface::GUI__slot_save_flags(char flag)
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup("MAIN");

        additionalSettings->setValue("AUTOLOAD_DLL",static_cast<bool>(flag&load_dll_auto));
        additionalSettings->setValue("AUTOLOAD_GPIB",static_cast<bool>(flag&load_gpib_auto));
        additionalSettings->setValue("AUTOLOAD_EXCEL",static_cast<bool>(flag&load_excel_auto));
        additionalSettings->setValue("SHOW_WARNING",static_cast<bool>(flag&show_warning));
        additionalSettings->setValue("SHOW_INFO",static_cast<bool>(flag&show_info));
        additionalSettings->setValue("SAVE_RESULTS",static_cast<bool>(flag&result_as_global));

        additionalSettings->endGroup();
    }
}

void GUI_Interface::GUI__slot_load_flags()
{
    QMap<QString,QVariant> slot_map;


    if(additionalSettings)
    {
        char autotest_flags = 0;
        additionalSettings->beginGroup("MAIN");
        if(additionalSettings->value("SHOW_INFO").toBool())
        {
            autotest_flags|=show_info;
            interpritator.AUTOTEST_Show_info_msg(true);
        }
        if(additionalSettings->value("SHOW_WARNING").toBool())
        {
            autotest_flags|=show_warning;
            interpritator.AUTOTEST_Show_warning_msg(true);
        }
        if(additionalSettings->value("SAVE_RESULTS").toBool())
        {
            autotest_flags|=result_as_global;
            interpritator.AUTOTEST_Load_ret_val_as_global(true);
        }
        if(additionalSettings->value("AUTOLOAD_DLL").toBool())
        {
            autotest_flags|=load_dll_auto;
        }
        if(additionalSettings->value("AUTOLOAD_EXCEL").toBool())
        {
            autotest_flags|=load_excel_auto;
        }
        if(additionalSettings->value("AUTOLOAD_GPIB").toBool())
        {
            autotest_flags|=load_gpib_auto;
        }
        additionalSettings->endGroup();
        if(autotest_flags&load_dll_auto)
        {
            GUI__slot_load_dll_from_config();
        }
        if(autotest_flags&load_gpib_auto)
        {
            GUI__slot_load_gpib_from_config();
        }
        if(autotest_flags&load_excel_auto)
        {
            GUI__slot_load_excel_from_config();
        }
        setting_window->load_flags(autotest_flags);
    }

}

void GUI_Interface::GUI_load_settings(const QStringList & keys)
{
    if(keys.isEmpty())
    {
        return;
    }
    enum localkeys
    {
        flExaddress = 0x01,
        flExtemplate = 0x02,
        flExlist = 0x04,
        flSqload = 0x08,
        flprelEx = 0x10,
        flprelSeq = 0x20,

    };
    char flags = 0;



    if(keys.size()>1)
    {

    }
    else
    {
        GUI__slot_load_flags();
        GUI__slot_load_excel_from_settings();
    }
}

void GUI_Interface::GUI_wr_sect(const QString & section, const QMap<QString,QVariant> & container)
{
    QMap<QString,QVariant>::const_iterator it = container.begin();
    while(it!=container.end())
    {
        GUI_wr_conf(section,it.key(),it.value());
        it++;
    }
}

void GUI_Interface::GUI_rd_sect(const QString & section, QMap<QString,QVariant> & container)
{
    QMap<QString,QVariant>::iterator it = container.begin();
    while(it!=container.end())
    {
        it.value()= GUI_rd_conf(section,it.key());
        it++;
    }
}

void GUI_Interface::GUI_wr_conf(const QString & section, const QString & name, const QVariant & value)
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup(section);
        additionalSettings->setValue(name,value);
        additionalSettings->endGroup();
    }
}

QVariant GUI_Interface::GUI_rd_conf(const QString & section, const QString & name)
{
    QVariant answer;
    if(additionalSettings)
    {
        additionalSettings->beginGroup(section);
        answer=additionalSettings->value(name);
        additionalSettings->endGroup();
    }
    return answer;
}

void GUI_Interface::GUI_clr_conf(const QString & section, const QString &name)
{
    if(additionalSettings)
    {
        additionalSettings->beginGroup(section);
        additionalSettings->remove(name);
        additionalSettings->endGroup();
    }
}
