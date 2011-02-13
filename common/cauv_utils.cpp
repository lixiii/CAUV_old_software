#include "cauv_utils.h"

#include <generated/messages.h>

#include <boost/date_time.hpp>
#include <boost/thread/thread.hpp>

uint16_t cauv::sumOnesComplement(std::vector<uint16_t> bytes)
{
    uint32_t sum = 0;
    foreach(uint16_t byte, bytes)
    {
        sum += byte;
    }
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0xffff);

    return ~(uint16_t)sum;
}


cauv::TimeStamp cauv::now(){
    cauv::TimeStamp r;
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::ptime current_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = current_time - epoch;
    r.secs = diff.total_seconds();
    r.musecs = diff.fractional_seconds();
    return r;
}

std::string cauv::now(std::string const& format){
    using namespace boost::posix_time;
    std::ostringstream oss;
    time_facet* facet = new time_facet(format.c_str());
    oss.imbue(std::locale(oss.getloc(), facet));
    oss << microsec_clock::local_time();
    return oss.str();
}

void cauv::msleep(unsigned msecs){
    boost::this_thread::sleep(boost::posix_time::milliseconds(msecs));
}

std::string cauv::implode( const std::string &glue, const std::set<std::string> &pieces )
{
    std::string a;
    int leng=pieces.size();
    
    std::set<std::string>::iterator i;
    for (i=pieces.begin(); i!=pieces.end(); i++){
        a += *i;
        if (--leng)
            a += glue;
    }
    return a;
}

