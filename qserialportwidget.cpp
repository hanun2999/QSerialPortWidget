#include "qserialportwidget.h"
#include "ui_qserialportwidget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QTimer>
#include <QSettings>
#include <QFile>

QSerialPortWidget::QSerialPortWidget(const QString & path,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSerialPortWidget),
    vis(Port | BaudRate | DataBits | StopBits | Parity | AutoOpen),
    timer(new QTimer(this)),
    pth(path)
{
    ui->setupUi(this);

    p = new QSerialPort(this);

    connect(ui->butClose,SIGNAL(clicked()),this,SLOT(closeComport()));
    connect(ui->butOpen,SIGNAL(clicked()),this,SLOT(openComport()));

    connect(p,SIGNAL(aboutToClose()),this,SLOT(closeComport()));
    setVisibleFlags(vis);

    fillCombos();
    //load from file
    if (loadFile(Info))
    {
        setWidget(Info);
        if (Info.autoOpen)
            openComport();
    }
    else
    {
        SetDefaultValues();
    }

    timer->start(1000);
    connect(timer,SIGNAL(timeout()),this,SLOT(timeout()));
}

QSerialPortWidget::~QSerialPortWidget()
{
    p->close();
    delete ui;
}

void QSerialPortWidget::openComport()
{
    //save to file
    //setup port
    fillInfo(Info);
    setPort(Info);

    //try open
    bool ok = p->open(QIODevice::ReadWrite);
    if (!ok)
    {
        emit cannotOpenPort();
        return;
    }

    emit portOpened(true);
    emit portOpened(p);
    enableWidget(false);
    saveFile(Info);
}

/**
 * @brief QSerialPortWidget::setPort
 * write port parameters from inf to real serial port
 * @param inf
 */
void QSerialPortWidget::setPort(const Info_t &inf)
{
    p->setPortName(inf.Port);
    p->setParity(inf.Parity);
    p->setBaudRate(inf.BaudRate);
    p->setDataBits(inf.DataBits);
    p->setFlowControl(inf.FlowControl);
    p->setStopBits(inf.StopBits);
}



void QSerialPortWidget::closeComport()
{
    if (sender() != p)
        p->close();

    enableWidget(true);
    emit portOpened(false);

}

void QSerialPortWidget::enableWidget(bool b)
{
    ui->comboBaud->setEnabled(b);
    ui->comboDataBits->setEnabled(b);
    ui->comboHandsake->setEnabled(b);
    ui->comboPort->setEnabled(b);
    ui->comboStopBits->setEnabled(b);
    ui->butOpen->setEnabled(b);
    //ui->checkAuto->setEnabled(b);
    ui->comboParity->setEnabled(b);
}

void QSerialPortWidget::setVisibleFlags(Visibility flags)
{
    vis = flags;

    for (int i = 0 ; i < ui->formLayout->rowCount(); i++)
    {
        bool vis = flags.testFlag((Visible) (1 << i));

        ui->formLayout->itemAt(i,QFormLayout::LabelRole)->widget()->setVisible(vis);
        ui->formLayout->itemAt(i,QFormLayout::FieldRole)->widget()->setVisible(vis);
    }

    ui->checkAuto->setVisible(flags.testFlag(AutoOpen));
}

void QSerialPortWidget::SetDefaultValues()
{
    QString str = ui->comboPort->currentText();

    Info.autoOpen = false;
    Info.BaudRate = 19200;
    Info.DataBits = QSerialPort::Data8;
    Info.FlowControl = QSerialPort::NoFlowControl;
    Info.Parity = QSerialPort::NoParity;
    Info.Port = str;
    Info.StopBits = QSerialPort::OneStop;
    setWidget(Info);
}

/**
 * @brief QSerialPortWidget::timeout
 * autoscanning list of serial ports
 */
void QSerialPortWidget::timeout()
{
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    QStringList lst;

    foreach (QSerialPortInfo info, list) {
        QString name = info.portName();
        int i = ui->comboPort->findText(name);
        lst << name;
        if (i == -1)
        {
            ui->comboPort->addItem(name);
        }
    }

    for (int i = 0 ; i < ui->comboPort->count(); i++)
    {
        if (!lst.contains(ui->comboPort->itemText(i)))
            ui->comboPort->removeItem(i);
    }

    Info_t in;
    fillInfo(in);
    if (!(Info == in))
    {
        Info = in;
        saveFile(Info);
    }
}

bool QSerialPortWidget::setPortSetup(const Info_t &inf)
{
    if (p->isOpen())
        return false;

    setWidget(inf);
    return true;
}

/**
 * @brief QSerialPortWidget::setWidget
 * load setting from info structure to gui
 * @param inf
 */
void QSerialPortWidget::setWidget(const Info_t &inf)
{
    setCombo(ui->comboBaud,inf.BaudRate);
    setCombo(ui->comboDataBits,inf.DataBits);
    setCombo(ui->comboPort,inf.Port,Qt::DisplayRole,false);
    setCombo(ui->comboHandsake,inf.FlowControl,Qt::UserRole);
    setCombo(ui->comboParity,inf.Parity,Qt::UserRole);
    setCombo(ui->comboStopBits,inf.StopBits,Qt::UserRole);
    ui->checkAuto->setChecked(inf.autoOpen);
}

/**
 * @brief QSerialPortWidget::fillInfo
 * fill param structure from gui
 * @param inf
 */
