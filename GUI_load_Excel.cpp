#include "GUI_load_Excel.h"
#include "ui_GUI_load_Excel.h"

GUI_load_Excel::GUI_load_Excel(QWidget *parent) :
    QWidget(parent,Qt::Window),
    ui(new Ui::GUI_load_Excel)
{
    ui->setupUi(this);
    ui->checkBox_template->setChecked(true);
    ui->checkBox_worksheet->setChecked(true);
    ui->comboBox_template->setEnabled(false);
    ui->comboBox_worksheet->setEnabled(false);
    Load_connections();
}

GUI_load_Excel::GUI_load_Excel(const QString & excAddr,
               const QString & templName,
               const QString & sheetName,
               const QString & confDir,
               const QString & scrDir,
               char flag, QWidget * parent):
    QWidget(parent,Qt::Window),
    ui(new Ui::GUI_load_Excel)
{
    ui->setupUi(this);
    local_load(excAddr,templName,sheetName,
               confDir,scrDir,flag);
    Load_connections();
}

void GUI_load_Excel::local_load(const QString &excAddr,
                                const QString &templName,
                                const QString &sheetName,
                                const QString &confDir,
                                const QString &scrDir,
                                char flag)
{
    ui->checkBox_template->setChecked(true);
    ui->checkBox_worksheet->setChecked(true);
    ui->comboBox_template->setEnabled(false);
    ui->comboBox_worksheet->setEnabled(false);

    ui->lineEdit_fileaddr->setText(excAddr);
    ui->lineEdit_template->setText(templName);
    ui->lineEdit_worksheet->setText(sheetName);
    ui->lineEdit_config->setText(confDir);
    ui->lineEdit_script->setText(scrDir);
    if(flag&ExcelApp::ExApp_WriteResultWhenBreak)
    {
        ui->checkBox_flagBR->setChecked(true);
    }
    if(flag&ExcelApp::ExApp_AutoClearSeq)
    {
        ui->checkBox_flagCLR->setChecked(true);
    }
    if(flag&ExcelApp::ExApp_QuickResult)
    {
        ui->checkBox_flagQR->setChecked(true);
    }
    if(flag&ExcelApp::ExApp_BreakAfterMiss)
    {
        ui->checkBox_breakaftermiss->setChecked(true);
    }
}

void GUI_load_Excel::Load_connections()
{
    connect(ui->pushButton_fileaddr,SIGNAL(clicked(bool)),this,SLOT(catchEvent()));
    connect(ui->pushButton_config,SIGNAL(clicked(bool)),this,SLOT(catchEvent()));
    connect(ui->pushButton_script,SIGNAL(clicked(bool)),this,SLOT(catchEvent()));
    connect(ui->checkBox_template,SIGNAL(clicked(bool)),this,SLOT(catchEvent()));
    connect(ui->checkBox_worksheet,SIGNAL(clicked(bool)),this,SLOT(catchEvent()));
    connect(ui->Load,SIGNAL(clicked(bool)),this,SLOT(Load()));
}

GUI_load_Excel::~GUI_load_Excel()
{
    delete ui;
}

