#ifndef PREDEFINES_H
#define PREDEFINES_H

#define _sERROR_VARIABLE_INITIALISATION 3                      //                       A problem with a variable in the constructor
#define _sERROR_VARIABLE_MANIPULATION 4                        //                                  A problem with using the variable
#define _sERROR_FUNCTION_MANIPULATION 5                        //                        A problem with using the callback functions
#define _sERROR_INPUT_DATA 6                                   //                                     A problem with the coming data
#define _sERROR_PARSING_DATA 7                                 //                A problem with parsing the input data into a tokens
#define _sERROR_TYPE_ANALYSIS 8                                //                            A problem with the tokens type analysis
#define _sERROR_RAILSTATION_SORT 9                             //  A problem with the rail station sorting in the algorithmic blocks
#define _sERROR_CODE_EXECUTION 10                              //                        A problem with execution of the script code
#define _sERROR_BLOCK_LOGIC 11                                 //        A problem with the incorrect using of the algorithmic block

#define _int_ERROR      -1
#define scr_ERROR       2
#define scr_WARNING     1
#define scr_INFO        0
#define scr_MESSAGE     3

#define PTYPE_free          //          This signature means that a pointer was made in function, and needs to be deleted manually
#define PTYPE_connect       //This signature means that a pointer is connect to other pointers, and no need to be deleted manually
#define PTYPE_depending     //          This signature means that a pointer free/connect type depends of the deeper level function

//main includes
#include <QApplication>
#include <QMainWindow>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QVariant>
#include <QPair>
#include <QStack>
#include <QVector>
#include <QList>
#include <QLibrary>
#include <QLinkedList>
#include <QStringList>
#include <QMap>
#include <QMultiMap>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QTableView>
#include <QSettings>
#include <QTextStream>
#include <QThread>
#include <QAxObject>
#include <chrono>
#include <ctime>
#include <QTime>
#include <QtMath>
#include <typeinfo>
#include <QMessageBox>
#include <memory>

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>



enum objectTypes {Tbase = 0, Tvariable=1, Tsignal=2, Tfunction=3, Tblock=4 };
typedef unsigned long handle_t;

template<typename T>
struct innerMessage
{
    void (T::*_callmsg)(const QString & msg);
};


#endif // PREDEFINES_H