void QSerialPortWidget::fillInfo(Info_t &inf)
{
    bool okk;

    inf.BaudRate = readCombo(ui->comboBaud).toInt(&okk);
    Q_ASSERT(okk);
    inf.DataBits = (QSerialPort::DataBits)readCombo(ui->comboDataBits).toInt(&okk);
    Q_ASSERT(okk);
    inf.Parity =  (QSerialPort::Parity)readCombo(ui->comboParity,Qt::UserRole).toInt(&okk);
    Q_ASSERT(okk);
    inf.Port = readCombo(ui->comboPort).toString();
    inf.StopBits = (QSerialPort::StopBits)readCombo(ui->comboStopBits,Qt::UserRole).toInt(&okk);
    Q_ASSERT(okk);
    inf.FlowControl = (QSerialPort::FlowControl)readCombo(ui->comboHandsake,Qt::UserRole).toInt(&okk);
    Q_ASSERT(okk);
    inf.autoOpen = ui->checkAuto->isChecked();
}

/**
 * @brief QSerialPortWidget::fillCombos
 * fill all comboboxes with all items and set their userrole
 */
void QSerialPortWidget::fillCombos()
{
    //list combos
    ui->comboPort->clear();
    QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
    foreach (QSerialPortInfo info, list) {
        ui->comboPort->addItem(info.portName());
    }

    //bauds
    QStringList bauds;
    int b = 1200;
    for (int i = 0 ; i < 6;i++)
    {
        bauds.append(QString::number(b));
        b*=2;
    }
    bauds << "57600" << "115200";
    ui->comboBaud->clear();
    ui->comboBaud->addItems(bauds);

    //data bits
    ui->comboDataBits->clear();
    for (int i = 4 ; i < 9;i++)
    {
        ui->comboDataBits->addItem(QString::number(i));
    }

    //stop bits
    QStringList stops;
    stops << "1"  << "2" << "1.5";
    ui->comboStopBits->clear();
    ui->comboStopBits->addItems(stops);
    for(int i = 0 ; i < 3; i++)
    {
        ui->comboStopBits->setItemData(i,i+1);
    }

    //handshaking
    QStringList shaking ;
    shaking << tr("None") << tr("Hardware (RTS/CTS)") << tr("Software (Xon/Xoff)");
    ui->comboHandsake->clear();
    ui->comboHandsake->addItems(shaking);
    for (int i =  0 ; i < ui->comboHandsake->count(); i++)
    {
        ui->comboHandsake->setItemData(i,i);
    }

    //parity
    QStringList parity;
    parity << tr("No parity") << tr("Even") << tr("Odd") << tr("Space") << tr("Mark");
    ui->comboParity->clear();
    ui->comboParity->addItems(parity);

    ui->comboParity->setItemData(0,0);
    for (int i =  1 ; i < ui->comboParity->count(); i++)
    {
        ui->comboParity->setItemData(i,i+1);
    }
}

/**
 * @brief QSerialPortWidget::readCombo
 * return Qvariant role of current item in combobox
 * @param combo
 * @param role
 * @return
 */
QVariant QSerialPortWidget::readCombo(QComboBox *combo, Qt::ItemDataRole role)
{
    return combo->itemData(combo->currentIndex(),role);
}

/**
 * @brief QSerialPortWidget::setCombo
 * set combo current index according to data and role
 * @param combo
 * @param data
 * @param role
 * @param check
 */
void QSerialPortWidget::setCombo(QComboBox *combo, const QVariant &data,
                                 Qt::ItemDataRole role, bool check)
{
    int i;
    i = combo->findData(data,role);
    if (check)
        Q_ASSERT(i != -1);

    combo->setCurrentIndex(i);
}

void QSerialPortWidget::printSetting(const Info_t &info)
{
    qDebug() << "\n";
    qDebug() << "Port " << info.Port;
    qDebug() << "Rate " << info.BaudRate;
    qDebug() << "Data bits " << info.DataBits;
    qDebug() << "Stop bits " << info.StopBits;
    qDebug() << "handshake " << info.FlowControl;
    qDebug() << "parity " << info.Parity;
    qDebug() << "\n";
}

bool QSerialPortWidget::loadFile(Info_t &inf)
{
    QFile fil(pth);
    if (!fil.exists())
        return false;

    QSettings s(pth,QSettings::IniFormat);

    inf.autoOpen = s.value("com/autoOpen").toBool();
    inf.BaudRate = s.value("com/baudRate").toInt();
    inf.Port = s.value("com/port").toString();
    inf.DataBits = (QSerialPort::DataBits) s.value("com/dataBits").toInt();
    inf.StopBits = (QSerialPort::StopBits) s.value("com/stopBits").toInt();
    inf.FlowControl = (QSerialPort::FlowControl)s.value("com/handshake").toInt();
    inf.Parity = (QSerialPort::Parity) s.value("com/parity").toInt();

    return true;
}

void QSerialPortWidget::saveFile(const Info_t &inf)
{
    QSettings s(pth,QSettings::IniFormat);
    s.setValue("com/autoOpen",inf.autoOpen);
    s.setValue("com/baudRate",inf.BaudRate);
    s.setValue("com/port",inf.Port);
    s.setValue("com/dataBits",inf.DataBits);
    s.setValue("com/stopBits",inf.StopBits);
    s.setValue("com/handshake",inf.FlowControl);
    s.setValue("com/parity",inf.Parity);
}
