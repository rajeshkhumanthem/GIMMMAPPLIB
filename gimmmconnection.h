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
        ExponentialBackoff  __exBackOff;
        int                 __maxRetry;
    public:
        GimmmConnection(int max_retry = 6);
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
        void connectionStarted(int attemptno, int waittime);
        void connectionEstablished();
        void connectionShutdownStarted();
        void connectionShutdownCompleted();
        void connectionError(int errorno, QString error);
        void connectionLost();
        void sessionEstablished();
        void connectionHandshakeStarted();
        void maxConnectionAttemptReached(int max_attempt);

        void newUpstreamMessage(const QJsonDocument& upstream_msg);
        void newDownstreamAckMessage(const QJsonDocument& orig_msg);
        void newDownstreamReceiptMessage(const QJsonDocument& orig_msg);
        void newDownstreamRejectMessage(const QJsonDocument& orig_msg);
    private:
        void handleNewMessage(const QJsonDocument& jdoc);
        void handleLogonResponseMessage(const QJsonDocument& jdoc);
        void handleUpstreamMessage(const QJsonDocument& jdoc);
        void handleDownstreamAckMessage(const QJsonDocument& jdoc);
        void handleDownstreamRejectMessage(const QJsonDocument& jdoc);
        void tryConnect();
        void retryConnectWithBackoff();
        void handleError(QAbstractSocket::SocketError error);
        void sendMessage(const QJsonDocument& upstream_msg);
};

#endif // GIMMMCONNECTION_H
