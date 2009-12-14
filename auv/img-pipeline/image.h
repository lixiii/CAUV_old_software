#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <opencv/cv.h>

class Image{
    public:
        enum Source {file, camera, sonar};
        
        Image(cv::Mat const& cv_image, Source const& source)
            : m_img(cv_image), m_source(source){
        }
        
        // Copy constructor; take a deep copy of the image data
        Image(Image const& other)
            : m_img(other.m_img.clone()), m_source(other.m_source){
        }

        cv::Mat const& cvMat() const{
            return m_img;
        }

        cv::Mat& cvMat(){
            return m_img;
        }

        Source source() const{
            return m_source;
        }
        
    private:
        cv::Mat m_img;
        Source m_source;
};

#endif // ndef __IMAGE_H__
