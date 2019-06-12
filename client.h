#ifndef CLIENT_H
#define CLIENT_H

#include <QDataStream>
#include <QDialog>
#include <QTcpSocket>
#include <QBuffer>
#include "clickablelabel.h"

class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;
class QNetworkSession;

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);

private slots:
    void requestNewFortune();
    void readFortune();
    void toggleVideo();
    void displayError(QAbstractSocket::SocketError socketError);
    void enableGetFortuneButton();
    void sessionOpened();
    void sendClick(QPoint pt);

private:
    QComboBox *hostCombo = nullptr;
    QLineEdit *portLineEdit = nullptr;
    ClickableLabel *statusLabel = nullptr;
    QPushButton *getFortuneButton = nullptr;
    QPushButton *connectButton = nullptr;

    QTcpSocket *tcpSocket = nullptr;

    QDataStream in;
    QString currentFortune;
    QBuffer buffer;

    QNetworkSession *networkSession = nullptr;
    quint32 imageSize;
    bool toggle;
};

#endif
