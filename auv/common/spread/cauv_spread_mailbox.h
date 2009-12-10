#ifndef CAUV_SPREAD_MAILBOX_H_INCLUDED
#define CAUV_SPREAD_MAILBOX_H_INCLUDED

#include <string>
#include <ssrc/spread/Mailbox.h>
#include "cauv_spread_exceptions.h"
#include "cauv_spread_messages.h"
#include "../cauv_application_message.h"

enum MessagePriority {
    LOW = LOW_PRIORITY, MEDIUM = MEDIUM_PRIORITY, HIGH = HIGH_PRIORITY
};


/**
 * Timeout is a simple wrapper around Spread::sp_time, the %Spread C
 * API's struct that facilitates specifying connection
 * timeouts for the Mailbox constructor.  The constructor will convert
 * an integral number into a Timeout instance, allowing you to specify
 * timeouts to the Mailbox constructor as either Timeout instances or
 * a single number (interpreted as a number of seconds).
 */
class Timeout : public NS_SSRCSPREAD::Timeout {
public:
    /**
     * Converts a Spread::sp_time instance to a Timeout instance, copying
     * the stored time representation in the process.
     *
     * @param time The Spread::sp_time instance to convert.
     */
    Timeout(const Spread::sp_time time) : NS_SSRCSPREAD::Timeout(time) { }

    /**
     * Creates a Timeout instance representing a number of seconds
     * plus microseconds.
     *
     * @param sec The number of seconds.
     * @param usec The number of microseconds.
     */
    Timeout(const long sec = 0, const long usec = 0) : NS_SSRCSPREAD::Timeout(sec, usec) { }
};

/**
 * A SpreadMailbox object encapsulates a connection to a Spread daemon.
 * It connects upon successful creation and disconnects upon destruction.
 * You cannot reuse a SpreadMailbox to establish multiple connections in succession;
 * a new mailbox must be created for each daemon connection.
 * See the libssrcspread Mailbox class for more information.
 */
class SpreadMailbox {
public:
    static const Timeout ZERO_TIMEOUT;

    /**
     * @param portAndHost A string of the form "port", "port@domain_name", or "port@ip_address". A
     * zero-length string indicates that the default daemon should be
     * connected to ("4803" or "4803@localhost"; this is an undocumented
     * feature of SP_connect).
     * @param internalConnectionName The name to use for this mailbox connection internally. If
     * this does not matter to the application, NULL may be passed, and a name will be generated automatically.
     * This value is used to create the private group name.
     * @param shouldReceiveMembershipMessages Just as it sounds.
     * @param timeout A timeout for connecting to the daemon; 0 indicates that the
     * connection attempt should not time out.
     * @param priority The priority level for this connection. Currently ignored by Spread.
     */
    SpreadMailbox(const std::string &portAndHost, const std::string &internalConnectionName = "",
                  bool shouldReceiveMembershipMessages = true, const Timeout &timeout = ZERO_TIMEOUT,
                  MessagePriority priority = MEDIUM) throw(ConnectionError);
    virtual void disconnect() throw(InvalidSessionError);

    /**
     * @return The internal connection identifier assigned to this mailbox.
     */
    const std::string &getInternalName();

    /**
     * @return The unique group name assigned to this mailbox.
     */
    const std::string &getPrivateGroupName();

    virtual void joinGroup(const std::string &groupName)
        throw(ConnectionError, InvalidSessionError, IllegalGroupError);
    virtual void leaveGroup(const std::string &groupName)
        throw(ConnectionError, InvalidSessionError, IllegalGroupError);

    /**
     * @return The number of bytes sent
     */
    virtual int sendMessage(ApplicationMessage &message, Spread::service serviceType)
        throw(InvalidSessionError, ConnectionError, IllegalMessageError);

    /**
     * @return The number of bytes sent
     */
    virtual int sendMultigroupMessage(ApplicationMessage &message, Spread::service serviceType)
        throw(InvalidSessionError, ConnectionError, IllegalMessageError);

    /**
     * Blocks until a message comes in from the Spread daemon. The received messsage may
     * be anything in the RegularMessage or MembershipMessage hierarchies.
     * @return An object containing the received message and associated metadata.
     */
    virtual SpreadMessage receiveMessage() throw(InvalidSessionError, ConnectionError, IllegalMessageError);

    /**
     * Similar to receiveMessage(), but the message is a group of submessages, allowing for
     * use in the scatter/gather paradigm. See the libssrcspread documentation. Not currently implemented.
     * @return An object containing the received message and associated metadata.
     */
    virtual SpreadMessage receiveScatterMessage() throw(InvalidSessionError, ConnectionError, IllegalMessageError);

    int waitingMessageByteCount() throw(InvalidSessionError, ConnectionError);
};

/**
 * A ReconnectingSpreadMailbox automatically handles disconnection problems by trying
  * to reconnect, retrying whatever operation failed due to disconnection.
 */
class ReconnectingSpreadMailbox : public SpreadMailbox {
public:
    ReconnectingSpreadMailbox(const std::string &portAndHost, const std::string &privateConnectionName = "",
                  bool shouldReceiveMembershipMessages = true, const Timeout &timeout = ZERO_TIMEOUT,
                  MessagePriority priority = MEDIUM) throw(ConnectionError)
        : SpreadMailbox(portAndHost, privateConnectionName, shouldReceiveMembershipMessages,
                        timeout, priority) {}
};

#endif // CAUV_SPREAD_MAILBOX_H_INCLUDED
