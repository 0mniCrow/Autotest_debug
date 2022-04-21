#include "GUI_Interface.h"

int main(int argc, char *argv[])
{  
    QApplication a(argc, argv);
    QStringList arguments = a.arguments();
    arguments.removeFirst();
    GUI_Interface w(arguments);
    w.show();

    return a.exec();
}
