#include "GUI_load_window.h"
#include "ui_GUI_load_window.h"

GUI_load_lib::GUI_load_lib(QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::GUI_load_lib)
{
    ui->setupUi(this);
    main_layout=nullptr;
    cur_type=0;
    connect(ui->approp_select,SIGNAL(clicked(bool)),this,SLOT(load_button_pressed()));
}

void GUI_load_lib::__download_get_data(QMap<QString, QString> data, int type)
{
    cur_type = type;
    switch(type)
    {
    case LoadDLL:
    {
        load_dll(data);
    }
        break;
//    case LoadExcel:
//    {
//        load_excel(data);
//    }
        break;
    case LoadGPIB:
    {
        load_gpib(data);
    }
        break;
    }
}

void GUI_load_lib::load_dll(QMap<QString,QString> & data)
{

    QDir path(QDir::currentPath()+"/lib");
    QFileInfoList list = path.entryInfoList(QDir::Files);
    for(QFileInfoList::iterator it = list.begin();it!=list.end();it++)
    {
        QFileInfo current = *it;
        QString name = current.baseName();
        if(name=="basic_autotest_functions")
        {
            continue;
        }

        if(current.suffix()=="dll")
        {


            loading_data temp;
            temp.AbsolutePath = current.absoluteFilePath();
            temp.checkbox = new QCheckBox;
            if(!data.isEmpty())
            {
                QMap<QString,QString>::iterator it = data.find(name);
                if(it!=data.end())
                {
                    if(it.value()==temp.AbsolutePath)
                    {
                        temp.checkbox->setChecked(true);
                    }
                }
            }

            temp.label = new QLabel;
            temp.label->setText(name);
            temp.layout = new QHBoxLayout;
            temp.layout->addWidget(temp.label);
            temp.layout->addWidget(temp.checkbox);
            inner_data.insert(name,temp);
        }
    }
    if(!inner_data.isEmpty())
    {
        main_layout = new QVBoxLayout;
    }
    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
    {
        main_layout->addLayout(it.value().layout);
    }
    QLayout * stemp = ui->libraries_list->layout();
    delete stemp;
    ui->libraries_list->setLayout(main_layout);
}

void GUI_load_lib::load_gpib(QMap<QString,QString> & data)
{
    for(QMap<QString,QString>::iterator it = data.begin();it!=data.end();it++)
    {
            loading_data temp;
            temp.AbsolutePath = it.value();
            temp.checkbox = new QCheckBox;
            temp.label = new QLabel;
            temp.label->setText(it.key());
            temp.layout = new QHBoxLayout;
            temp.layout->addWidget(temp.label);
            temp.layout->addWidget(temp.checkbox);
            inner_data.insert(it.key(),temp);
    }
    if(!inner_data.isEmpty())
    {
        main_layout = new QVBoxLayout;
    }
    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
    {
        main_layout->addLayout(it.value().layout);
    }
    QLayout * stemp = ui->libraries_list->layout();
    delete stemp;
    ui->libraries_list->setLayout(main_layout);
}

//void GUI_load_lib::load_excel(QMap<QString,QString> & data)
//{
//
//    loading_data temp;
//    QString name,address;
//    if(!data.isEmpty())
//    {
//        auto it = data.begin();
//        name = it.key();
//        address = it.value();
//    }
//    else
//    {
//        name = "Default";
//        address = QDir::currentPath();
//    }

//    temp.AbsolutePath = address;
//    temp.checkbox = new QCheckBox;
//    temp.checkbox->setVisible(false);
//    temp.label = new QLabel;
//    temp.label->setText("Press button to select file (last file ["+name+"])");
//    temp.layout = new QHBoxLayout;
//    temp.layout->addWidget(temp.label);
//    temp.layout->addWidget(temp.checkbox);
//    inner_data.insert(name,temp);

//    if(!inner_data.isEmpty())
//    {
//        main_layout = new QVBoxLayout;
//    }

//    QLayout * stemp = ui->libraries_list->layout();
//    delete stemp;
//    ui->libraries_list->setLayout(main_layout);
//
//}

void GUI_load_lib::check_dll()
{
    QMap<QString,QString> senddata;
    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
    {
        if(it.value().checkbox->isChecked())
        {
            senddata.insert(it.key(),it.value().AbsolutePath);
        }
    }
    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
    {
       loading_data temp = it.value();
       delete temp.checkbox;
       delete temp.label;
       delete temp.layout;
    }
    inner_data.clear();
    if(main_layout)
    {
        delete main_layout;
        main_layout = nullptr;
    }
    emit __signal_adress_list(senddata,cur_type);
}

void GUI_load_lib::check_gpib()
{
    check_dll();
}

//void GUI_load_lib::check_excel()
//{
//
//    auto it = inner_data.begin();
//    QString address = QFileDialog::getOpenFileName(
//                this,("Open Excel"),it.value().AbsolutePath,
//                ("Excel files (*.xls *.xlsx)"),nullptr,QFileDialog::DontUseNativeDialog);

//    QMap<QString,QString> senddata;
//    QFileInfo current(address);
//    senddata.insert(current.fileName(),address);
//    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
//    {
//       loading_data temp = it.value();
//       delete temp.checkbox;
//       delete temp.label;
//       delete temp.layout;
//    }
//    inner_data.clear();
//    if(main_layout)
//    {
//        delete main_layout;
//        main_layout = nullptr;
//    }
//    emit __signal_adress_list(senddata,cur_type);
//
//    QMap<QString,QString> for_send;
//    for_send.insert("path","C:\\excel.xlsx");
//    for_send.insert("template","newlist");
//    emit __signal_adress_list(for_send,cur_type);
//}

void GUI_load_lib::load_button_pressed()
{
    switch(cur_type)
    {
    case LoadDLL:
    {
        check_dll();
    }
        break;
//    case LoadExcel:
//    {
//        check_excel();
//    }
        break;
    case LoadGPIB:
    {
        check_gpib();
    }
        break;
    }

    this->close();
}

GUI_load_lib::~GUI_load_lib()
{
    for(QMap<QString,loading_data>::iterator it = inner_data.begin(); it!=inner_data.end();it++)
    {
       loading_data temp = it.value();
       delete temp.checkbox;
       delete temp.label;
       delete temp.layout;
    }
    if(main_layout)
    {
        delete main_layout;
    }
    delete ui;
}
