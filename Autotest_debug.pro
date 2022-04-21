#-------------------------------------------------
#
# Project created by QtCreator 2020-08-24T12:03:39
# Axolotles are cool!
#
#-------------------------------------------------

QT       += core gui axcontainer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Autotest_debug
TEMPLATE = app


SOURCES += main.cpp\
        GUI_Interface.cpp \
    DataObject.cpp \
    VarObject_v2.cpp \
    FuncObject_v2.cpp \
    FuncManager_v2.cpp \
    BlockObject_v2.cpp \
    SignalObject_v2.cpp \
    ScripterCore_v2.cpp \
    LogicManager_v2.cpp \
    DataModels_v2.cpp \
    ScripterManager_v2.cpp \
    ConfigEditor_v2.cpp \
    GUI_Setting.cpp \
    GUI_load_window.cpp \
    ExcelApp.cpp \
    GUI_load_Excel.cpp \
    RemoteFunctionController.cpp \
    SettingManager.cpp

HEADERS  += GUI_Interface.h \
    Predefines.h \
    DataObject.h \
    MemoryDirector.h \
    VarObject_v2.h \
    FuncObject_v2.h \
    FuncManager_v2.h \
    BlockObject_v2.h \
    SignalObject_v2.h \
    ScripterCore_v2.h \
    LogicManager_v2.h \
    DataModels_v2.h \
    ScripterManager_v2.h \
    ConfigEditor_v2.h \
    GUI_Setting.h \
    GUI_load_window.h \
    ExcelApp.h \
    GUI_load_Excel.h \
    RemoteFunctionController.h \
    SettingManager.h

FORMS    += GUI_Interface.ui \
    GUI_Setting.ui \
    GUI_load_window.ui \
    GUI_load_Excel.ui

#win32: LIBS += -L$$PWD/./ -llibusb-1.0.dll
INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

#win32: LIBS += "C:/Program Files (x86)/IVI Foundation/VISA/WinNT/lib/msc/visa32.lib"
#INCLUDEPATH += "C:/Program Files/IVI Foundation/VISA/Win64/Include"
#DEPENDPATH  += "C:/Program Files/IVI Foundation/VISA/Win64/Include"
