#include "qserialportwidget.h"
#include "ui_qserialportwidget.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QTreeWidgetItem>

QSerialPortWidget::QSerialPortWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QSerialPortWidget),
    vis(Port | BaudRate | DataBits | StopBits | Parity | AutoOpen)
{
    ui->setupUi(this);

    p = new QSerialPort(this);

    connect(ui->butClose,SIGNAL(clicked()),this,SLOT(closeComport()));
    connect(ui->butOpen,SIGNAL(clicked()),this,SLOT(openComport()));

    connect(p,SIGNAL(aboutToClose()),this,SLOT(closeComport()));
    setVisibleFlags(vis);

    fillCombos();
    //load from file
    SetDefaultValues();
    //if auto click open



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

/**
 * @brief QSerialPortWidget::fillInfo
 * fill param structure from gui
 * @param inf
 */
void QSerialPortWidget::fillInfo(Info_t &inf)
{
    bool okk;

    inf.BaudRate = ui->comboBaud->currentText().toInt(&okk);
    Q_ASSERT(okk);
    inf.DataBits = (QSerialPort::DataBits)ui->comboDataBits->currentText().toInt(&okk);
    Q_ASSERT(okk);
    inf.Parity =  (QSerialPort::Parity)
            ui->comboParity->itemData(ui->comboParity->currentIndex()).toInt(&okk);
    Q_ASSERT(okk);
    inf.Port = ui->comboPort->currentText();
    inf.StopBits = (QSerialPort::StopBits)
            ui->comboStopBits->itemData(ui->comboStopBits->currentIndex()).toInt(&okk);
    Q_ASSERT(okk);
    inf.FlowControl =  (QSerialPort::FlowControl)ui->comboHandsake->currentIndex();
    Q_ASSERT(okk);
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
    ui->checkAuto->setEnabled(b);
    ui->comboParity->setEnabled(b);
}

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
    ui->comboBaud->setCurrentIndex(4);
    ui->comboDataBits->setCurrentIndex(4);
    ui->comboHandsake->setCurrentIndex(0);
    ui->comboParity->setCurrentIndex(0);
    ui->comboPort->setCurrentIndex(0);
    ui->comboStopBits->setCurrentIndex(0);
}
