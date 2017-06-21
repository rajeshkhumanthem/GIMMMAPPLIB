#include "gimmmapplication.h"

#include <iostream>
#include <sstream>

#include <QSettings>
#include <QFileInfo>

#define PRINT_JSON_DOC(out, jsondoc) \
    QString str = jsondoc.toJson(QJsonDocument::Indented); \
    out << str.toStdString() << std::endl;


GimmmApplication::GimmmApplication()
{
    start();
}


void GimmmApplication::start()
{
    readConfigFile();
    setupGimmmStuff();
}


/*!
 * \brief GimmmGimmmApplication::readConfigFile
 */
void GimmmApplication::readConfigFile()
{
    std::cout << "Reading config file..." << std::endl;
    QString filename;

    if (QFileInfo::exists("./config.ini"))
    {
        filename = "./config.ini";
    }else
    {
        std::cout << "ERROR. Configuration file 'config.ini' missing. Please do the following:\n"
                  << "1) Copy 'config.copy.ini' as 'config.ini' into the same dir as the executable.\n"
                  << "2) Update 'config.ini' with the appropriate credential and try again. Exiting..." << std::endl;
        exit(0);
    }
    __ini = new QSettings(filename, QSettings::IniFormat);


    //phantom port no.
    __hostPortNo = __ini->value("GIMMMSERVER_SECTION/port_no", 0).toInt();
    if (__hostPortNo == 0)
    {
        std::cout << "ERROR: Missing config parameter 'GIMMMSERVER_SECTION/port_no. Exiting..." << std::endl;
        exit(0);
    }
    // phantom host addr
    QString addr = __ini->value("GIMMMSERVER_SECTION/host_address", "NULL").toString();
    if ( addr == "NULL")
    {
        std::cout << "ERROR: Missing config parameter 'GIMMMSERVER_SECTION/host_address. Exiting..." << std::endl;
        exit(0);
    }
    if (!__hostAddress.setAddress(addr))
    {
        std::cout << "ERROR: Unable to parse GIMMMSERVER_SECTION/host_address. Exiting..." << std::endl;
        exit(0);
    }

    // session id aka bundle identifier for ios app.
    __sessionId = __ini->value("CLIENT_SECTION/session_id", "NULL").toString().toStdString();
    if ( __sessionId == "NULL")
    {
        std::cout<< "ERROR: No session id found. Exiting..." << std::endl;
        exit(0);
    }
    __currentMessageId = __ini->value("APP_SECTION/message_sequence_no", 0).toInt();
}


std::string GimmmApplication::getNextMessageId()
{
    std::int64_t nextid = ++__currentMessageId;
    __ini->setValue("APP_SECTION/message_sequence_no",  nextid);
    __ini->sync();

    std::stringstream next;
    next << nextid;
    return next.str();
}

void GimmmApplication::setupGimmmStuff()
{
    // setup signal handlers
    connect(&__gimmmConn, SIGNAL(connectionStarted()),             this, SLOT(handleConnectionStarted()));
    connect(&__gimmmConn, SIGNAL(connectionEstablished()),         this, SLOT(handleConnectionEstablished()));
    connect(&__gimmmConn, SIGNAL(connectionHandshakeStarted()),    this, SLOT(handleConnectionHandshakeStarted()));
    connect(&__gimmmConn, SIGNAL(sessionEstablished()),            this, SLOT(handleSessionEstablished()));
    connect(&__gimmmConn, SIGNAL(connectionShutdownStarted()),     this, SLOT(handleConnectionShutdownStarted()));
    connect(&__gimmmConn, SIGNAL(connectionShutdownCompleted()),   this, SLOT(handleConnectionShutdownCompleted()));
    connect(&__gimmmConn, SIGNAL(connectionLost()),                this, SLOT(handleConnectionLost()));
    connect(&__gimmmConn, SIGNAL(connectionError(const QString&)), this, SLOT(handleConnectionError(const QString&)));
    connect(&__gimmmConn, SIGNAL(newUpstreamMessage(const QJsonDocument&)),this, SLOT(handleNewUpstreamMessage(const QJsonDocument&)));

    connect(&__gimmmConn, SIGNAL(newDownstreamRejectMessage(const QJsonDocument&, const QString&)),
            this, SLOT(handleNewDownstreamRejectMessage(const QJsonDocument&, const QString&)));

    //setup queued connection to fcmconn for sending downstream message to FCM.
    connect(this, SIGNAL(sendMessage(const QJsonDocument&)),&__gimmmConn, SLOT(handleSendMessage(const QJsonDocument&)));

    __gimmmConn.connectToServer(__hostAddress, __hostPortNo, __sessionId);
}

void GimmmApplication::sendDownstreamMessage(const QJsonDocument &jdoc)
{
    emit sendMessage(jdoc);
}

void GimmmApplication::handleConnectionStarted()
{
}


void GimmmApplication::handleConnectionEstablished()
{
}


void GimmmApplication::handleConnectionHandshakeStarted()
{
}


void GimmmApplication::handleSessionEstablished()
{
}


void GimmmApplication::handleConnectionLost()
{
}


void GimmmApplication::handleConnectionShutdownStarted()
{
}


void GimmmApplication::handleConnectionShutdownCompleted()
{
}


void GimmmApplication::handleConnectionError(const QString& error)
{
}


void GimmmApplication::handleNewUpstreamMessage(const QJsonDocument& client_msg)
{
}


void GimmmApplication::handleNewDownstreamRejectMessage(
    const QJsonDocument& client_msg,
    const QString& reject_reason)
{
}
