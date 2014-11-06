#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QProcess>
#include <QTime>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QProgressBar;
class QTextEdit;
class QTableWidget;
class QTime;

class Widget : public QWidget
{
    Q_OBJECT

private:
    QLabel *ssidLabel, *passwordLabel, *networkLabel;
    QLineEdit *ssidLineEdit, *passwordLineEdit;
    QPushButton *startButton, *stopButton;
    QPushButton *moreButton;
    QComboBox *networkComboBox;
    QTableWidget *table;
    QCheckBox *showPassword;
    QString ssid, password;
    QString wlan, net;
    QString user;
    QProcess *procAp;

public:
    QString  initInformation;
    bool wlanEnabled;
    bool wlanConnected;
    bool netConnected;

    //they should be const!!!
    QString hotspotconfig;	//the directory of hot spot configuration file
    QString dnsmasqconfig;	//the directory of dnsmasq configuration file
    QString logFile;
    QString pidFile;


    Widget(QWidget *parent = 0);

    QString InterfaceWlan();
    QString InterfaceNet();
    bool IsWlanAvailable();
    bool IsRoot();

    void setNetComboBox();
    bool readConfig();
    bool writeConfig();
    void onStartUp();


    ~Widget();
 private slots:
    bool startHotspot();
    void stopHotspot();
    void displayPassword();
    void getSSID();
    void getPassWD();
    void getNet( const QString &str);
 //   void readInitialInformation();
 //   void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
 //   void processError(QProcess::ProcessError error);
public slots:
    bool IsSupport();
    void GetInitInformation();
};

#endif // WIDGET_H
