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
        virtual void handleNewDownstreamRejectMessage(
                        const QJsonDocument& json,
                        const QString& reject_reason);
        virtual void handleConnectionStarted();
        virtual void handleConnectionEstablished();
        virtual void handleConnectionShutdownStarted();
        virtual void handleConnectionShutdownCompleted();
        virtual void handleConnectionError(const QString& error);
        virtual void handleConnectionLost();
        virtual void handleConnectionHandshakeStarted();
        virtual void handleSessionEstablished();
        virtual std::string getNextMessageId();
        virtual void sendDownstreamMessage(const QJsonDocument& jdoc);
        virtual void readConfigFile();
   signals:
        void sendMessage(const QJsonDocument& jdoc);
   private:
        // setup functions
        void start();
        void setupGimmmStuff();
};

#endif // GIMMMAPPLICATION_H
