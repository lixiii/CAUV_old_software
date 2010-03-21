#ifndef __HOUGH_LINESNODE_H__
#define __HOUGH_LINESNODE_H__

#include <map>
#include <vector>
#include <string>
#include <cmath>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../node.h"


class HoughLinesNode: public Node{
    public:
        HoughLinesNode(Scheduler& s)
            : Node(s){
            // slow node:
            m_speed = slow;
            
            // one input:
            registerInputID("image_in");
            
            // one output
            registerOutputID("image_out");
            
            // parameters:
            registerParamID<bool>("probabalistic", false);
            registerParamID<int>("rho", 1);
            registerParamID<float>("theta", CV_PI/180);
            registerParamID<int>("threshold", 80);
            // probabalistic only:
            registerParamID<int>("minLineLength", 30);
            registerParamID<int>("maxLineGap", 10);
            // non-probabalistic only:
            registerParamID<int>("srn", 0);
            registerParamID<int>("stn", 0);
        }
        
        // this node should be run even if nothing is connected to its output
        virtual bool isOutputNode() throw() { return true; } 

    protected:
        out_image_map_t doWork(in_image_map_t& inputs){
            out_image_map_t r;

            image_ptr_t img = inputs["image_in"];
            
            bool probabalistic = param<bool>("probabalistic");
            int rho = param<int>("rho");
            float theta = param<float>("theta");
            int threshold = param<int>("threshold");
            int min_ll = param<int>("minLineLength");
            int max_lg = param<int>("maxLineGap");
            int srn = param<int>("srn");
            int stn = param<int>("stn");

            cv::vector<cv::Vec4i> lines;
            try{
                if(probabalistic){
                    cv::HoughLinesP(img->cvMat(), lines, rho, theta, threshold, min_ll, max_lg);

                }else{
                    cv::vector<cv::Vec2f> r_theta_lines;
                    cv::HoughLines(img->cvMat(), r_theta_lines, rho, theta, threshold, srn, stn);
                    
                    if(numChildren()){
                        // convert lines to easy-to-draw form
                        for(unsigned i = 0; i < r_theta_lines.size(); i++)
                            lines.push_back(rThetaLineToSegment(r_theta_lines[i],
                                                                img->cvMat().size()));
                    }
                }
                
                if(numChildren()){
                    // then produce an output image overlay
                    boost::shared_ptr<Image> out = boost::make_shared<Image>();
                    out->source(img->source());
                    
                    // make a colour copy to draw pretty lines on
                    cvtColor(img->cvMat(), out->cvMat(), CV_GRAY2BGR);

                    for(unsigned i = 0; i < lines.size(); i++)
                        cv::line(out->cvMat(),
                                 cv::Point(lines[i][0], lines[i][1]),
                                 cv::Point(lines[i][2], lines[i][3]),
                                 cv::Scalar(0, 0, 255), 3, 8);
                    r["image_out"] = out;
                }
            }catch(cv::Exception& e){
                error() << "HoughLinesNode:\n\t"
                        << e.err << "\n\t"
                        << "in" << e.func << "," << e.file << ":" << e.line;
            }
     
            // TODO: send message about where lines are / do further processing
            
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

#endif // ndef __HOUGH_LINESNODE_H__
