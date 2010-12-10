#ifndef __RUNNING_AVERAGE_NODE_H__
#define __RUNNING_AVERAGE_NODE_H__

#include <map>
#include <vector>
#include <string>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>
#include <boost/random/variate_generator.hpp>

#include <opencv/cv.h>

#include <common/cauv_utils.h>
#include "../node.h"


class RunningAverageNode: public Node{
    public:
        RunningAverageNode(Scheduler& sched, ImageProcessor& pl, NodeType::e t) :
                Node(sched, pl, t),
                m_channels(-1),
                bytedist(0, 255),
                randbyte(gen, bytedist)
        {
        }

        void init()
        {
            // fast node:
            m_speed = fast;

            // one input:
            registerInputID("image");

            // two outputs:
            registerOutputID<image_ptr_t>("average (not copied)");

            // parameters:
            //   K: the number of clusters
            //   colorise: colour each pixel with its clusters centre (otherwise, colour with cluster id)
            registerParamID<float>("alpha", 0.2);
        }
    
        virtual ~RunningAverageNode()
        {
            stop();
        }
        
    protected:

        cv::Mat avg;

        out_map_t doWork(in_image_map_t& inputs){
            out_map_t r;

            image_ptr_t img = inputs["image"];
            
            float alpha = param<float>("alpha");

            if(alpha < 0 | alpha > 1)
            {
                error() << "alpha must be within [0,1]";
                return r;
            }

            if (avg.empty()
                || avg.type() != img->cvMat().type()
                || avg.size() != img->cvMat().size())
            {
                avg = img->cvMat()->clone();
            }
            else
            {
                cv::accumulateWeighted(*img, *avg, alpha);
            }

            const int elem_size = img->cvMat().elemSize();
            const int row_size = cols * elem_size;
            unsigned char *img_rp, *img_cp, *img_bp;

            boost::shared_ptr<Image> clusterids = boost::make_shared<Image>(cv::Mat(rows, cols, CV_8UC1));
            cv::Mat clusteridsMat = clusterids->cvMat();

            // Clear val sums and sizes (val sum for single pass mean calculation)
            for (size_t i = 0; i < m_clusters.size(); i++) {
                cluster& cl = m_clusters[i];
                for(ch = 0; ch < m_channels; ch++)
                {
                    cl.valsum[ch] = 0;
                }
                cl.size = 0;
            }

            // Assign each pixel to the nearest cluster
            for(y = 0, img_rp = img->cvMat().data; y < rows; y++, img_rp += row_size)
                for(x = 0, img_cp = img_rp; x < cols; x++, img_cp += elem_size)
                {
                    size_t best_cl_i = K;
                    unsigned int best_cl_sqdiff = UINT_MAX;      
                    for (size_t i = 0; i < m_clusters.size(); i++) {
                        cluster& cl = m_clusters[i];
                        unsigned int sqdiff = 0;
                        for(ch = 0, img_bp = img_cp; ch < m_channels; ch++, img_bp++)
                        {
                            sqdiff += (*img_bp - cl.centre[ch]) * (*img_bp - cl.centre[ch]);
                        }

                        if (sqdiff < best_cl_sqdiff) {
                            best_cl_i = i;
                            best_cl_sqdiff = sqdiff;
                        }
                    }
                    assert(best_cl_i < (unsigned int)K);
                    clusteridsMat.at<unsigned char>(y,x) = clamp_cast<unsigned char>((unsigned char)0, best_cl_i, (unsigned char)255);
                    cluster& best_cl = m_clusters[best_cl_i];
                    best_cl.size++;
                    for(ch = 0, img_bp = img_cp; ch < m_channels; ch++, img_bp++)
                    {
                        best_cl.valsum[ch] += *img_bp;
                    }
                }

            // Find empty clusters, and assign to them the most distant point
            for (size_t i = 0; i < m_clusters.size(); i++)
            {
                cluster& cl = m_clusters[i];
                if (cl.size == 0)
                {
                    int farthest_point_x = -1;
                    int farthest_point_y = -1;
                    unsigned int farthest_point_sqdist = 0;

                    for(y = 0, img_rp = img->cvMat().data; y < rows; y++, img_rp += row_size)
                        for(x = 0, img_cp = img_rp; x < cols; x++, img_cp += elem_size)
                        {
                            cluster& cl = m_clusters[clusteridsMat.at<unsigned char>(y,x)];
                            if(cl.size < 2)
                            {
                                break;
                            }
                            
                            unsigned int sqdist = 0;
                            for(ch = 0, img_bp = img_cp; ch < m_channels; ch++, img_bp++)
                            {
                                sqdist += (*img_bp - cl.centre[ch]) * (*img_bp - cl.centre[ch]);
                            }
                            
                            if (sqdist > farthest_point_sqdist) {
                                farthest_point_sqdist = sqdist;
                                farthest_point_x = x;
                                farthest_point_y = y;
                            }
                        }
                   
                    assert(farthest_point_x != -1 && farthest_point_y != -1);
                    
                    int farthest_point_clid = clusteridsMat.at<unsigned char>(farthest_point_y,farthest_point_x);
                    cluster& farthest_point_cl = m_clusters[farthest_point_clid];
                    clusteridsMat.at<unsigned char>(farthest_point_y,farthest_point_x) = i;
                    
                    farthest_point_cl.size--;
                    cl.size++;
                    for(ch = 0, img_bp = img->cvMat().data + y * row_size + x * elem_size; ch < m_channels; ch++, img_bp++)
                    {
                        farthest_point_cl.valsum[ch] -= *img_bp;
                        cl.valsum[ch] += *img_bp;
                    }
                }
            }


            // Change cluster centres (means) based on mean of pixel values
            for (size_t i = 0; i < m_clusters.size(); i++)
            {
                cluster& cl = m_clusters[i];
                assert(cl.size != 0);
                
                for(ch = 0; ch < m_channels; ch++)
                {
                    cl.centre[ch] = cl.valsum[ch]/cl.size;
                    debug(5) << i << ".centre[" << ch << "] =" << (int) cl.centre[ch];
                }
            }

            if (colorise) {
                // Colorise if necessary

                for(y = 0, img_rp = img->cvMat().data; y < rows; y++, img_rp += row_size)
                    for(x = 0, img_cp = img_rp; x < cols; x++, img_cp += elem_size)
                    {
                        cluster& cl = m_clusters[clusteridsMat.at<unsigned char>(y,x)];

                        for(ch = 0, img_bp = img_cp; ch < m_channels; ch++, img_bp++)
                        {
                            *img_bp = cl.centre[ch];
                        }
                    }
            }
            
            r["average (not copied)"] = img;
            
            return r;
        }
    
    // Register this node type
    DECLARE_NFR;
};

#endif // ndef __GAUSSIAN_BLUR_NODE_H__
