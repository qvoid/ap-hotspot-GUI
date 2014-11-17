#include "widget.h"
#include <QtWidgets>
#include <QDebug>
#include <QMessageBox>
#include <QFile>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QRegExp ssidRegExp("\\S{0,32}");
    QRegExp passwordRegExp("\\S{0,32}");

    //Create widgets
    ssidLabel = new QLabel(tr("SSID"));
    ssidLabel->setAlignment(Qt::AlignRight);
    ssidLineEdit = new QLineEdit;
    ssidLineEdit->setValidator(new QRegExpValidator(ssidRegExp,this));

    passwordLabel = new QLabel(tr("Password"));
    passwordLabel->setAlignment(Qt::AlignRight);
    passwordLineEdit = new QLineEdit;
    passwordLineEdit->setValidator(new QRegExpValidator(passwordRegExp,this));

    networkLabel = new QLabel(tr("Network"));
    networkLabel->setAlignment(Qt::AlignRight);
    networkComboBox = new QComboBox;

    startButton = new QPushButton(tr("Start"));
    stopButton = new QPushButton(tr("Stop"));
    startButton->setDefault(true);
    stopButton->setEnabled(false);
    startButton->setIcon(QIcon(":/images/start.png"));
    stopButton->setIcon(QIcon(":/images/stop.png"));

    showPassword = new QCheckBox(tr("Show Password"));
    showPassword->setChecked(true);
    moreButton = new QPushButton(tr("More Options"));

    table = new QTableWidget(this);
    table->setColumnCount(4);
    table->setRowCount(5);
    QStringList header;
    header<<"Device Name"<<"IP"<<"Mac"<<"Security";
    table->setHorizontalHeaderLabels(header);
    table->setItem(0,0, new QTableWidgetItem(QString("android-32323223123")));
    table->setItem(0,1, new QTableWidgetItem(QString("192.168.150.3")));
    table->setItem(0,2, new QTableWidgetItem(QString("98:98:00:00:00:00")));
    table->setItem(0,3, new QTableWidgetItem(QString("Unknown")));


    //Set layouts
    QHBoxLayout *ssidLayout = new QHBoxLayout;
    ssidLayout->addWidget(ssidLabel);
    ssidLayout->addWidget(ssidLineEdit);

    QHBoxLayout *passwordLayout = new QHBoxLayout;
    passwordLayout->addWidget(passwordLabel);
    passwordLayout->addWidget(passwordLineEdit);

    QHBoxLayout *networkLayout = new QHBoxLayout;
    networkLayout->addWidget(networkLabel);
    networkLayout->addWidget(networkComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);

    QHBoxLayout *moreLayout = new QHBoxLayout;
    moreLayout->addWidget(showPassword);
    moreLayout->addWidget(moreButton);


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(ssidLayout);
    mainLayout->addLayout(passwordLayout);
    mainLayout->addLayout(networkLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addLayout(moreLayout);
    mainLayout->addWidget(table);

    setLayout(mainLayout);

    setWindowTitle("AP-Hotsopt");
    setWindowIcon(QIcon(":/images/wireless.png"));

    passwordLineEdit->setFixedWidth(140);
    ssidLineEdit->setFixedWidth(passwordLineEdit->width());
    networkComboBox->setFixedWidth(passwordLineEdit->width());
    setFixedSize(sizeHint().width(),sizeHint().height());

    //configuration files
    hotspotconfig = "/etc/ap-hotspot.conf";
    dnsmasqconfig = "/etc/dnsmasq.d/ap-hotspot.rules";
    logFile = "/tmp/hostapd.log";
    pidFile = "/tmp/hotspot.pid";

    //set the defalut ssid and password
    ssid = "myhotspot";
    password = "qwerty0987";

    //Signal and Slot
    connect(showPassword, SIGNAL(stateChanged(int)),
            this, SLOT(displayPassword()));
    connect(startButton, SIGNAL(clicked()),
            this, SLOT(startHotspot()));
    connect(stopButton, SIGNAL(clicked()),
            this, SLOT(stopHotspot()));
    connect(ssidLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(getSSID()));
    connect(passwordLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(getPassWD()));
    connect(networkComboBox,SIGNAL(activated(QString)),
            this, SLOT(getNet(QString)));
}

/*********************************************************
 * Show password whether or not.
 * If the checkbox is checked, showing the password.
 * Otherwise, hiding the password.
 * ******************************************************/
void Widget::displayPassword()
{
    if(showPassword->isChecked())
        passwordLineEdit->setEchoMode(QLineEdit::Normal);
    else
        passwordLineEdit->setEchoMode(QLineEdit::Password);
}


void Widget::getSSID()
{
    if(ssidLineEdit->hasAcceptableInput())
        ssid = ssidLineEdit->text();
    qDebug()<<"ssid is: "<<ssid<<endl;
}


