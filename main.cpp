#include "widget.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    QMessageBox msgBox;
    if(!w.IsSupport())		//if not supported, exit
    {
        msgBox.setText("Sorry, Your wireless card or driver do not support AP mode!");
        msgBox.exec();
        return 0;
    }
    w.onStartUp();
    w.IsRoot();
    //w.writeConfig();
    w.show();

    return a.exec();
}
