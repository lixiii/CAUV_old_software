#ifndef __COPY_NODE_MASK_H__
#define __COPY_NODE_MASK_H__

#include <map>
#include <vector>
#include <string>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../node.h"
#include "../nodeFactory.h"


class CopyNodeMask: public Node{
    public:
        CopyNodeMask(Scheduler& sched, ImageProcessor& pl, NodeType::e t)
            : Node(sched, pl, t){
        }

        void init(){
            // fast node:
            m_speed = fast;

            // two input:
            registerInputID("image");
            registerInputID("mask");
            
            // output:
            registerOutputID<image_ptr_t>("image copy");
            
            // no parameters
            // registerParamID<>();
        }
    
        virtual ~CopyNodeMask(){
            stop();
        }

    protected:
        out_map_t doWork(in_image_map_t& inputs){
            out_map_t r;
            
            image_ptr_t img = inputs["image"];
            image_ptr_t mask = inputs["mask"];
            
            boost::shared_ptr<Image> out = boost::make_shared<Image>();
            
            try{
                img->cvMat().copyTo(out->cvMat(), mask->cvMat());
                r["image copy"] = out;
            }catch(cv::Exception& e){
                error() << "CopyNodeMask:\n\t"
                        << e.err << "\n\t"
                        << e.func << "," << e.file << ":" << e.line << "\n\t";
            }
            
            return r;
        }
        
        // Register this node type
        DECLARE_NFR;
};

#endif // ndef __COPY_NODE_MASK_H__

