#ifndef __CAUV_XSENS_IMU_H__
#define	__CAUV_XSENS_IMU_H__

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#include <xsens/cmt3.h>

#include <generated/messages_fwd.h>
#include <utility/observable.h>

namespace cauv{

class XsensObserver
{
    public:
        virtual void onTelemetry(const floatYPR& attitude) = 0;
};

class XsensIMU : public Observable<XsensObserver>, boost::noncopyable
{
    public:
        XsensIMU(int id);
        virtual ~XsensIMU();

        floatYPR getAttitude();
        void configure(CmtOutputMode &mode, CmtOutputSettings &settings);
        void setObjectAlignmentMatrix(CmtMatrix m);

        void start();

    protected:
        xsens::Cmt3 m_cmt3;
        CmtPortInfo m_port;
        boost::thread m_readThread;

        void readThread();
};


class XsensException : public std::exception
{
    protected:
        std::string message;
    public:
        XsensException(const std::string& msg);
        ~XsensException() throw();
        virtual const char* what() const throw();
};

} // namespace cauv

#endif // ndef __CAUV_XSENS_IMU_H__
