#include "gimmmapplication.h"
#include "gimmmmessage.h"

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
    connect(&__gimmmConn, SIGNAL(connectionStarted(int, int)),     this, SLOT(handleConnectionStarted(int, int)));
    connect(&__gimmmConn, SIGNAL(connectionEstablished()),         this, SLOT(handleConnectionEstablished()));
    connect(&__gimmmConn, SIGNAL(connectionHandshakeStarted()),    this, SLOT(handleConnectionHandshakeStarted()));
    connect(&__gimmmConn, SIGNAL(sessionEstablished()),            this, SLOT(handleSessionEstablished()));
    connect(&__gimmmConn, SIGNAL(connectionShutdownStarted()),     this, SLOT(handleConnectionShutdownStarted()));
    connect(&__gimmmConn, SIGNAL(connectionShutdownCompleted()),   this, SLOT(handleConnectionShutdownCompleted()));
    connect(&__gimmmConn, SIGNAL(connectionLost()),                this, SLOT(handleConnectionLost()));
    connect(&__gimmmConn, SIGNAL(connectionError(int, const QString&)), this, SLOT(handleConnectionError(int, const QString&)));
    connect(&__gimmmConn, SIGNAL(newUpstreamMessage(const QJsonDocument&)),this, SLOT(newUpstreamMessage(const QJsonDocument&)));
    connect(&__gimmmConn, SIGNAL(newDownstreamAckMessage(const QJsonDocument&)),this, SLOT(newDownstreamAckMessage(const QJsonDocument&)));
    connect(&__gimmmConn, SIGNAL(newDownstreamRejectMessage(const QJsonDocument&)), this, SLOT(newDownstreamRejectMessage(const QJsonDocument&)));
    connect(&__gimmmConn, SIGNAL(newDownstreamReceiptMessage(const QJsonDocument&)),this, SLOT(newDownstreamReceiptMessage(const QJsonDocument&)));

    //setup queued connection to fcmconn for sending downstream message to FCM.
    connect(this, SIGNAL(sendMessage(const QJsonDocument&)),&__gimmmConn, SLOT(handleSendMessage(const QJsonDocument&)));

    __gimmmConn.connectToServer(__hostAddress, __hostPortNo, __sessionId);
}

void GimmmApplication::sendDownstreamMessage(const QJsonDocument &jdoc)
{
    emit sendMessage(jdoc);
}

void GimmmApplication::sendAckMessage(const QJsonDocument& orig_msg)
{
    SequenceId_t seqid = orig_msg.object().value(msgfieldnames::SEQUENCE_ID).toInt();

    AckMessage ackmsg(seqid);
    QJsonDocument jdoc = ackmsg.toJson();

    emit sendMessage(jdoc);
}

void GimmmApplication::handleConnectionEstablished()
{
}


void GimmmApplication::handleConnectionHandshakeStarted()
{
}


void GimmmApplication::handleConnectionShutdownStarted()
{
}


void GimmmApplication::handleConnectionShutdownCompleted()
{
}

void GimmmApplication::handleNewUpstreamMessage(const QJsonDocument &json)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    PRINT_JSON_DOC(std::cout, json);
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleNewDownstreamAckMessage(const QJsonDocument &json)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    PRINT_JSON_DOC(std::cout, json);
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleNewDownstreamRejectMessage(const QJsonDocument &json, const QString &reject_reason)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    PRINT_JSON_DOC(std::cout, json);
    std::cout << reject_reason.toStdString() << std::endl;
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleNewDownstreamReceiptMessage(const QJsonDocument &json)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    PRINT_JSON_DOC(std::cout, json);
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleConnectionStarted(int attemptno, int waittime)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    std::cout << "Connecting to GIMMM server, connection attempt#:" << attemptno
              << ", Will connect after Waittime(msec):" << waittime << std::endl;
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleConnectionError(int, const QString& error)
{
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
    std::cout << "ERROR: Connection error:" << error.toStdString() << std::endl;
    std::cout << "--------------------------------------------------------------------------------------------------"  << std::endl;
}

void GimmmApplication::handleConnectionLost()
{

}

void GimmmApplication::handleSessionEstablished()
{

}

void GimmmApplication::newUpstreamMessage(const QJsonDocument& bal_msg)
{
    QJsonDocument upstream;
    QJsonObject root = bal_msg.object().value(msgfieldnames::FCM_DATA).toObject();
    upstream.setObject(root);
    handleNewUpstreamMessage(upstream);

    sendAckMessage(bal_msg);
}

void GimmmApplication::newDownstreamAckMessage(const QJsonDocument& bal_msg)
{
    QJsonDocument ack_msg;
    QJsonObject root = bal_msg.object().value(msgfieldnames::FCM_DATA).toObject();
    ack_msg.setObject(root);
    handleNewDownstreamAckMessage(ack_msg);

    sendAckMessage(bal_msg);
}


void GimmmApplication::newDownstreamRejectMessage(
    const QJsonDocument& bal_msg)
{
    QJsonDocument orig_msg;
    QJsonObject root = bal_msg.object().value(msgfieldnames::FCM_DATA).toObject();
    orig_msg.setObject(root);

    QString reject_reason = bal_msg.object().value(msgfieldnames::ERROR_DESC).toString();

    handleNewDownstreamRejectMessage(orig_msg, reject_reason);
    sendAckMessage(bal_msg);
}

void GimmmApplication::newDownstreamReceiptMessage(
    const QJsonDocument& bal_msg)
{
    QJsonDocument recpt_msg;
    QJsonObject root = bal_msg.object().value(msgfieldnames::FCM_DATA).toObject();
    recpt_msg.setObject(root);

    handleNewDownstreamReceiptMessage(recpt_msg);
    sendAckMessage(bal_msg);
}
