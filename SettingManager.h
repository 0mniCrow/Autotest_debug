#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H
#include "GUI_load_window.h"

class SettingManager:public QObject
{
public:
    SettingManager();
    bool getSettings(QMap<QString,QVariant> & container);
    bool setSettings(QMap<QString,QVariant> & container);


};

#endif // SETTINGMANAGER_H
