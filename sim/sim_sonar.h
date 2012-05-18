#ifndef CAUV_SIMSONAR
#define CAUV_SIMSONAR

#include <generated/types/CameraID.h>
#include <osg/Node>
#include <osg/Image>
#include <osgViewer/Viewer>
#include <common/cauv_node.h>
#include "FixedNodeTrackerManipulator.h"

namespace cauv {

class SimSonar {
    public:
    SimSonar (osg::Node *track_node,
              osg::Vec3d translation,
              osg::Vec3d axis1, float angle1,
              osg::Vec3d axis2, float angle2,
              osg::Vec3d axis3, float angle3,
              unsigned int width, unsigned int height,
              cauv::CauvNode *sim_node,
              unsigned int max_rate);
    void tick(double timestamp);
    void setup(osg::Node *root);
    private:
    cauv::CauvNode *sim_node;
    unsigned int width, height, depth;
    RateLimiter output_limit;
    osg::ref_ptr<osgViewer::Viewer> viewer;
    osg::ref_ptr<osg::Image> image;
    osg::ref_ptr<osg::Camera> camera;
    osg::ref_ptr<cauv::FixedNodeTrackerManipulator> fixed_manip;
};

}

#endif