void Widget::getPassWD()
{
    if(passwordLineEdit->hasAcceptableInput())
        password = passwordLineEdit->text();
    qDebug()<<"password is: "<<password<<endl;
}

void Widget::getNet(const QString &str)
{
    net = str;
    qDebug()<<"network interface is: "<<net<<endl;
}

/*************************************************
 * Check if the Wireless card or driver is supported
 * Return:
 * ture: Supported; false: Not Supported
 ************************************************/
bool Widget::IsSupport()
{
    QProcess proc;

    proc.start("/home/void/Desktop/ap-hotspot-GUI/cmd/ap-hotspot.sh check 2>/dev/null");
    proc.waitForStarted();
    proc.waitForFinished();

    QByteArray stdOut = proc.readAllStandardOutput();
    qDebug()<<"stdOut:"<<stdOut<<endl;
    qDebug()<<"stdOut[0]-48:"<<stdOut[0]-48<<endl;
    qDebug()<<"Finished"<<endl;
    //'0' is 48, '1' is 49
    return (bool)(stdOut[0]-48);
}

/*********************************************************
 * get User, Wlan, Net, etc
 * stored in a QString
 * ******************************************************/
void Widget::GetInitInformation()
{
    QProcess proc;
    proc.start("/home/void/Desktop/ap-hotspot-GUI/cmd/ap-hotspot.sh vars 2>/dev/null");
    proc.waitForStarted();
    proc.waitForFinished();

    QByteArray stdOut = proc.readAllStandardOutput();
    initInformation = QString::fromLocal8Bit(stdOut);
    qDebug()<<"initInformation:"<<initInformation<<endl;
}
/********************************************************************
 * Check Wireless card if it is enabled or disconnected
 * Return:
 * true: available; false: unavailable
 * BUG: If the WiFi is connected, the checking operation is too slow,
 * 	cause the main widget pop out at after a short time delay.
 * *****************************************************************/
bool Widget::IsWlanAvailable()
{
    int q = initInformation.indexOf("WLANStatus:");
    std::string str = initInformation.toStdString();
    char status = str[q+11];
    qDebug()<<"Wlan status:"<<status<<endl;
    if(status=='0')
    {
        wlanEnabled = false;
        QMessageBox msgBox;
        msgBox.setText("WiFi is disabled! Please enable!");
        msgBox.exec();
        return false;
    }
    else
        wlanEnabled = true;
    if(status=='1')
    {
        wlanConnected = true;
        QMessageBox msgBox;
        msgBox.setText("Please disconnect your WiFi");
        msgBox.exec();
        return false;
    }
    if(status=='2')
        wlanConnected = false;

    qDebug()<<"wlanEnabled="<<wlanEnabled<<endl;
    qDebug()<<"wlanConnected="<<wlanConnected<<endl;

    return true;
}
/********************************************************************
 * Get the interface of wlan, such as wlan0
 * *****************************************************************/
QString Widget::InterfaceWlan()
{
    //QString::SectionFlag flag = QString::SectionSkipEmpty;
    QString str = initInformation.section(',', 2, 2);
    QString wlanStr = str.section(':', 1, 1);
    qDebug()<<"wlan field is: "<<str<<endl;
    qDebug()<<"Interface of WLAN is: "<<wlanStr<<endl;
    return wlanStr;
}

/********************************************************************
 * Get the interface of net, such as eth0
 * cnt means the counts of interfaces of net
 * str may equal to "tun0 eth0 "(attention: there is a space in the end)
 * *****************************************************************/
QString Widget::InterfaceNet()
{
    QString str = initInformation.section(',', 3, 3);
    str = str.section(':',1);
    //int cnt = str.count(' ');
    //qDebug()<<"cnt="<<cnt<<endl;
    qDebug()<<"net field is:"<<str<<endl;
    return str;
}
/**********************************************************************
 * Set net interface items. Items come from initial information.
 * If the configuration file exists, and the net interface happens to
 * be one of the items from initial information, set it as default value,
 * and show it as default.
 * *******************************************************************/
