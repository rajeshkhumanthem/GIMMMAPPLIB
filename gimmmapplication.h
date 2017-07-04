#ifndef GIMMMAPPLICATION_H
#define GIMMMAPPLICATION_H


#include "gimmmconnection.h"

#include <QObject>
#include <QHostAddress>
#include <QSettings>

class GimmmApplication: public QObject
{
    Q_OBJECT
        GimmmConnection             __gimmmConn;
        QHostAddress                __hostAddress;
        quint16                     __hostPortNo;
        std::string                 __sessionId;
        std::int64_t                __currentMessageId;
        QSettings*                  __ini;

    public:
        GimmmApplication();
    public slots:
        // Application hooks. Override them as necessary.
        virtual void handleNewUpstreamMessage(const QJsonDocument& json);
        virtual void handleNewDownstreamAckMessage(const QJsonDocument& json);
        virtual void handleNewDownstreamRejectMessage(
                        const QJsonDocument& json,
                        const QString& reject_reason);
        virtual void handleNewDownstreamReceiptMessage(
                        const QJsonDocument& json);
        virtual void handleConnectionStarted(int attemptno, int waittime);
        virtual void handleConnectionError(int, const QString& error);
        virtual void handleSessionEstablished();
        virtual void handleConnectionLost();

        virtual void handleConnectionEstablished();
        virtual void handleConnectionShutdownStarted();
        virtual void handleConnectionShutdownCompleted();
        virtual void handleConnectionHandshakeStarted();

        virtual void sendDownstreamMessage(const QJsonDocument& jdoc);
        virtual void sendAckMessage(const QJsonDocument& orig_msg);
        virtual std::string getNextMessageId();
   signals:
        void sendMessage(const QJsonDocument& jdoc);
   private:
        // setup functions
        void start();
        void setupGimmmStuff();
        virtual void readConfigFile();
   private slots:
        void newUpstreamMessage(const QJsonDocument& json);
        void newDownstreamAckMessage(const QJsonDocument& json);
        void newDownstreamRejectMessage(const QJsonDocument& json);
        void newDownstreamReceiptMessage(const QJsonDocument& json);
};

#endif // GIMMMAPPLICATION_H
