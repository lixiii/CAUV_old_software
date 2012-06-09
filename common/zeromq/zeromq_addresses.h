#ifndef ZEROMQ_ADDRESSES_H
#define ZEROMQ_ADDRESSES_H
#include <string>
#include <vector>
#include <stdint.h>

namespace cauv {

#define NODE_MARKER_MSGID 223
#define DAEMON_MARKER_MSGID 224

typedef std::pair<bool, std::vector<uint32_t> > subscription_vec_t;

std::string gen_subscription_message(subscription_vec_t);

subscription_vec_t parse_subscription_message(std::string msg);

std::string get_vehicle_name(void);

std::string get_ipc_directory(void);

std::string get_ipc_directory(const std::string vehicle_name);

}

#endif
