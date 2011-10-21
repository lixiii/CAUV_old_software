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

#ifndef __NULL_PARAM_NODE_H__
#define __NULL_PARAM_NODE_H__

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

#include "../node.h"
#include "outputNode.h"


namespace cauv{
namespace imgproc{

class NullParamNode: public Node{
    public:
        NullParamNode(ConstructArgs const& args)
            : Node(args){
        }

        void init(){
            // fast node:
            m_speed = fast;
            
            // no inputs
            //registerInputID(Image_In_Name);
            
            // one output:
            registerOutputID<NodeParamValue>("out");
            
            // parameters:
            registerParamID<NodeParamValue>("in", NodeParamValue(),
                                            "the input"); 
        }
    
        virtual ~NullParamNode(){
            stop();
        }

    protected:
        out_map_t doWork(in_image_map_t&){
            out_map_t r;
            r["out"] = param<NodeParamValue>("in");
            return r;
        }
    private:

    // Register this node type
    DECLARE_NFR;
};

} // namespace imgproc
} // namespace cauv

#endif // ndef __NULL_PARAM_NODE_H__

