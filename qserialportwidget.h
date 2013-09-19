#ifndef QSERIALPORTWIDGET_H
#define QSERIALPORTWIDGET_H

#include <QWidget>
#include <QtSerialPort/QSerialPort>

namespace Ui {
class QSerialPortWidget;
}

class QComboBox;
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
        QString Port;                   ///displayrole
        quint32 BaudRate;               ///displayrole
        QSerialPort::DataBits DataBits; ///displayrole
        QSerialPort::StopBits StopBits; ///userrole
        QSerialPort::Parity Parity;     ///userrole
        QSerialPort::FlowControl FlowControl; ///userrole
        bool autoOpen;
    }Info_t;


public slots:
    void openComport(void);
    void closeComport(void);


private:
    Ui::QSerialPortWidget *ui;
    QSerialPort * p;
    Info_t Info;
    Visibility vis;
    QTimer * timer;

    void fillCombos(void);
    void SetDefaultValues(void);
    void enableWidget(bool enabled);
    void fillInfo(Info_t & inf);
    void setPort(const Info_t & inf);
    bool loadFile(Info_t & inf);
    void setWidget(const Info_t & inf);
    static void setCombo(QComboBox * combo, const QVariant & data,
                         Qt::ItemDataRole role = Qt::DisplayRole, bool check = true);
    static QVariant readCombo(QComboBox * combo, Qt::ItemDataRole role = Qt::DisplayRole);
    void printSetting(const Info_t & info);


private slots:
    void timeout(void);

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
