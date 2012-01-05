/* Copyright 2011 Cambridge Hydronautics Ltd.
 *
 * Cambridge Hydronautics Ltd. licenses this software to the CAUV student
 * society for all purposes other than publication of this source code.
 * 
 * See license.txt for details.
 * 
 * Please direct queries to the officers of Cambridge Hydronautics:
 *     James Crosby    james@camhydro.co.uk
 *     Andy Pritchard   andy@camhydro.co.uk
 *     Leszek Swirski leszek@camhydro.co.uk
 *     Hugo Vincent     hugo@camhydro.co.uk
 */

#ifndef CAUV_SPREAD_RC_MAILBOX_H_INCLUDED
#define CAUV_SPREAD_RC_MAILBOX_H_INCLUDED

#include <iostream>
#include <string>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

#include <common/mailbox.h>
#include "spread_mailbox.h"

namespace cauv{

/**
 * A ReconnectingSpreadMailbox automatically handles disconnection problems by trying
 * to reconnect, retrying whatever operation failed due to disconnection.
 * TODO: doesn't retry anything other than connect at the moment. This may be
 * the behaviour we want.
 */

class ReconnectingSpreadMailbox: public Mailbox, boost::noncopyable {
    typedef boost::recursive_mutex mutex_t;
    typedef boost::lock_guard<mutex_t> lock_t;
public:
    ReconnectingSpreadMailbox();
    ~ReconnectingSpreadMailbox();
    
    virtual void connect(std::string const& portAndHost,
                         std::string const& privConnectionName = "",
                         bool recvMembershipMessages = true,
                         ConnectionTimeout const& timeout = SpreadMailbox::ZERO_TIMEOUT,
                         SpreadMailbox::MailboxPriority priority = SpreadMailbox::MEDIUM);
    
    /* should a thread be started in order to connect (ut=true), or should all
     * connection attempts be synchronous
     */
    void useThreads(bool ut){
        m_no_threads = !ut;
    }

    /**
     * @return The internal connection identifier assigned to this mailbox.
     */
    std::string getInternalName();
    
    /**
     * @return The unique group name assigned to this mailbox.
     */
    std::string getPrivateGroupName();
    
    virtual void joinGroup(const std::string &groupName);
    virtual void leaveGroup(const std::string &groupName);
    
    /**
     * @return The number of bytes sent
     */
    virtual int sendMessage(boost::shared_ptr<const Message> message, messageReliability reliability);

    /**
     * @return The number of bytes sent
     */
    virtual int sendMessage(boost::shared_ptr<const Message> message, messageReliability reliability,
                    const std::string &destinationGroup);
    
    /**
     * @return The number of bytes sent
     */
    virtual int sendMultigroupMessage(boost::shared_ptr<const Message> message,
                                      messageReliability reliability,
                                      const std::vector<std::string> &groupNames);


    /**
     * Blocks until a message comes in from the Spread daemon. The received messsage may
     * be anything in the RegularMessage or MembershipMessage hierarchies.
     * @return An object containing the received message and associated metadata.
     */
    virtual boost::shared_ptr<SpreadMessage> receiveMessage();

    void handleConnectionError(ConnectionError& e);

    int waitingMessageByteCount();

    bool isConnected();

    /* called by in a new thread */
    void operator()();
    
private:
    enum ConnectionState {DISCONNECTED=0, CONNECTING=1, CONNECTED=2};

    struct ConnectionInfo{ 
        ConnectionInfo(){
        }
        ConnectionInfo(std::string const& ph,
                       std::string const& pcn,
                       bool recvmm,
                       ConnectionTimeout const& t,
                       SpreadMailbox::MailboxPriority p)
            : portAndHost(ph),
              privConnectionName(pcn),
              recvMembershipMessages(recvmm),
              timeout(t),
              priority(p){
        }

        std::string portAndHost;
        std::string privConnectionName;
        bool recvMembershipMessages;
        ConnectionTimeout timeout;
        SpreadMailbox::MailboxPriority priority;
    };

    ConnectionState _checkConnected();

    /**
     * Return true as soon as possible, or false if timeout is reached.
     */
    bool _waitConnected(unsigned msecs, unsigned attempts = 5);
    void _doOnConnected();
    void _doJoinGroup(std::string const& g);
    void _doLeaveGroup(std::string const& g);
    void _synchroniseGroups();
    void _asyncConnect();
    void _disconnect();
    
    ConnectionInfo m_ci;
    
    mutex_t m_connection_state_lock;
    ConnectionState m_connection_state;
    
    // spread is thread-safe, ssrc spread is probably not thread safe in
    // general, however, it is probably thread safe provided two threads don't
    // try to send messages at the same time, or receive messages at the same
    // time.
    // TODO: mutual exclusion at this level is not practical, so if ssrcspread
    // causes problems, the easiest solution is to duplicate mailboxes
    // per-thread. In practise this probably means two mailboxes, one for the
    // single receing thread, and one mailbox with mutual exclusion for sending
    // messages (since sending messages will probably only happen from one
    // thread anyway)
    boost::shared_ptr<SpreadMailbox> m_mailbox;

    volatile bool m_keep_trying;
    bool m_no_threads;

    boost::thread m_thread;
    
    /* Groups joined, and groups that we have been asked to join but haven't
     * necessarily done so yet.
     */
    mutex_t m_groups_lock;
    typedef std::set<std::string> string_set_t;
    string_set_t m_groups;
};

} // namespace cauv

#endif // ndef CAUV_SPREAD_RC_MAILBOX_H_INCLUDED
