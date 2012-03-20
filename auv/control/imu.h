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

#ifndef __CAUV_IMU_OBSERVER_H__
#define	__CAUV_IMU_OBSERVER_H__

#include <generated/types/floatYPR.h>

namespace cauv{

class IMUObserver
{
    public:
        virtual void onTelemetry(const floatYPR& attitude) = 0;
};

} // namespace cauv

#endif // ndef __CAUV_IMU_OBSERVER_H__
