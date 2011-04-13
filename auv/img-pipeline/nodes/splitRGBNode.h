#ifndef __SPLIT_RGB_NODE_H__
#define __SPLIT_RGB_NODE_H__

#include <map>
#include <vector>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "../node.h"


namespace cauv{
namespace imgproc{

class SplitRGBNode: public Node{
    public:
        SplitRGBNode(Scheduler& sched, ImageProcessor& pl, std::string const& n, NodeType::e t)
            : Node(sched, pl, n, t){
        }

        void init(){
            // fast node:
            m_speed = fast;

            // input:
            registerInputID("image");
            
            // outputs:
            registerOutputID<image_ptr_t>("R");
            registerOutputID<image_ptr_t>("G");
            registerOutputID<image_ptr_t>("B");
        }
    
        virtual ~SplitRGBNode(){
            stop();
        }

    protected:
        out_map_t doWork(in_image_map_t& inputs){
            out_map_t r;
            image_ptr_t img = inputs["image"];
            int channel_type = CV_MAKETYPE(CV_MAT_DEPTH_MASK & img->cvMat().type(), 1);
            
            boost::shared_ptr<Image> R = boost::make_shared<Image>();
            boost::shared_ptr<Image> G = boost::make_shared<Image>();
            boost::shared_ptr<Image> B = boost::make_shared<Image>(); 
            R->cvMat() = cv::Mat(img->cvMat().size(), channel_type);
            G->cvMat() = cv::Mat(img->cvMat().size(), channel_type);
            B->cvMat() = cv::Mat(img->cvMat().size(), channel_type);
            
            cv::Mat out[] = {R->cvMat(), G->cvMat(), B->cvMat()};
            int from_to[] = {0,0, 1,1, 2,2};

            try{
                cv::mixChannels(&img->cvMat(), 1, out, 3, from_to, 3);
            }catch(cv::Exception& e){
                error() << "SplitRGBNode:\n\t"
                        << e.err << "\n\t"
                        << e.func << "," << e.file << ":" << e.line << "\n\t";
            }

            r["R"] = R;
            r["G"] = G;
            r["B"] = B;
            
            return r;
        }
    
    // Register this node type
    DECLARE_NFR;
};

} // namespace imgproc
} // namespace cauv

#endif // ndef __SPLIT_RGB_NODE_H__

