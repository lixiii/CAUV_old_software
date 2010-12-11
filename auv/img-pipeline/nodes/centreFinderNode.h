#ifndef __CENTREFINDER_H__
#define __CENTREFINDER_H__

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <generated/messages.h>

#include "../node.h"
#include "outputNode.h"

class CentreFinderNode : public OutputNode{
    public:
        CentreFinderNode(Scheduler& sched, ImageProcessor& pl, NodeType::e t)
            : OutputNode(sched, pl, t){
        }

        void init(){
            //Fast node
            m_speed = fast;
            
            //One input
            registerInputID("image_in");
            
            //No output
            
            //Parameters
            registerParamID<std::string>("Name", "Unnamed");
            
        }
    
        virtual ~CentreFinderNode(){
            stop();
        }

    protected:
        out_map_t doWork(in_image_map_t& inputs){
            out_map_t r;

            std::string name = param<std::string>("Name");

            image_ptr_t img = inputs["image_in"];

            if(!img->cvMat().isContinuous())
                throw(parameter_error("Image must be continuous."));
            if((img->cvMat().type() & CV_MAT_DEPTH_MASK) != CV_8U)
                throw(parameter_error("Image must have unsigned bytes."));
            if(img->cvMat().channels() > 1)
                throw(parameter_error("Image must have only one channel."));
                //TODO: support vector parameters

            int totalX = 0;
            int totalY = 0;
            int sum = 0;

            for(int i = 0; i < img->cvMat().cols; i++) {
                for(int j = 0; j < img->cvMat().rows; j++) {
                   if(img->cvMat().at<uint8_t>(i, j) > 127) {
                       totalX += i;
                       totalY += j;
                       sum++;
                   }
                }
            }

            sendMessage(boost::make_shared<CentreMessage>(name, totalX / sum, totalY / sum));
            return r;
        }

    //Register this node type
    DECLARE_NFR;
};

#endif //ndef __CENTREFINDER_H__
