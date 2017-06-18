#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <cstdint>

#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>

#define MAX_TTL                         2419200 // Max time to live in secs(28 days)
#define MIN_TTL                         0

typedef std::uint64_t MessageId_t;
typedef std::string   GroupId_t;


namespace msgfieldnames
{
  static const char* const MESSAGE_TYPE     = "message_type";
  static const char* const MESSAGE_ID       = "message_id";
  static const char* const GROUP_ID         = "group_id";
  static const char* const SESSION_ID       = "session_id";
  static const char* const STATUS           = "status";
  static const char* const ERROR            = "error";
  static const char* const ERROR_DESC       = "error_description";
  static const char* const TIME_TO_LIVE     = "time_to_live";
  static const char* const FCM_DATA         = "fcm_data";
}



/*!
 * field names in the root JSON message inside a xmpp stanza
 */
namespace fcmfieldnames
{
  // target
  static const char* const TO                           = "to";
  static const char* const CONDITION                    = "condition";
  // options
  static const char* const MESSAGE_ID                   = "message_id";
  static const char* const COLLAPSE_KEY                 = "collapse_key";
  static const char* const PRIORITY                     = "priority";
  static const char* const CONTENT_AVAILABLE            = "content_available";
  static const char* const MUTABLE_CONTENT              = "mutable_content";
  static const char* const TIME_TO_LIVE                 = "time_to_live";
  static const char* const DELIVER_RECIEPT_REQUESTED    = "delivery_receipt_requested";
  static const char* const DRY_RUN                      = "dry_run";
  // payload
  static const char* const DATA                         = "data";
  static const char* const NOTIFICATION                 = "notification";
  // TODO add notification fields

  // downstream response
  static const char* const FROM                         = "from"; // who sent; token
  static const char* const MESSAGE_TYPE                 = "message_type"; // ack/nack
  static const char* const REGISTRATION_ID              = "registration_id";
  static const char* const ERROR                        = "error";
  static const char* const ERROR_DESC                   = "error_description";

  // upstream message
  static const char* const CATEGORY                     = "category";
  //static const char* const CONTROL_TYPE               = "control_type";
}


enum class MessageType: char
{
    UNKNOWN = 0,
    LOGON = 1,            // BAL --> GIMMM
    LOGON_RESPONSE,       // GIMMM --> BAL
    ACK = 2,              // BAL --> GIMMM
    UPSTREAM = 3,         // GIMMM --> BAL
    DOWNSTREAM = 4,       // BAL --> GIMMM
    DOWNSTREAM_ACK = 5,   // GIMMM --> BAL
    DOWNSTREAM_REJECT = 6 // GIMMM --> BAL
};


enum class PayloadType: int
{
    UNKNOWN = 0,
    DATA = 1,
    NOTIFICATION = 2
};

class Message
{
        MessageType         __messageType;
        // start GIMMM header
        GroupId_t           __groupId;
        std::string         __messageId;
        // End GIMMM header
    public:
        virtual QJsonDocument toJson() = 0;
        Message(MessageType type);
        virtual ~Message();

        MessageType             getMessageType() const { return __messageType;}
        const std::string&      getMessageId() const { return __messageId;}
        const GroupId_t&        getGroupId() const { return __groupId;}

        void setMessageType(MessageType type) { __messageType = type;}
        void setMessageId(const std::string& message_id) { __messageId = message_id;}
        void setGroupId(const std::string& group_id) { __groupId = group_id;}
};

class AckMessage: public Message
{
    public:
       AckMessage();
       virtual ~AckMessage();
       virtual QJsonDocument toJson();
    private:
       void validateAck()const;
};


/*!
 * \brief The Message class
 *        Convenient class to create 'Downstream XMPP message(JSON)'.
 *        Consists of 3 parts; Target, Options & Data.
 *        See https://firebase.google.com/docs/cloud-messaging/xmpp-server-ref for details.
 */
class DataMessage: public Message
{
    // Target
    std::string         __to;
            // or
    std::string         __condition;

    //Options
    std::string         __collapseKey;
    std::string         __priority;
    QVariant            __contentAvailable;
    QVariant            __mutableContent;
    std::int32_t        __timeToLive;
    QVariant            __deliveryRecieptRequested;
    QVariant            __dryRun;

    //Payload
    QJsonObject         __data;

public:
    DataMessage();
    virtual ~DataMessage();
    virtual QJsonDocument toJson(); // throw std::exception
    void setTo(const std::string& to) { __to = to;}
    void setTo(const char* to) { __to = to;}
    void setCondition(const std::string& condition){ __condition = condition;}
    void setCollapseKey( const std::string& collapse_key) { __collapseKey = collapse_key;}
    void setpriority(const std::string& priority) { __priority = priority;}
    void setContentAvailable(bool content_available) { __contentAvailable = content_available;}
    void setTimeToLive (std::int32_t ttl) { __timeToLive = ttl;}
    void setDeliveryRecieptRequested(bool dcr) { __deliveryRecieptRequested = dcr;}
    void setDryRun (bool dry_run) { __dryRun = dry_run;}
    void setData(const QJsonObject& data) { __data = data;}
    /*!
     * \brief validate
     *        Call to validate the current message.
     *        Throws 'std::invalid_argument' exception on failure.
     */
    void validate()const;

    const std::string& getTo()const { return __to;}
    const std::string& getCondition() const { return __condition;}

    // OPTIONAL FIELDS
    /*!
     * \brief getCollapseKey: Optional Field.
     * \return
     */
    const std::string& getCollapseKey() const { return __collapseKey;}
    /*!
     * \brief getPriority: Optional Field.
     * \return
     */
    const std::string& getPriority() const { return __priority;}
    /*!
     * \brief getTimeToLive: Optional Field.
     * \return
     */
    std::int32_t       getTimeToLive() const { return __timeToLive;}
    /*!
     * \brief getContentAvailable: Optional Field.
     *          Possible value is NULL/true/false.
     *          User should check for NULL before using it.
     * \return: Returns a QVariant which could be NULL since this is an optional field.
     */
    QVariant           getContentAvailable() const { return __contentAvailable;}
    /*!
     * \brief getMutableContent: Optional Field.
     *          Possible value is NULL/true/false.
     *          User should check for NULL before using it.
     * \return: Returns a QVariant which could be NULL since this is an optional field.
     */
    QVariant           getMutableContent() const { return __mutableContent;}
    /*!
     * \brief getDeliveryRecieptRequested: Optional Field.
     *          Possible value is NULL/true/false.
     *          User should check for NULL before using it.
     * \return: Returns a QVariant which could be NULL since this is an optional field.
     */
    QVariant           getDeliveryRecieptRequested() const { return __deliveryRecieptRequested;}
    /*!
     * \brief getDryRun: Optional Field.
     *          Possible value is NULL/true/false.
     *          User should check for NULL before using it.
     * \return: Returns a QVariant which could be NULL since this is an optional field.
     */
    QVariant           getDryRun()const { return __dryRun;}
private:
    void validateAck()const;
    void validateDownstream()const;
};


#endif // MESSAGE_H
