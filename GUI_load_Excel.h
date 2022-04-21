#ifndef GUI_LOAD_EXCEL_H
#define GUI_LOAD_EXCEL_H


#include "ExcelApp.h"

namespace Ui {
class GUI_load_Excel;
}

class GUI_load_Excel : public QWidget
{
    Q_OBJECT

public:
    explicit GUI_load_Excel(QWidget *parent = 0);
    GUI_load_Excel(const QString & excAddr,
                   const QString & templName,
                   const QString & sheetName = QString(),
                   const QString & confDir = QString(),
                   const QString & scrDir = QString(),
                   char flag = 0, QWidget * parent = 0);

    void local_load(const QString & excAddr,
                    const QString & templName,
                    const QString & sheetName = QString(),
                    const QString & confDir = QString(),
                    const QString & scrDir = QString(),
                    char flag = 0);
    ~GUI_load_Excel();
private:
    Ui::GUI_load_Excel *ui;
    void Load_connections();
private slots:
    void catchEvent();
    void Load();
signals:
    void Load_excel(QString excAddr,
                    QString templName,
                    QString sheetName,
                    QString confDir,
                    QString scrDir,
                    char flag);
};

#endif // GUI_LOAD_EXCEL_H
