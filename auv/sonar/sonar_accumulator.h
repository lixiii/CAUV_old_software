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

#ifndef __CAUV_SONAR_ACCUMULATOR_H__
#define __CAUV_SONAR_ACCUMULATOR_H__

#include <boost/shared_ptr.hpp>

#include <stdint.h>

namespace cv{
class Mat;
}

namespace cauv{

struct SonarDataLine;
struct PolarImage;
class Image;


float msgPolarAngleToRadians(int32_t angle);

class SonarAccumulator
{
    public: 
        SonarAccumulator(uint32_t size=400);
        
        bool accumulateDataLine(const SonarDataLine& data);
        bool setWholeImage(PolarImage const& image);

        void setSize(uint32_t size);

        boost::shared_ptr<Image> img() const;
        cv::Mat mat() const;

    protected:

        int m_last_line_bearing;    
        float m_image_completed;
        boost::shared_ptr<Image> m_img;
        int m_bearingRange, m_range, m_scanWidth;
        size_t m_nbins;
        // diameter of accumulator 
        uint32_t m_size;
        
        void reset();
};

} // namespace cauv

#endif //__CAUV_SONAR_ACCUMULATOR_H__

