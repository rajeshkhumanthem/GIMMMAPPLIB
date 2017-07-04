#include "gimmmmessage.h"

#include <sstream>

#define THROW_INVALID_ARGUMENT_EXCEPTION(errmsg) \
  std::stringstream out; \
  out << "Exception @ [File:" << __FILE__ \
      << ", Line:" << __LINE__ << ", errormsg:" \
      << errmsg << "]"; throw std::invalid_argument(out.str().c_str())


Message::Message(MessageType type)
    :__messageType(type),
     __sequenceId(0)
{

}

Message::~Message()
{

}

std::string Message::getMessageTypeString(MessageType type)
{
    switch (type)
    {
        case MessageType::ACK:
            return std::string("ACK");
        case MessageType::DOWNSTREAM:
            return std::string("DOWNSTREAM");
        default:
            {
                std::stringstream err;
                err << "Invalid message type [" << (int)type << "]found.";
                THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
            }
    }
}

AckMessage::AckMessage(const SequenceId_t& seqid)
    :Message(MessageType::ACK)
{
      setSequenceId(seqid);
}

AckMessage::~AckMessage()
{

}

void AckMessage::validateAck() const
{
    if (getSequenceId() == 0)
    {
        std::stringstream err;
        err << "Invalid ACK message. 'sequence_id' field cannot be zero.";
        THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
    }
}

QJsonDocument AckMessage::toJson()
{
    validateAck();

    QJsonDocument ackmsg;
    QJsonObject root;
    root[msgfieldnames::SEQUENCE_ID] = getSequenceId();

    std::string msg_type = getMessageTypeString(getMessageType());
    root[msgfieldnames::MESSAGE_TYPE] = msg_type.c_str();
    ackmsg.setObject(root);
    return ackmsg;
}


/*!
 * \brief DataMessage::DataMessage
 */
DataMessage::DataMessage()
    : Message(MessageType::DOWNSTREAM),
     __timeToLive(10)
{
}

DataMessage::~DataMessage()
{

}

QJsonDocument DataMessage::toJson()
{
    validate();

    QJsonDocument msg;
    QJsonObject root;

    // pack gimmm header
    std::string msg_type = getMessageTypeString(getMessageType());
    root[msgfieldnames::MESSAGE_TYPE] = msg_type.c_str();
    root[msgfieldnames::GROUP_ID] = getGroupId().c_str();
    root[msgfieldnames::FCM_MESSAGE_ID] = getMessageId().c_str();

    // pack fcm data
    QJsonObject fcmdata;
    fcmdata[fcmfieldnames::TO] = __to.c_str();
    fcmdata[fcmfieldnames::MESSAGE_ID] = getMessageId().c_str();
    fcmdata[fcmfieldnames::TIME_TO_LIVE] = __timeToLive;

    if (!__collapseKey.empty())
        fcmdata[fcmfieldnames::COLLAPSE_KEY]= __collapseKey.c_str();
    if (!__priority.empty())
        fcmdata[fcmfieldnames::PRIORITY] = __priority.c_str();
    if (!__contentAvailable.isNull())
        fcmdata[fcmfieldnames::CONTENT_AVAILABLE] = __contentAvailable.toBool();
    if (!__mutableContent.isNull())
        fcmdata[fcmfieldnames::MUTABLE_CONTENT] = __mutableContent.toBool();
    if (!__deliveryRecieptRequested.isNull())
        fcmdata[fcmfieldnames::DELIVER_RECIEPT_REQUESTED] = __deliveryRecieptRequested.toBool();
    if (!__dryRun.isNull())
        fcmdata[fcmfieldnames::DRY_RUN] = __dryRun.toBool();
    if (!__data.empty())
        fcmdata[fcmfieldnames::DATA] = __data;


    root[msgfieldnames::FCM_DATA] = fcmdata;

    msg.setObject(root);
    return msg;
}


void DataMessage::validate()const
{
    //std::cout << "Validate downstream mesage" << std::endl;
    // validate gimmm header
    if (getMessageType() == MessageType::UNKNOWN)
    {
        std::stringstream err;
        err << "Invalid DOWNSTREAM message. Message type cannot be unknown.";
        THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
    }
    if (getMessageId().empty() == true)
    {
        std::stringstream err;
        err << "Invalid DOWNSTREAM message. 'message_id' field cannot be empty.";
        THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
    }
    if (__to.empty() && __condition.empty())
    {
        std::stringstream err;
        err << "Invalid DOWNSTREAM message. Both 'to' && 'condition' field cannot be empty.";
        THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
    }
    // max is 4 week as per FCM documentation.
    if (__timeToLive < MIN_TTL || __timeToLive > MAX_TTL )
    {
        std::stringstream err;
        err << "Invalid DOWNSTREAM message. Invalid TTL <" << __timeToLive << "> found.";
        THROW_INVALID_ARGUMENT_EXCEPTION(err.str());
    }
}