void Widget::setNetComboBox()
{
    QString str = InterfaceNet();
    int cnt = str.count(' ');
    QString *netItem = new QString[cnt];
    for(int i=0; i<cnt; i++)
    {
        *(netItem+i) = str.section(' ', i, i);
        networkComboBox->addItem(*(netItem+i));
    }
    QFile fileDnsmasq(dnsmasqconfig);
    if(fileDnsmasq.exists())
    {
        if(fileDnsmasq.open(QIODevice::ReadOnly))
        {
            QTextStream *in =new QTextStream(&fileDnsmasq);
            QString strConfig = in->readAll();
            QString strTmp;
            fileDnsmasq.close();
            QRegExp rx("#INTERFACE_NET=\\S*");
            if(strConfig.indexOf( rx )> -1)
            {
                strTmp = rx.cap(0);
                net = strTmp.section('=', 1, 1);			//get net
                qDebug()<<"The interface of net in the config file is: "<<net<<endl;
                for(int i=0; i<cnt; i++)
                    if(networkComboBox->itemText(i) == net)
                        networkComboBox->setCurrentText(net);		//show this item
            }
        }
    }
    else
        net = networkComboBox->itemText(0);
    qDebug()<<"Ther network interface is: "<<net<<endl;
}

bool Widget::IsRoot()
{
    QString str = initInformation.section(',', 0, 0);
    str = str.section(':', 1, 1);
    qDebug()<<"Current User is:"<<str<<endl;
    return ("root"==str);
}


/******************************************************
 * Get vars(ssid, password, wlan, net) from
 * /etc/ap-hotspot.conf and /etc/dnsmasq.d/ap-hotspot.rules.
 * Return true if success, false for failure.
 * If failed to read the file, ssid and password remain the
 * default values(myhotspot, qwerty0987).
 * ****************************************************/
bool Widget::readConfig()
{
    QFile fileHotspot(hotspotconfig);
    if(!fileHotspot.open(QIODevice::ReadOnly))
        return false;
    QTextStream *in = new QTextStream(&fileHotspot);
    QString strConfig= in->readAll();
    fileHotspot.close();
    delete in;
    qDebug()<<"/etc/ap-hotspot.conf:"<<endl<<strConfig<<endl;

    QRegExp rx("ssid=\\S*");
    QString strTmp;
    int pos = strConfig.indexOf( rx );
    if(pos > -1)
        strTmp = rx.cap(0);
    else
        return false;
    ssid = strTmp.section('=', 1, 1);			//get ssid
    qDebug()<<"The ssid in the config file is: "<<ssid<<endl;

    rx = QRegExp("wpa_passphrase=\\S*");
    pos = strConfig.indexOf( rx );
    if(pos > -1)
        strTmp = rx.cap(0);
    else
        return false;
    password = strTmp.section('=', 1, 1);		//get password
    qDebug()<<"The password in the config file is: "<<password<<endl;

    rx = QRegExp("interface=\\S*");
    pos = strConfig.indexOf( rx);
    if(pos > -1)
        strTmp = rx.cap(0);
    else
        return false;

    /****************************************************************
     * It is appropriate to get the values of wlan and net from function
     * GetInitInformation(), not from configuration files, since every
     * time you runnint this program, they may be different from last
     * time. So I have to uncommont these line below.
     * *************************************************************/
    //wlan = strTmp.section('=', 1, 1);			//get wlan
    //qDebug()<<"The interface of wlan is: "<<wlan<<endl;

    //QFile fileDnsmasq(dnsmasqconfig);
    //if(!fileDnsmasq.open(QIODevice::ReadOnly))
    //    return false;
    //in =new QTextStream(&fileDnsmasq);
    //strConfig = in->readAll();
    //fileDnsmasq.close();
    //qDebug()<<"/etc/dnsmasq.d/ap-hotspot.rules:"<<endl<<strConfig<<endl;

    //rx = QRegExp("#INTERFACE_NET=\\S*");
    //pos = strConfig.indexOf( rx );
    //if(pos > -1)
    //    strTmp = rx.cap(0);
    //else
    //    return false;
    //net = strTmp.section('=', 1, 1);			//get net
    //qDebug()<<"The interface of net is: "<<net<<endl;

    qDebug()<<"Reading configuration from file succeeds!"<<endl;
    return true;
}

/*****************************************************
 * Write configure to /etc/ap-hotspot.conf and
 * /etc/dnsmasq.d/ap-hotspot.rules.
 * Return true if success, false for failure.
 * **************************************************/