void GUI_load_Excel::catchEvent()
{
    QObject * pObj = QObject::sender();
    if(pObj->objectName() == "pushButton_fileaddr")
    {
        QString name = QFileDialog::getOpenFileName(this, "Select Excel File",
                                                    QDir::currentPath(),
                                                    "Excel files (*.xls *.xlsx)");
        QFileInfo file(name);
        if((file.isFile())&&(file.isReadable()))
        {
            QAxObject * Excel = nullptr;
            QAxObject * Workbooks = nullptr;
            QAxObject * ActiveWorkBook = nullptr;
            QAxObject * Worksheets = nullptr;
            for(;;)
            {
                Excel = new QAxObject("Excel.Application",0);
                if(!Excel)
                {
                    ui->lineEdit_fileaddr->setText("!!!ERROR!!! Can't open excel app");
                    break;
                }

                Workbooks = Excel->querySubObject("Workbooks");
                if(!Workbooks)
                {
                    ui->lineEdit_fileaddr->setText("!!!ERROR!!! Can't load workbooks");
                    break;
                }
                ActiveWorkBook=Workbooks->querySubObject("Open (const QString&)", file.absoluteFilePath());
                if(!ActiveWorkBook)
                {
                    ui->lineEdit_fileaddr->setText("!!!ERROR!!! Can't load "+file.absoluteFilePath());
                    break;
                }
                Worksheets=ActiveWorkBook->querySubObject("Worksheets");
                if(!Worksheets)
                {
                    ui->lineEdit_fileaddr->setText("!!!ERROR!!! Can't load sheets from "+file.absoluteFilePath());
                    break;
                }

                QStringList data;
                int count = Worksheets->dynamicCall("Count()").toInt();
                for(int step = 1; step<=count;step++)
                {
                    QAxObject * temp = Worksheets->querySubObject("Item(int)",step);
                    data.append(temp->dynamicCall("Name").toString());
                    delete temp;
                }

                if(data.isEmpty())
                {
                     ui->lineEdit_fileaddr->setText("!!!ERROR!!! Selected Excel book is empty ");
                    break;
                }

                ui->comboBox_template->setEnabled(true);
                ui->comboBox_worksheet->setEnabled(true);
                ui->checkBox_template->setChecked(false);
                ui->checkBox_worksheet->setChecked(false);
                ui->lineEdit_template->setEnabled(false);
                ui->lineEdit_worksheet->setEnabled(false);
                ui->comboBox_template->addItems(data);
                ui->comboBox_worksheet->addItems(data);

                ui->lineEdit_fileaddr->setText(file.absoluteFilePath());
                break;
            }
            if(Worksheets)
            {
                delete Worksheets;
            }
            if(ActiveWorkBook)
            {
                //ActiveWorkBook->dynamicCall("Close()");
                delete ActiveWorkBook;
            }
            if(Workbooks)
            {
                delete Workbooks;
            }
            if(Excel)
            {
                Excel->dynamicCall("Quit()");
                delete Excel;
            }
        }
        else
        {
            ui->lineEdit_fileaddr->setText("!!!ERROR!!! can't be load ");
        }

    }
    else if(pObj->objectName() == "checkBox_template")
    {
        bool logic = ui->checkBox_template->isChecked();

        ui->comboBox_template->setEnabled(!logic);
        ui->lineEdit_template->setEnabled(logic);

    }
    else if(pObj->objectName() == "checkBox_worksheet")
    {
        bool logic = ui->checkBox_worksheet->isChecked();
        ui->comboBox_worksheet->setEnabled(!logic);
        ui->lineEdit_worksheet->setEnabled(logic);
    }
    else if(pObj->objectName()== "pushButton_script")
    {
        QString name = QFileDialog::getExistingDirectory(this,
                                                         "Select script directory",
                                                         QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly|
                                                         QFileDialog::DontResolveSymlinks);
        if(!name.isEmpty())
        {
            ui->lineEdit_script->setText(name);
        }
    }
    else if(pObj->objectName()=="pushButton_config")
    {
        QString name = QFileDialog::getExistingDirectory(this,
                                                         "Select config directory",
                                                         QDir::currentPath(),
                                                         QFileDialog::ShowDirsOnly|
                                                         QFileDialog::DontResolveSymlinks);
        if(!name.isEmpty())
        {
            ui->lineEdit_config->setText(name);
        }
    }
}

void GUI_load_Excel::Load()
{
    QString addr = ui->lineEdit_fileaddr->text();
    if(addr.contains("!ERROR")||(!QFile::exists(addr)))
    {
        ui->lineEdit_fileaddr->setStyleSheet("background: red");
        return;
    }
    else
    {
        ui->lineEdit_fileaddr->setStyleSheet("background: green");
    }

    QString templ;
    if(ui->checkBox_template->isChecked())
    {
        templ = ui->lineEdit_template->text();
        ui->lineEdit_template->setStyleSheet("background: green");
    }
    else
    {
        templ = ui->comboBox_template->currentText();
        ui->comboBox_template->setStyleSheet("background: green");
    }

    QString worksheet;
    if(ui->checkBox_worksheet->isChecked())
    {
        worksheet = ui->lineEdit_worksheet->text();
        ui->lineEdit_worksheet->setStyleSheet("background: green");
    }
    else
    {
        worksheet = ui->comboBox_worksheet->currentText();
        ui->comboBox_worksheet->setStyleSheet("background: green");
    }

    QString script = ui->lineEdit_script->text();
    if(!(script.isEmpty()))
    {
        QDir dir(script);
        if(dir.exists())
        {
            ui->lineEdit_script->setStyleSheet("background: green");
        }
        else
        {
            ui->lineEdit_script->setStyleSheet("background: red");
            return;
        }
    }
    QString conf = ui->lineEdit_config->text();
    if(!(conf.isEmpty()))
    {
        QDir dir(conf);
        if(dir.exists())
        {
            ui->lineEdit_config->setStyleSheet("background: green");
        }
        else
        {
            ui->lineEdit_config->setStyleSheet("background: red");
            return;
        }
    }
    char flags = 0;
    if(ui->checkBox_flagBR->isChecked())
    {
        flags|=ExcelApp::ExApp_WriteResultWhenBreak;
    }
    if(ui->checkBox_flagCLR->isChecked())
    {
        flags|=ExcelApp::ExApp_AutoClearSeq;
    }
    if(ui->checkBox_flagQR->isChecked())
    {
        flags|=ExcelApp::ExApp_QuickResult;
    }
    if(ui->checkBox_noload_excwndw->isChecked())
    {
        flags|=ExcelApp::ExApp_NotShowExcWndw;
    }
    if(ui->checkBox_breakaftermiss->isChecked())
    {
        flags|=ExcelApp::ExApp_BreakAfterMiss;
    }
    emit Load_excel(addr,templ,worksheet,conf,script,flags);
    this->close();
}
