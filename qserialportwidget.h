#ifndef QSERIALPORTWIDGET_H
#define QSERIALPORTWIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class QSerialPortWidget;
}

class QSerialPortWidget : public QWidget
{
    Q_OBJECT
    Q_FLAGS(Visibility)

public:
    explicit QSerialPortWidget(QWidget *parent = 0);
    ~QSerialPortWidget();

    inline QSerialPort * getPort() const {return p;}

    enum Visible  {
        Nothing = 0,
        Port = 0x01,
        BaudRate = 0x02,
        DataBits = 0x04,
        StopBits = 0x08,
        Parity = 0x10,
        AutoOpen = 0x20
    };
    Q_DECLARE_FLAGS(Visibility, Visible)

    typedef struct
    {
        QString Port;
        quint32 BaudRate;
        QSerialPort::DataBits DataBits;
        QSerialPort::StopBits StopBits;
        QSerialPort::Parity Parity;
        QSerialPort::FlowControl FlowControl;
    }Info_t;


public slots:
    void openComport(void);
    void closeComport(void);

private:
    Ui::QSerialPortWidget *ui;
    QSerialPort * p;
    Info_t Info;

    void fillCombos(void);
    void SetDefaultValues(void);
    void enableWidget(bool enabled);
    void fillInfo(Info_t & inf);
    void setPort(const Info_t & inf);

    Visibility vis;


signals:
    void cannotOpenPort();
    void portOpened(bool);
    void portOpened(QSerialPort * com);

public:
    //hide / show...
    void setVisibleFlags(Visibility flags);
    Visibility visibleFlags() const {return vis;}

    //set values before com port is opened
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSerialPortWidget::Visibility)

#endif // QSERIALPORTWIDGET_H