bool Widget::writeConfig()
{
    QFile *file = new QFile(hotspotconfig);
    //if not exists, create one;
    if(file->exists())
    {
        if(!file->remove())  //remove existing file
        {
            qDebug()<<"Failed to remove "<<hotspotconfig<<endl;
            return false;
        }
    }
    //Create a new one and then reopen it
    if(!file->open(QIODevice::WriteOnly))
    {
        qDebug()<<"Failed to open "<<hotspotconfig<<endl;
        return false;
    }
    file->close();
    if(!file->open(QIODevice::WriteOnly))
    {
        qDebug()<<"Failed to open "<<hotspotconfig<<endl;
        return false;
    }
    QTextStream *out = new QTextStream(file);
    *out<<"# WiFi Hotspot\n"<<"interface="<<wlan<<"\n"
      <<"driver=nl80211\n"<<"#Access Point\n"
      <<"ssid="<<ssid<<"\n"<<"hw_mode=g\n"
      <<"# WiFi Channel:\n"<<"channel=1\n"
      <<"macaddr_acl=0\n"<<"auth_algs=1\n"
      <<"ignore_broadcast_ssid=0\n"<<"wpa=2\n"
      <<"wpa_passphrase="<<password<<"\n"
      <<"wpa_key_mgmt=WPA-PSK\n"
      <<"wpa_pairwise=TKIP\n"
      <<"rsn_pairwise=CCMP\n\0";
    file->close();

    delete file;
    delete out;

    file = new QFile(dnsmasqconfig);
    //if it exists, remove it
    if(file->exists())
    {
        if(!file->remove())
            return false;
    }
    //Create a new one and then reopen it
    if(!file->open(QIODevice::WriteOnly))
    {
        qDebug()<<"Failed to open "<<dnsmasqconfig<<endl;
        return false;
    }
    file->close();
    if(!file->open(QIODevice::WriteOnly))
    {
        qDebug()<<"Failed to open "<<dnsmasqconfig<<endl;
        return false;
    }
    out = new QTextStream(file);
    *out<<"# Bind to only one interface\n"
        <<"bind-interfaces\n"
        <<"# Choose interface for binding\n"
        <<"interface="<<wlan<<"\n"
        <<"# Specify range of IP addresses for DHCP leasses\n"
        <<"dhcp-range=192.168.150.2,192.168.150.10,12h\n"
        <<"#INTERFACE_NET="<<net<<"\n\0";
    file->close();
    // make the dnsmasq configuration file executable
    file->setPermissions(file->permissions() | QFileDevice::ExeOwner | QFileDevice::ExeGroup);

    qDebug()<<"Writing configuration to files succeeds!"<<endl;
    return true;
}

void Widget::onStartUp()
{
    GetInitInformation();
    IsWlanAvailable();
    wlan = InterfaceWlan();
    setNetComboBox();	//net gets its value from here.
    readConfig();		//ssid and password get their values from here.
    ssidLineEdit->setText(ssid);
    passwordLineEdit->setText(password);
    //check if the ap-hotspot is already runnig
    QFile *file = new QFile(pidFile);
    if(file->exists())
    {
        QMessageBox msgBox;
        msgBox.setText("Another process is already running");
        msgBox.exec();
        startButton->setEnabled(false);
        stopButton->setEnabled(true);
        ssidLineEdit->setReadOnly(true);
        passwordLineEdit->setReadOnly(true);
        networkComboBox->setEnabled(false);
    }
}

/*******************************************************
 *
 ******************************************************/
bool Widget::startHotspot()
{
    //If the password is unacceptable, set as default value.
    QRegExp rx("\\S{8,32}");
    if(password.indexOf(rx) < 0)
    {
        password = "qwerty0987";
        passwordLineEdit->setText(password);
        QMessageBox msgBox;
        msgBox.setText("Password should be at least 8 characters without whitespace!!!");
        msgBox.exec();
        return false;
    }
    //if the ssid is unacceptable, set as default value.
    rx = QRegExp("\\S{1,32}");
    if(ssid.indexOf(rx) < 0)
    {
        ssid = "myhotspot";
        ssidLineEdit->setText(ssid);
        QMessageBox msgBox;
        msgBox.setText("Please name you SSID!!!");
        msgBox.exec();
        return false;
    }

    GetInitInformation();
    if(!IsWlanAvailable())
        return false;
    if(!writeConfig())
        return false;
    //Making line editor uneditable and the start button unclickable.
    networkComboBox->setEnabled(false);
    ssidLineEdit->setReadOnly(true);
    passwordLineEdit->setReadOnly(true);
    startButton->setEnabled(false);
    //startButton->setText("Restart");
    //startButton->setIcon(QIcon(":/images/restart.png"));
    stopButton->setEnabled(true);
    procAp = new QProcess(this);
    procAp->startDetached("ap-hotspot start");
    //if(!procAp->waitForFinished())
    //    qDebug()<<"ap-hotspot hasnt start to work"<<endl;
    //qDebug()<<"ap-hotspot is now running at backend"<<endl;
    return true;
}

void Widget::stopHotspot()
{
    procAp = new QProcess(this);
    procAp->start("ap-hotspot stop");
    if(!procAp->waitForFinished())
        qDebug()<<"Failed stop ap-hotspot"<<endl;
    else
        qDebug()<<"ap-hotspot has been shutdown successfully"<<endl;
    networkComboBox->setEnabled(true);
    ssidLineEdit->setReadOnly(false);
    passwordLineEdit->setReadOnly(false);
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
}
Widget::~Widget()
{

}
