#ifndef __MODULE_H
#define __MODULE_H__

#include <ftdi.h>
#include <iostream>
#include <iomanip>
#include <string.h>
#include <sstream>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/thread.hpp>

#include <common/cauv_utils.h>
#include <common/cauv_global.h>
#include <common/blocking_queue.h>
#include <common/messages.h>

class FTDIException : public std::exception
{
    protected:
        std::string message;
    public:
        FTDIException(const std::string& msg);
        FTDIException(const std::string& msg, int errCode, ftdi_context* ftdic);
        ~FTDIException() throw();
        virtual const char* what() const throw();
};

typedef boost::shared_ptr<struct usb_device> usb_device_ptr;

class FTDIContext : boost::noncopyable
{
    public:
        FTDIContext() : m_usb_open(false)
        {
            ftdi_init(&ftdic);
        }
        ~FTDIContext()
        {
            if (m_usb_open)
            {
                debug() << "Closing context USB...";
                ftdi_usb_close(&ftdic);
            }
            ftdi_deinit(&ftdic);
        }

        std::vector<usb_device_ptr> usbFindAll()
        {
            struct ftdi_device_list* devices;
            int ret;
            if ((ret = ftdi_usb_find_all(&ftdic, &devices, 0x0403, 0x6001)) < 0)
            {
                throw FTDIException("ftdi_usb_find_all failed", ret, &ftdic);
            }
            std::vector<usb_device_ptr> v;
            
            struct ftdi_device_list* pdevices = devices;
            while (ret--)
            {
                if (pdevices == 0)
                {
                    throw FTDIException("Error loading devices");
                }
                v.push_back(usb_device_ptr(pdevices->dev));
                pdevices = pdevices->next;
            }
            ftdi_list_free(&devices);

            return v;
        }

        void openDevice(usb_device_ptr dev)
        {
            int ret;
            if ((ret = ftdi_usb_open_dev(&ftdic, dev.get())) < 0)
            {
                throw FTDIException("Unable to open ftdi device", ret, &ftdic);
            }
            m_usb_open = true;
            debug() << "USB opened...";
        }

        void setBaudRate(int baudrate)
        {
            int ret;
            if ((ret = ftdi_set_baudrate(&ftdic, baudrate)) < 0)
            {
                throw FTDIException("Unable to set baudrate", ret, &ftdic);
            }
        }
        void setLineProperty(ftdi_bits_type bits, ftdi_stopbits_type stopBits, ftdi_parity_type parity)
        {
            int ret;
            if ((ret = ftdi_set_line_property(&ftdic, bits, stopBits, parity)) < 0)
            {
                throw FTDIException("Unable to set line property", ret, &ftdic);
            }
        }
        void setFlowControl(int flowControl)
        {
            int ret;
            if ((ret = ftdi_setflowctrl(&ftdic, flowControl)) < 0)
            {
                throw FTDIException("Unable to set flow control", ret, &ftdic);
            }
        }
        
        std::streamsize read(unsigned char* s, std::streamsize n)
        {
            int ret = ftdi_read_data(&ftdic, s, n);
            if (ret < 0)
            {
                throw FTDIException("Unable to read from ftdi device", ret, &ftdic);
            }
            return ret;
        }

        std::streamsize write(const unsigned char* s, std::streamsize n)
        {
            int ret = ftdi_write_data(&ftdic, const_cast<unsigned char*>(s), n);
            if (ret < 0)
            {
                throw FTDIException("Unable to write to ftdi device", ret, &ftdic);
            }
            return ret;
        }


    protected:
        struct ftdi_context ftdic;
        bool m_usb_open;
};

template<int baudrate, ftdi_bits_type bits, ftdi_stopbits_type stopBits, ftdi_parity_type parity, int flowControl>
class FTDIDevice : public boost::iostreams::device<boost::iostreams::bidirectional>
{
    public:
        FTDIDevice(int deviceID)
        {
            m_ftdic = boost::make_shared<FTDIContext>();

            std::vector<usb_device_ptr> devices = m_ftdic->usbFindAll();

            info() << "Number of devices found: " << devices.size();

            if (devices.size() == 0)
                throw FTDIException("Could not open device (No devices found)");
            else if (devices.size() < static_cast<size_t>(deviceID))
                throw FTDIException("Could not open device (No matching device found)");

            m_device = devices[deviceID];
            info() << "Module's device (id:" << deviceID << ") found...";

            m_ftdic->openDevice(m_device);
            m_ftdic->setBaudRate(baudrate);
            m_ftdic->setLineProperty(bits, stopBits, parity);
            m_ftdic->setFlowControl(flowControl);
        }
        
        std::streamsize read(char* s, std::streamsize n)
        {
            std::streamsize r;
            while (true) {
                r = m_ftdic->read(reinterpret_cast<unsigned char*>(s), n);
                if (r != 0) {
                    break;
                }
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
            }

            debug(4) << "Received module message";
#ifndef CAUV_NO_DEBUG
            std::stringstream ss;
            for (int i = 0; i < r; i++)
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)s[i] << " ";
            debug(5) << "Received" << ss.str();
#endif

            return r;
        }

        std::streamsize write(const char* s, std::streamsize n)
        {
            debug(4) << "Sending module message";
#ifndef CAUV_NO_DEBUG
            std::stringstream ss;
            for (int i = 0; i < n; i++)
                ss << std::hex << std::setw(2) << std::setfill('0') << (int)(unsigned char)s[i] << " ";
            debug(5) << "Sending" << ss.str();
#endif

            return m_ftdic->write(reinterpret_cast<const unsigned char*>(s), n);
        }
        
    protected:
        boost::shared_ptr<FTDIContext> m_ftdic;
        boost::shared_ptr<struct usb_device> m_device;
};


