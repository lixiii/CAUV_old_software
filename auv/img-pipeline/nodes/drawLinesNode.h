#ifndef __DRAW_LINESNODE_H__
#define __DRAW_LINESNODE_H__

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <generated/messages.h>

#include "../node.h"
#include "outputNode.h"


namespace cauv{
namespace imgproc{

class DrawLinesNode: public Node{
    public:
        DrawLinesNode(Scheduler& sched, ImageProcessor& pl, NodeType::e t)
            : Node(sched, pl, t){
        }

        void init(){
            // slow node:
            m_speed = slow;
            
            // one input:
            registerInputID(Image_In_Name);
            
            // one output
            registerOutputID<image_ptr_t>(Image_Out_Copied_Name);
            
            // parameters:
            registerParamID< std::vector<Line> >("lines", std::vector<Line>());
        }
    
        virtual ~DrawLinesNode(){
            stop();
        }

    protected:
        out_map_t doWork(in_image_map_t& inputs){
            out_map_t r;

            image_ptr_t img = inputs[Image_In_Name];
            
            const std::vector<Line> lines = param< std::vector<Line> >("lines");

            try{
                cv::Mat out_mat;
                
                if (img->channels() == 1)
                {
                    // make a colour copy to draw pretty lines on
                    cvtColor(img->cvMat(), out_mat, CV_GRAY2BGR);
                }
                else if (img->channels() == 3)
                {
                    img->cvMat().copyTo(out_mat);
                }
                else
                {
                    error() << "WTF kind of image has" << img->channels() << "channels?";
                    return r;
                }

                const float width = img->width();
                const float height = img->height();
                float maxlen = width*width + height*height;
                foreach(const Line& line, lines)
                {
                    float sa = std::sin(line.angle);
                    float ca = std::cos(line.angle);
                    cv::Point2f c(line.centre.x * width, line.centre.y * height);
                    cv::Point2f dir(ca,sa);
                    float l = std::min(line.length * width, maxlen);

                    cv::line(out_mat,
                             c - l/2 * dir, c + l/2 * dir,
                             cv::Scalar(0, 0, 255), 3, 8);
                }
                
                r[Image_Out_Copied_Name] = boost::make_shared<Image>(out_mat);
            }catch(cv::Exception& e){
                error() << "DrawLinesNode:\n\t"
                        << e.err << "\n\t"
                        << "in" << e.func << "," << e.file << ":" << e.line;
            }

            return r;
        }

    private:
        static cv::Vec4i rThetaLineToSegment(cv::Vec2f const& l, cv::Size const& s){
            cv::Vec4i r;
            float rho = l[0];
            float theta = l[1];
            double a = std::cos(theta);
            double b = std::sin(theta);
            if(std::fabs(a) < 0.001)
            {
                r[0] = r[2] = cvRound(rho);
                r[1] = 0;
                r[3] = s.height;
            }else if(std::fabs(b) < 0.001)
            {
                r[1] = r[3] = cvRound(rho);
                r[0] = 0;
                r[3] = s.width;
            }
            else
            {
                r[0] = 0;
                r[1] = cvRound(rho/b);
                r[2] = cvRound(rho/a);
                r[3] = 0;
            }
            return r;
        }

    // Register this node type
    DECLARE_NFR;
};

} // namespace imgproc
} // namespace cauv

#endif // ndef __DRAW_LINESNODE_H__

