/* Copyright 2011-2013 Cambridge Hydronautics Ltd.
 *
 * See license.txt for details.
 */


#ifndef __BROADCAST_HISTOGRAMNODE_H__
#define __BROADCAST_HISTOGRAMNODE_H__

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <generated/types/HistogramMessage.h>

#include "../node.h"
#include "outputNode.h"


namespace cauv{
namespace imgproc{

class BroadcastHistogramNode: public OutputNode{
    public:
        BroadcastHistogramNode(ConstructArgs const& args)
            : OutputNode(args){
        }

        void init(){
            // fast node:
            m_speed = fast;
            
            // no inputs:
            
            // no outputs
            
            // parameters:
            registerParamID< std::vector<float> >("histogram",
                                                  std::vector<float>(),
                                                  "histogram to broadcast",
                                                  Must_Be_New);
            registerParamID<std::string>("name",
                                         "unnamed histogram",
                                         "name for histogram");
        }

    protected:
        void doWork(in_image_map_t&, out_map_t&){
            const std::string name = param<std::string>("name");
            const std::vector<float> histogram = param< std::vector<float> >("histogram");
            
            if(histogram.size())
                sendMessage(boost::make_shared<HistogramMessage>(name, histogram));
        }

    // Register this node type
    DECLARE_NFR;
};

} // namespace imgproc
} // namespace cauv

#endif // ndef __BROADCAST_HISTOGRAMNODE_H__