template<int baudrate, ftdi_bits_type bits, ftdi_stopbits_type stopBits, ftdi_parity_type parity, int flowControl>
class Module : public MessageSource
{
    typedef FTDIDevice<baudrate, bits, stopBits, parity, flowControl> ftdi_device_t;
    
    public:
        Module(int deviceID) : m_ftdiStreamBuffer(deviceID)
        {
        }

        void start()
        {
            if (m_ftdiStreamBuffer.is_open())
            {
                m_readThread = boost::thread(&Module::readLoop, this);
                m_sendThread = boost::thread(&Module::sendLoop, this);
            }
            else
            {
                error() << "FTDI Stream could not be opened";
            }
        }

        ~Module()
        {
            if (m_readThread.get_id() != boost::thread::id()) {
                m_readThread.interrupt();
                m_readThread.join();
            }
            if (m_sendThread.get_id() != boost::thread::id()) {
                m_sendThread.interrupt();
                m_sendThread.join();
            }
        }


        void send(boost::shared_ptr<const Message> message)
        {
            debug(3) << "Adding message to send queue: " << *message;
            m_sendQueue.push(message);
            debug(2) << "Added message to send queue: " << *message;
        }

    protected:
        boost::iostreams::stream_buffer<ftdi_device_t> m_ftdiStreamBuffer;
        boost::thread m_readThread, m_sendThread;
        BlockingQueue< boost::shared_ptr<const Message> > m_sendQueue;


        void sendLoop()
        {
            debug() << "Started module send thread";
            try {
                debug(3) << "Initialising ftdi buffer archive";
                while (true) {
                    boost::archive::binary_oarchive ar(m_ftdiStreamBuffer, boost::archive::no_header);
                    
                    debug(3) << "Waiting for message on send queue";
                    boost::shared_ptr<const Message> message = m_sendQueue.popWait();
                    debug(3) << "Sending message to module: " << *message;
                    
                    boost::shared_ptr<const byte_vec_t> bytes = message->toBytes();

                    uint32_t startWord = 0xdeadc01d;
                    uint32_t len = bytes->size();

                    uint32_t buf[2];
                    buf[0] = startWord;
                    buf[1] = len;
                    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf); 
                    std::vector<uint16_t> vheader(buf16, buf16+4);
                    uint16_t checksum = sumOnesComplement(vheader);
                    
                    ar << startWord;
                    ar << len;
                    ar << checksum;
                    foreach (char c, *bytes)
                        ar << c;
                    debug(3) << "Wrote message to module.";
                }
            }
            catch (boost::thread_interrupted&)
            {
                debug() << "Module send thread interrupted";
            }

            debug() << "Ending module send thread";
        }

        void readLoop()
        {
            debug() << "Started module read thread";
           
            try {

                byte_vec_t curMsg;
                boost::archive::binary_iarchive ar(m_ftdiStreamBuffer, boost::archive::no_header);

                while (true)
                {
                    boost::this_thread::interruption_point();
                    uint32_t startWord;
                    ar >> startWord;

                    if (startWord != 0xdeadc01d)
                    {
                        // Start word doesn't match, drop it
                        warning() << (std::string)(MakeString() << "Start word doesn't match (0x" << std::hex << startWord << " instead of 0xdeadc01d) -- starting to search for it"); 
                        while (startWord != 0xdeadc01d)
                        {
                            char nextByte;
                            ar >> nextByte;
                            startWord = (nextByte << 24) | ((startWord >> 8) & 0xffffff);
                        }
                    }

                    uint32_t len;
                    uint16_t checksum;
                    ar >> len;
                    ar >> checksum;

                    uint32_t buf[2];
                    buf[0] = startWord;
                    buf[1] = len;

                    uint16_t* buf16 = reinterpret_cast<uint16_t*>(buf); 
                    std::vector<uint16_t> vheader(buf16, buf16+4);

                    if (sumOnesComplement(vheader) != checksum)
                    {
                        // Checksum doesn't match, drop it
                        warning() << (std::string)(MakeString() << "Checksum is incorrect (0x" << std::hex << checksum << " instead of 0x" << std::hex << sumOnesComplement(vheader) << ")"); 
                        continue;
                    }

                    curMsg.clear();
                    char c;
                    for (uint32_t i = 0; i < len; ++i)
                    {
                        ar >> c;
                        curMsg.push_back(c);
                    }

#ifdef DEBUG_MCB_COMMS
                    std::stringstream ss;
                    ss << "Message from module  [ len: " << len << " | checksum: " << checksum << " ]" << std::endl;
                    foreach(char c, curMsg)
                    {
                        ss << std::hex << std::setw(2) << std::setfill('0') << (unsigned int)c << " ";
                    }
                    debug() << ss.str();
#endif

                    boost::shared_ptr<byte_vec_t> msg = boost::make_shared<byte_vec_t>(curMsg);
                    try {
                        this->notifyObservers(msg);
                    } catch (UnknownMessageIdException& e) {
                        error() << "Error when receiving message: " << e.what();
                    }
                }
           
            }
            catch (boost::thread_interrupted&)
            {
                debug() << "Module read thread interrupted";
            }

            debug() << "Ending module read thread";
        }
};


typedef Module<38400, BITS_8, STOP_BIT_1, NONE, SIO_DISABLE_FLOW_CTRL> MCBModule;

#endif // __MODULE_H__
