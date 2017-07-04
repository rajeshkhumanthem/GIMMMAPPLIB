#include "gimmmconnection.h"
#include "exponentialbackoff.h"
#include "gimmmmessage.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include <iostream>
#include <sstream>



#define THROW_INVALID_ARGUMENT_EXCEPTION(errmsg) \
  std::stringstream out; \
  out << "Exception @ [File:" << __FILE__ \
      << ", Line:" << __LINE__ << ", errormsg:" \
      << errmsg << "]"; throw std::invalid_argument(out.str().c_str())

#define PRINT_EXCEPTION_STRING(out, ex) \
  out << "Exception caught @ [FILE:" << __FILE__ \
      << ", Line:" << __LINE__ << "], What:" \
  << ex.what() << std::endl

#define PRINT_JSON_DOC(out, jsondoc) \
    QString str = jsondoc.toJson(QJsonDocument::Indented); \
    out << str.toStdString() << std::endl;

#define PRINT_JSON_DOC_RAW(out, jsondoc) \
    QString str = jsondoc.toJson(QJsonDocument::Compact); \
    out << str.toStdString() << std::endl;


/*!
 * \brief GimmmConnection::GimmmConnection
 */
GimmmConnection::GimmmConnection(int max_retry)
      :__connectAttempt(0),
        __maxRetry(max_retry)
{
    __in.setVersion(QDataStream::Qt_5_8);
    connect( &__socket, &QAbstractSocket::connected,
             this, &GimmmConnection::handleConnected, Qt::DirectConnection);
    connect( &__socket, &QAbstractSocket::disconnected,
             this, &GimmmConnection::handleDisconnected, Qt::DirectConnection);
    connect( &__socket, &QAbstractSocket::readyRead,
             this, &GimmmConnection::handleReadyRead, Qt::DirectConnection);
    connect( &__socket, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
             this, &GimmmConnection::handleError, Qt::DirectConnection);
}


/*!
 * \brief GimmmConnection::~GimmmConnection
 */
GimmmConnection::~GimmmConnection()
{
}


/*!
 * \brief GimmmConnection::connectToServer
 * \param add
 * \param port
 * \param session_id
 */
void GimmmConnection::connectToServer(
                     const QHostAddress& add,
                     quint16 port,
                     const std::string& session_id)
{
    __address = add;
    __port    = port;
    __sessionId = session_id;

    tryConnect();
}


void GimmmConnection::tryConnect()
{

    __socket.connectToHost(__address, __port);
    if ( __socket.waitForConnected())
    {
        emit connectionEstablished();
        __connectAttempt = 0;
        __exBackOff.resetRetry();
        return;
    }
    retryConnectWithBackoff();
}

/*!
 * \brief GimmmConnection::tryConnect
 */
void GimmmConnection::retryConnectWithBackoff()
{
    __connectAttempt++;
    if(__connectAttempt > __maxRetry)
    {
        std::stringstream err;
        err << "Max connection attempt[" << __maxRetry
            <<"] reached. Cannot connect to the server. Goodbye!";
        emit connectionError(-1, err.str().c_str());
        exit(0);
    }
    int waittime = __exBackOff.next();
    emit connectionStarted(__connectAttempt, waittime);
    QTimer::singleShot(waittime, this, &GimmmConnection::tryConnect);
}


/*!
 * \brief GimmmConnection::handleConnected
 */
void GimmmConnection::handleConnected()
{
    QJsonDocument jdoc;
    QJsonObject root;
    root[msgfieldnames::MESSAGE_TYPE]= "LOGON";
    root[msgfieldnames::SESSION_ID] = __sessionId.c_str();
    jdoc.setObject(root);

    //PRINT_JSON_DOC(std::cout, jdoc);
    emit connectionHandshakeStarted();
    sendMessage(jdoc);
}


/*!
 * \brief GimmmConnection::handleDisconnected
 */
void GimmmConnection::handleDisconnected()
{
    emit connectionLost();
    //try reconnecting right away.
    tryConnect();
}


/*!
 * \brief GimmmConnection::handleReadyRead
 */
void GimmmConnection::handleReadyRead()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    __in.setDevice(0); // unset the current device.
    __in.setDevice(socket);

    while (socket->bytesAvailable())
    {
        QByteArray bytes;
        __in.startTransaction();
        __in >> bytes;
        if (!__in.commitTransaction())
            return;

        //std::cout<< "===============================START NEW GIMMM MESSAGE===========================================\n" ;
        //std::cout << "read [" << bytes.size() << "]" << std::endl;
        try
        {
            QJsonDocument jsondoc = QJsonDocument::fromBinaryData(bytes);
            //PRINT_JSON_DOC(std::cout, jsondoc);
            handleNewMessage(jsondoc);
        }
        catch(std::exception& err)
        {
            PRINT_EXCEPTION_STRING(std::cout, err) << std::endl;
        }
        //std::cout<< "===============================END NEW GIMMM MESSAGE=============================================" << std::endl;
    }
}


/*!
 * \brief GimmmConnection::handleNewMessage
 * \param jdoc
 */
void GimmmConnection::handleNewMessage(
        const QJsonDocument& jdoc)
{
    std::string msgtype = jdoc.object().value("message_type").toString().toStdString();
    if ( msgtype == "LOGON_RESPONSE")
    {
        handleLogonResponseMessage(jdoc);
    }
    else if ( msgtype == "UPSTREAM")
    {
        emit newUpstreamMessage(jdoc);
    }
    else if (msgtype == "DOWNSTREAM_REJECT")
    {
        emit newDownstreamRejectMessage(jdoc);
    }
    else if (msgtype == "DOWNSTREAM_ACK")
    {
        emit newDownstreamAckMessage(jdoc);
    }
    else if (msgtype == "DOWNSTREAM_RECEIPT")
    {
        emit newDownstreamReceiptMessage(jdoc);
    }
}


/*!
 * \brief GimmmConnection::handleLogonResponseMessage
 * \param jdoc
 */
void GimmmConnection::handleLogonResponseMessage(
        const QJsonDocument& jdoc)
{
    std::string status = jdoc.object().value("status").toString().toStdString();
    if (status == "SUCCESS")
    {
        emit sessionEstablished();
    }else
    {
        QString reject_reason = jdoc.object().value(msgfieldnames::ERROR_DESC).toString();
        emit connectionError(-1, reject_reason);
    }
}

/*!
 * \brief GimmmConnection::handleError
 * \param error
 */
void GimmmConnection::handleError(QAbstractSocket::SocketError error)
{
    //emit a human readable string instead of the enum.
    emit connectionError((int)error, __socket.errorString());
}

/*!
 * \brief GimmmConnection::handleSendMessage
 * \param msg
 */
void GimmmConnection::handleSendMessage(
        const QJsonDocument& msg)
{
  sendMessage(msg);
}

/*!
 * \brief GimmmConnection::sendMessage
 * \param msg
 */
void GimmmConnection::sendMessage(
        const QJsonDocument& msg)
{
    if (__socket.isValid())
    {
        QByteArray m;
        QDataStream out(&m, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_8);
        out << msg.toBinaryData();

        __socket.write(m);
    }
}
