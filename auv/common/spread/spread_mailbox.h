#ifndef CAUV_SPREAD_MAILBOX_H_INCLUDED
#define CAUV_SPREAD_MAILBOX_H_INCLUDED

#include <string>
#include <vector>
#include <ssrc/spread/Mailbox.h>
#include <boost/shared_ptr.hpp>

#include <common/messages.h>

#include "spread_exceptions.h"
#include "spread_messages.h"

/**
 * ConnectionTimeout is a simple wrapper around Spread::sp_time, the %Spread C
 * API's struct that facilitates specifying connection
 * timeouts for the Mailbox constructor.  The constructor will convert
 * an integral number into a ConnectionTimeout instance, allowing you to specify
 * timeouts to the Mailbox constructor as either ConnectionTimeout instances or
 * a single number (interpreted as a number of seconds).
 */
class ConnectionTimeout : public ssrc::spread::Timeout {
public:
    /**
     * Converts a Spread::sp_time instance to a Timeout instance, copying
     * the stored time representation in the process.
     *
     * @param time The Spread::sp_time instance to convert.
     */
    ConnectionTimeout(const Spread::sp_time time) : ssrc::spread::Timeout(time) { }

    /**
     * Creates a Timeout instance representing a number of seconds
     * plus microseconds.
     *
     * @param sec The number of seconds.
     * @param usec The number of microseconds.
     */
    ConnectionTimeout(const long sec = 0, const long usec = 0) : ssrc::spread::Timeout(sec, usec) { }
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
    static const ConnectionTimeout ZERO_TIMEOUT;
    friend bool operator!(const SpreadMailbox &mailbox) { return mailbox.m_ssrcMailbox->killed(); }

    enum MailboxPriority {
        LOW = LOW_PRIORITY, MEDIUM = MEDIUM_PRIORITY, HIGH = HIGH_PRIORITY
    };


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
    virtual void connect(const std::string &portAndHost, const std::string &internalConnectionName = "",
                         const bool shouldReceiveMembershipMessages = true,
                         const ConnectionTimeout &timeout = ZERO_TIMEOUT,
                         const MailboxPriority priority = MEDIUM);
    virtual void disconnect();

    /**
     * @return The internal connection identifier assigned to this mailbox.
     */
    const std::string &getInternalName() const { return m_ssrcMailbox->name(); }

    /**
     * @return The unique group name assigned to this mailbox.
     */
    const std::string &getPrivateGroupName() const {return m_ssrcMailbox->private_group(); }

    virtual void joinGroup(const std::string &groupName);
    virtual void leaveGroup(const std::string &groupName);

    /**
     * @return The number of bytes sent
     */
    int sendMessage(const boost::shared_ptr<const Message> message, Spread::service serviceType);
    virtual int sendMessage(boost::shared_ptr<const Message> message, Spread::service serviceType, const std::string &destinationGroup);

    /**
     * @return The number of bytes sent
     */
    virtual int sendMultigroupMessage(boost::shared_ptr<const Message> message, Spread::service serviceType,
        const std::vector<std::string> &groupNames);

    /**
     * Blocks until a message comes in from the Spread daemon. The received messsage may
     * be anything in the RegularMessage or MembershipMessage hierarchies.
     * @return An object containing the received message and associated metadata.
     */
    virtual boost::shared_ptr<SpreadMessage> receiveMessage();

    int waitingMessageByteCount() const;
    bool isConnected() const { return !m_ssrcMailbox->killed(); }

protected:
    boost::shared_ptr< ssrc::spread::Mailbox > m_ssrcMailbox;

private:
    int doSendMessage(boost::shared_ptr<const Message> message, Spread::service serviceType,
        boost::shared_ptr<ssrc::spread::GroupList> const groupNames );

    void handleSpreadError(ssrc::spread::Error& e);
};


#endif // CAUV_SPREAD_MAILBOX_H_INCLUDED