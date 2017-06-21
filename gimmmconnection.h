#ifndef GIMMMCONNECTION_H
#define GIMMMCONNECTION_H

#include "exponentialbackoff.h"

#include <memory>

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QEvent>
#include <QThread>
#include <QJsonDocument>
#include <QDataStream>
#include <QtNetwork/QHostAddress>


class GimmmConnection: public QObject
{
        Q_OBJECT
        QTcpSocket          __socket;
        QDataStream         __in;
        QHostAddress        __address;
        quint16             __port;
        std::string         __sessionId;
        int                 __connectAttempt;
        int                 __connectWaitTime;
        ExponentialBackoff  __exBackOff;
    public:
        GimmmConnection();
        virtual ~GimmmConnection();
        void connectToServer(const QHostAddress& hostadd,
                             quint16 portno,
                             const std::string& session_id);

        //getter/setters
    public slots:
        void handleConnected();
        void handleDisconnected();
        void handleReadyRead();
        void handleSendMessage(const QJsonDocument& upstream_msg);
    signals:
        void connectionStarted();
        void connectionEstablished();
        void connectionShutdownStarted();
        void connectionShutdownCompleted();
        void connectionError(QString error);
        void connectionLost();
        void sessionEstablished();
        void connectionHandshakeStarted();

        void newUpstreamMessage(const QJsonDocument& upstream_msg);
        void newDownstreamRejectMessage(
            const QJsonDocument& orig_msg,
            const QString& reject_reason);
        void newDownstreamAckMessage(const QJsonDocument& orig_msg);
    private:
        void handleNewMessage(const QJsonDocument& jdoc);
        void handleLogonResponseMessage(const QJsonDocument& jdoc);
        void handleUpstreamMessage(const QJsonDocument& jdoc);
        void handleDownstreamAckMessage(const QJsonDocument& jdoc);
        void handleDownstreamRejectMessage(const QJsonDocument& jdoc);
        void tryConnect();
        void handleError(QAbstractSocket::SocketError error);
        void sendMessage(const QJsonDocument& upstream_msg);
};

#endif // GIMMMCONNECTION_H
