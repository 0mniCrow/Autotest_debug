#ifndef GUI_LOAD_LIB_H
#define GUI_LOAD_LIB_H

#include "GUI_load_Excel.h"


namespace Ui {
class GUI_load_lib;
}

enum loading_type
{
    LoadNil = 0,
   // LoadExcel = 1,
    LoadGPIB = 2,
    LoadDLL = 3
};

struct base_data
{
    QHBoxLayout * layout;
    QLabel * label;
};

struct loading_data:public base_data
{
    QString AbsolutePath;
    QCheckBox * checkbox;
};

struct excel_data:public base_data
{
    QPushButton * select_file;

};

class GUI_load_lib : public QWidget
{
    Q_OBJECT

public:
    explicit GUI_load_lib(QWidget *parent = 0);
    ~GUI_load_lib();

signals:
    void __signal_adress_list(QMap<QString,QString> data, int type);

public slots:
    void __download_get_data(QMap<QString,QString> data, int type);

private slots:
    void load_button_pressed();

private:
    Ui::GUI_load_lib *ui;
    QVBoxLayout * main_layout;
    QMap<QString,loading_data> inner_data;
    int cur_type;
    void load_dll(QMap<QString,QString> & data);
    void load_gpib(QMap<QString,QString> & data);
//    void load_excel(QMap<QString,QString> &data);

    void check_dll();
    void check_gpib();
//    void check_excel();
};

#endif // GUI_LOAD_LIB_H
