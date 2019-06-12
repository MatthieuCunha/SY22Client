#include <QtWidgets>
#include <QtNetwork>

#include "client.h"

Client::Client(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , getFortuneButton(new QPushButton(tr("Toggle video")))
    , connectButton(new QPushButton(tr("connect client")))
    , tcpSocket(new QTcpSocket(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    hostCombo->setEditable(true);
    imageSize = 0;
    toggle = false;
    // find out name of this machine
    QString name = QHostInfo::localHostName();
    if (!name.isEmpty()) {
        hostCombo->addItem(name);
        QString domain = QHostInfo::localDomainName();
        if (!domain.isEmpty())
            hostCombo->addItem(name + QChar('.') + domain);
    }
    if (name != QLatin1String("localhost"))
        hostCombo->addItem(QString("localhost"));
    // find out IP addresses of this machine
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // add non-localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (!ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }
    // add localhost addresses
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i).isLoopback())
            hostCombo->addItem(ipAddressesList.at(i).toString());
    }

    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    auto hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel(tr("S&erver port:"));
    portLabel->setBuddy(portLineEdit);

    statusLabel = new ClickableLabel();



    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(true);

    connectButton->setDefault(true);
    connectButton->setEnabled(true);

    auto quitButton = new QPushButton(tr("Quit"));

    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(getFortuneButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);
     buttonBox->addButton(connectButton, QDialogButtonBox::ActionRole);

    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    connect(hostCombo, &QComboBox::editTextChanged,
            this, &Client::enableGetFortuneButton);
    connect(portLineEdit, &QLineEdit::textChanged,
            this, &Client::enableGetFortuneButton);
    connect(connectButton, &QAbstractButton::clicked,
            this, &Client::requestNewFortune);
    connect(getFortuneButton, &QAbstractButton::clicked,
            this, &Client::toggleVideo);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);
    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readFortune);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &Client::displayError);
    connect(statusLabel,SIGNAL(clicked(QPoint)) ,this, SLOT(sendClick(QPoint)));

    QGridLayout *mainLayout = nullptr;
    if (QGuiApplication::styleHints()->showIsFullScreen() || QGuiApplication::styleHints()->showIsMaximized()) {
        auto outerVerticalLayout = new QVBoxLayout(this);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
        auto outerHorizontalLayout = new QHBoxLayout;
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        auto groupBox = new QGroupBox(QGuiApplication::applicationDisplayName());
        mainLayout = new QGridLayout(groupBox);
        outerHorizontalLayout->addWidget(groupBox);
        outerHorizontalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Ignored));
        outerVerticalLayout->addLayout(outerHorizontalLayout);
        outerVerticalLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding));
    } else {
        mainLayout = new QGridLayout(this);
    }
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostCombo, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addWidget(buttonBox, 3, 0, 1, 2);

    setWindowTitle(QGuiApplication::applicationDisplayName());
    portLineEdit->setFocus();

    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }

        networkSession = new QNetworkSession(config, this);
        connect(networkSession, &QNetworkSession::opened, this, &Client::sessionOpened);

        getFortuneButton->setEnabled(false);
        statusLabel->setText(tr("Opening network session."));
        networkSession->open();
    }

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readFortune()));
    timer->start(2000);
}

void Client::requestNewFortune()
{
    connectButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(),
                             portLineEdit->text().toInt());
    qDebug() << tcpSocket->state();

}

void Client::toggleVideo()
{
    toggle= !toggle;

}


void Client::readFortune()
{
    //if(!toggle)return;
     qDebug() << tcpSocket->state();
    QPixmap pixmap = QPixmap();
    /*in.startTransaction();

    QString nextFortune;
    in >> pixmap;

    //in.abortTransaction();
    if (!in.commitTransaction())
        return;

    currentFortune = nextFortune;


    //pixmap->loadFromData(buffer.buffer());*/

    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_7);

    // initialize data
    QImage image;
    //atic quint32 imageSize = 0;

    if ( 0 == imageSize ){

        if ( tcpSocket->bytesAvailable() < (int)sizeof(quint32) )
            return;
        in >> imageSize;
        qDebug() << imageSize;
        qDebug() << tcpSocket->bytesAvailable();
    }

    if ( tcpSocket->bytesAvailable() < imageSize )
        return;
    QByteArray tjpeg;
    in >> tjpeg;
    image.loadFromData(tjpeg);
    // data is complete, do something with it
    imageSize = 0;
    //    if (image.isNull()) qDebug() << "QImage is null!";
    //    else {
    pixmap = QPixmap::fromImage(image);
    statusLabel->setPixmap(pixmap);
    getFortuneButton->setEnabled(true);
    //update();
    //}
}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }

    getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton()
{
    connectButton->setEnabled((!networkSession || networkSession->isOpen()) &&
                                 !hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty());

}

void Client::sessionOpened()
{
    // Save the used configuration
    QNetworkConfiguration config = networkSession->configuration();
    QString id;
    if (config.type() == QNetworkConfiguration::UserChoice)
        id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
    else
        id = config.identifier();

    QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
    settings.beginGroup(QLatin1String("QtNetwork"));
    settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
    settings.endGroup();

    statusLabel->setText(tr("This examples requires that you run the "
                            "Fortune Server example as well."));

    enableGetFortuneButton();
}


void Client::sendClick(QPoint pt)
{
        QString coordinate = QString::number(pt.x())+","+QString::number(pt.y());
         qDebug() << coordinate;

        if(tcpSocket==nullptr){return;}


        QByteArray trame;
        QDataStream out(&trame, QIODevice::WriteOnly);
    //    out.open();
        out.setVersion(QDataStream::Qt_5_7);
        out << (quint32)0;
        out<< coordinate;
       // out.device()->seek(0);
       // out<<((trame.size()-sizeof(quint32)));

        tcpSocket->write(trame);

}

