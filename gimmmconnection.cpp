#include "gimmmconnection.h"
#include "macros.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QTimer>

#include <iostream>
#include <sstream>

GimmmConnection::GimmmConnection()
      :__connectAttempt(0),
       __connectWaitTime(5000)
{
    __in.setVersion(QDataStream::Qt_5_8);
    connect( &__socket, &QAbstractSocket::connected, this, &GimmmConnection::handleConnected, Qt::DirectConnection);
    connect( &__socket, &QAbstractSocket::disconnected, this, &GimmmConnection::handleDisconnected, Qt::DirectConnection);
    connect( &__socket, &QAbstractSocket::readyRead, this, &GimmmConnection::handleReadyRead, Qt::DirectConnection);
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

/*!
 * \brief GimmmConnection::tryConnect
 */
void GimmmConnection::tryConnect()
{
    if (__socket.state() == QAbstractSocket::ConnectedState)
    {
        __connectAttempt = 0;
        return;
    }
    emit connectionStarted();
    __connectAttempt++;
    __socket.connectToHost(__address, __port);
    QTimer::singleShot(__connectWaitTime, this, &GimmmConnection::tryConnect);
}


/*!
 * \brief GimmmConnection::handleConnected
 */
void GimmmConnection::handleConnected()
{
    emit connectionEstablished();

    QJsonDocument jdoc;
    QJsonObject root;
    root["message_type"]= "LOGON";
    root["session_id"] = __sessionId.c_str();
    jdoc.setObject(root);
    PRINT_JSON_DOC(std::cout, jdoc);

    emit connectionHandshakeStarted();
    sendMessage(jdoc);
}

/*!
 * \brief GimmmConnection::handleDisconnected
 */
void GimmmConnection::handleDisconnected()
{
    emit connectionLost();
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

    QByteArray bytes;
    __in.startTransaction();
    __in >> bytes;
    if (!__in.commitTransaction())
        return;

    std::cout<< "-----------------------START NEW PHANTOM Message----------------------\n" ;
    try
    {
        QJsonDocument jsondoc = QJsonDocument::fromBinaryData(bytes);
        PRINT_JSON_DOC(std::cout, jsondoc);
        handleNewMessage(jsondoc);
    }
    catch(std::exception& err)
    {
        PRINT_EXCEPTION_STRING(std::cout, err) << std::endl;
    }

    std::cout<< "-----------------------END NEW PHANTOM Message --------------------" << std::endl;
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
        handleUpstreamMessage(jdoc);
    }
    else if (msgtype == "DOWNSTREAM_REJECT")
    {
        handleDownstreamRejectMessage(jdoc);
    }
}


/*!
 * \brief GimmmConnection::handleLogonResponseMessage
 * \param jdoc
 */
void GimmmConnection::handleLogonResponseMessage(
        const QJsonDocument& jdoc)
{
    std::cout << "Processing phantom logon reponse..." << std::endl;
    std::string status = jdoc.object().value("status").toString().toStdString();
    if (status == "SUCCESS")
    {
        emit sessionEstablished();
    }else
    {
        QString reject_reason = jdoc.object().value("error_description").toString();
        emit connectionError(reject_reason);
    }
}


/*!
 * \brief GimmmConnection::handleUpstreamMessage
 * \param msg
 */
void GimmmConnection::handleUpstreamMessage(const QJsonDocument& msg)
{
  QJsonDocument upstream;
  QJsonObject root = msg.object().value("upstream_data").toObject();
  upstream.setObject(root);

  emit newUpstreamMessage(upstream);
}


/*!
 * \brief GimmmConnection::handleDownstreamRejectMessage
 * \param jdoc
 */
void GimmmConnection::handleDownstreamRejectMessage(
        const QJsonDocument& jdoc)
{
    QString reject_reason = jdoc.object().value("error_description").toString();
    emit newDownstreamRejectMessage(jdoc, reject_reason);
}

/*!
 * \brief GimmmConnection::handleError
 * \param error
 */
void GimmmConnection::handleError(QAbstractSocket::SocketError error)
{
    std::cout << "Seabus messaging socket error:" << error << std::endl;
    emit connectionError(__socket.errorString());
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
    // TODO remove this check so that error is pass backed to the application.
    if (__socket.isValid())
    {
        QByteArray m;
        QDataStream out(&m, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_8);
        out << msg.toBinaryData();

        //TODO delete later
        std::cout << "Attempting to write json message of length <" << m.length() <<  "> bytes." << std::endl;
        qint64 bytes = __socket.write(m);
    }
}
