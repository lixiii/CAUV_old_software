/* Copyright 2011-2012 Cambridge Hydronautics Ltd.
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

#ifndef __CAUV_SONAR_SLAM_CLOUD_PART_H__
#define __CAUV_SONAR_SLAM_CLOUD_PART_H__

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <pcl/point_cloud.h>
#include <pcl/common/transforms.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/surface/convex_hull.h>

#include <Eigen/StdVector>

#include <generated/types/TimeStamp.h>
#include <generated/types/KeyPoint.h>

#include <utility/foreach.h>

#include "graphOptimiser.h"
#include "common.h"

namespace cauv{
namespace imgproc{


// - SLAM Clouds
class SlamCloudLocation{
    public:

        SlamCloudLocation(TimeStamp const& t)
            : m_relative_to(),
              m_relative_transformation(Eigen::Matrix4f::Identity()),
              m_time(t){
        }

        template<typename PointT>
        explicit SlamCloudLocation(boost::shared_ptr<SlamCloudPart<PointT> > const& p)
            : m_relative_to(p->m_relative_to),
              m_relative_transformation(p->m_relative_transformation),
              m_time(p->m_time){
        }

        explicit SlamCloudLocation(float x, float y, float theta_radians)
            : m_relative_to(), 
              m_relative_transformation(Eigen::Matrix4f::Identity()),
              m_time(){
            m_relative_transformation.block<2,1>(0,3) = Eigen::Vector2f(x, y);
            m_relative_transformation.block<2,2>(0,0) *= Eigen::Matrix2f(Eigen::Rotation2D<float>(theta_radians));
        }

        virtual ~SlamCloudLocation(){
        }

        // post-multiply
        void transform(Eigen::Matrix4f const& transformation){
            m_relative_transformation = m_relative_transformation * transformation;
            transformationChanged();
        }

        void setRelativeToNone(){
            if(m_relative_to){
                m_relative_to.reset();
                transformationChanged();
            }
        }
        void setRelativeTo(location_ptr p){
            if(p != m_relative_to){
                m_relative_to = p;
                transformationChanged();
            }
        }
        
        // add constraint from this -> p (p observed from this) t is the
        // RELATIVE (not incremental) pose at which p is observed
        RelativePose addConstraintTo(location_ptr p, Eigen::Matrix4f const& t){
            const RelativePose r = RelativePose::from4dAffine(t);
            
            m_constrained_to.push_back(p);
            m_constraints.push_back(r);
            
            return r;
        }

        static pose_constraint_ptr addConstraintBetween(location_ptr from,
                                                        location_ptr to,
                                                        Eigen::Matrix4f from_to_to){
            const RelativePose rel_pose = from->addConstraintTo(to, from_to_to);
            return boost::make_shared<RelativePoseConstraint>(rel_pose, from, to);
        }

        void setRelativeTransform(Eigen::Matrix4f const& m){
            m_relative_transformation = m;
            transformationChanged();            
        }

        Eigen::Matrix4f globalTransform() const{
            Eigen::Matrix4f r = relativeTransform();
            location_ptr p = relativeTo();
            if(p)
                return p->globalTransform() * r;
            return r;
        }

        Eigen::Matrix4f const& relativeTransform() const{ return m_relative_transformation; }
        location_ptr relativeTo() const{ return m_relative_to; }
        TimeStamp const& time() const{ return m_time; }

        location_vec const& constrainedTo() const{ return m_constrained_to; }

        // (relatively) expensive: use sparingly
        typedef std::vector<Eigen::Vector3f,Eigen::aligned_allocator<Eigen::Vector3f> > v3f_vec;
        v3f_vec constraintEndsGlobal() const{
            v3f_vec r;
            foreach(RelativePose const& rel, m_constraints){
                const Eigen::Matrix4f pt_transform = globalTransform() * rel.to4dAffine();
                const Eigen::Vector3f pt = pt_transform.block<3,1>(0,3);
                r.push_back(pt);
            }
            return r;
        }
        

        // We have an Eigen::Matrix4f as a member
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    protected:
        virtual void transformationChanged(){}

    private:
        typedef std::vector< RelativePose, Eigen::aligned_allocator<RelativePose> > rel_pose_vec;
    
        location_ptr    m_relative_to;
        Eigen::Matrix4f m_relative_transformation;

        location_vec  m_constrained_to;
        rel_pose_vec  m_constraints;

        TimeStamp       m_time;
};

template<typename PointT>
class KDTreeCachingCloud: public pcl::PointCloud<PointT>,
                          public boost::enable_shared_from_this< KDTreeCachingCloud<PointT> >{
    public:
        // - public types
        typedef boost::shared_ptr<SlamCloudPart<PointT> > Ptr;
        typedef boost::shared_ptr<const SlamCloudPart<PointT> > ConstPtr;
        typedef pcl::PointCloud<PointT> base_cloud_t;
        typedef typename base_cloud_t::Ptr base_cloud_ptr;

        using boost::enable_shared_from_this< KDTreeCachingCloud<PointT> >::shared_from_this;
    
        KDTreeCachingCloud()
            : pcl::PointCloud<PointT>(), m_kdtree(), m_kdtree_invalid(true){
        }

        inline void push_back(PointT const& p){
            base_cloud_t::push_back(p);
            m_kdtree_invalid = true;
        }

        int nearestKSearch(PointT const& point,
                            int k,
                            std::vector<int> &k_indices,
                            std::vector<float> &k_sqr_distances){
            ensureKdTree();
            return m_kdtree.nearestKSearch(point, k, k_indices, k_sqr_distances);
        }

        float nearestSquaredDist(PointT const& point){
            std::vector<int> k_indices(1);
            std::vector<float> k_sqr_distances(1);
            ensureKdTree();
            m_kdtree.nearestKSearch(point, 1, k_indices, k_sqr_distances);
            return k_sqr_distances[0];
        }
    
        void invalidateKDTree(){
            m_kdtree_invalid = true;
        }
        
    private:
        void ensureKdTree(){
            if(m_kdtree_invalid){
                m_kdtree_invalid = false;
                m_kdtree.setInputCloud(shared_from_this());
                // clear the circular reference just created through call to
                // base version of setInputCloud, which doesn't clobber the
                // kdtree...
                m_kdtree.pcl::KdTree<PointT>::setInputCloud(base_cloud_ptr());
           }
        }

        pcl::KdTreeFLANN<PointT> m_kdtree;
        bool m_kdtree_invalid;
};

template<typename PointT>
class SlamCloudPart: public SlamCloudLocation,
                     public KDTreeCachingCloud<PointT>,
                     public boost::enable_shared_from_this< SlamCloudPart<PointT> >{
    public:
        // - public types
        typedef boost::shared_ptr<SlamCloudPart<PointT> > Ptr;
        typedef boost::shared_ptr<const SlamCloudPart<PointT> > ConstPtr;
        typedef pcl::PointCloud<PointT> base_cloud_t;
        typedef typename base_cloud_t::Ptr base_cloud_ptr;

        using pcl::PointCloud<PointT>::points;
        using pcl::PointCloud<PointT>::width;
        using pcl::PointCloud<PointT>::size;
        using pcl::PointCloud<PointT>::push_back;

        using boost::enable_shared_from_this< SlamCloudPart<PointT> >::shared_from_this;

        struct FilterResponse{
            FilterResponse(float thr)
                : m_thr(thr){
            }
            bool operator()(KeyPoint const& k) const{
                return k.response > m_thr;
            }
            const float m_thr;
        };
        template<typename Callable>
        SlamCloudPart(std::vector<KeyPoint> const& kps, TimeStamp const& t, Callable const& filter)
            : SlamCloudLocation(t),
              boost::enable_shared_from_this< SlamCloudPart<PointT> >(),
              m_point_descriptors(),
              m_keypoint_indices(),
              m_keypoint_goodness(kps.size(), 0),
              m_local_convexhull_verts(),
              m_local_convexhull_cloud(),
              m_local_convexhull_invalid(true){

            this->height = 1;
            this->is_dense = true;
            reserve(kps.size());

            for(size_t i = 0; i < kps.size(); i++){
                if(!filter(kps[i]))
                    continue;
                push_back(PointT(kps[i].pt.x, kps[i].pt.y, 0.0f), kps[i].response, i);
                // start out assuming all keypoints that have passed the weight
                // test are good:
                m_keypoint_goodness[i] = 1;
            }
        }

        virtual ~SlamCloudPart(){
        }

        void getLocalConvexHull(base_cloud_ptr& hull_points,
                                std::vector<pcl::Vertices>& polygons){
            ensureLocalConvexHull();
            hull_points = m_local_convexhull_cloud;
            polygons = m_local_convexhull_verts;
        }

        void getGlobalConvexHull(base_cloud_ptr& hull_points,
                                std::vector<pcl::Vertices>& polygons){
            // !!! TODO: cache this result too
            ensureLocalConvexHull();
            polygons = m_local_convexhull_verts;
            hull_points = boost::make_shared<base_cloud_t>();
            pcl::transformPointCloud(*m_local_convexhull_cloud, *hull_points, globalTransform());
        }

        std::vector<descriptor_t>& descriptors(){ return m_point_descriptors; }
        std::vector<descriptor_t> const& descriptors() const{ return m_point_descriptors; }

        std::vector<std::size_t>& ptIndices(){ return m_keypoint_indices; }
        std::vector<std::size_t> const& ptIndices() const{ return m_keypoint_indices; }

        std::vector<int>& keyPointGoodness(){ return m_keypoint_goodness; }
        std::vector<int> const& keyPointGoodness() const{ return m_keypoint_goodness; }

        // "overridden" methods: note that none of the base class methods are
        // virtual, so these are only called if the call is made through a
        // pointer to this derived type
        void reserve(std::size_t s){
            base_cloud_t::reserve(s);
            m_point_descriptors.reserve(s);
        }

        inline void push_back(PointT const& p, descriptor_t const& d, std::size_t const& idx){
            KDTreeCachingCloud<PointT>::push_back(p);
            m_point_descriptors.push_back(d);
            m_keypoint_indices.push_back(idx);
            m_local_convexhull_invalid = true;
        }
        

        // this type derives from something with an Eigen::Matrix4f as a
        // member:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    protected:
        virtual void transformationChanged(){
            this->invalidateKDTree();
            m_local_convexhull_invalid = true;
        }

    private:
        void ensureLocalConvexHull(){
            if(m_local_convexhull_invalid){
                m_local_convexhull_invalid = false;

                pcl::ConvexHull<PointT> hull_calculator;
                #if PCL_VERSION >= PCL_VERSION_CALC(1,5,0)
                hull_calculator.setDimension(2);
                #endif
                m_local_convexhull_cloud = (boost::make_shared<base_cloud_t>());
                m_local_convexhull_cloud->is_dense = true;

                hull_calculator.setInputCloud(shared_from_this());
                hull_calculator.reconstruct(*m_local_convexhull_cloud, m_local_convexhull_verts);
            }
        }

        std::vector<descriptor_t> m_point_descriptors;

        // original indices of the keypoints from which the points in this
        // cloud were derived - used to generate training data for those
        // keypoints: these are not generally preserved by operations on the
        // point cloud
        std::vector<std::size_t> m_keypoint_indices;

        // 1 = keypoint turned out to be good, 0 = keypoint turned out to be
        // bad
        std::vector<int> m_keypoint_goodness;

        std::vector<pcl::Vertices> m_local_convexhull_verts;
        base_cloud_ptr m_local_convexhull_cloud;
        bool m_local_convexhull_invalid;
};

} // namespace imgproc
} // namespace cauv 


#endif //ndef __CAUV_SONAR_SLAM_CLOUD_PART_H__