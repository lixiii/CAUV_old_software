/* Copyright 2012 Cambridge Hydronautics Ltd.
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

#include "graphOptimiser.h"

#include <map>
#include <vector>
#include <cmath>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <utility/foreach.h>

#include <debug/cauv_debug.h>

using namespace cauv;
using namespace cauv::imgproc;

IncrementalPose IncrementalPose::from4dAffine(Eigen::Matrix4f const& a){
    const Eigen::Matrix3f r = a.block<3,3>(0, 0);
    const Eigen::Vector3f t = a.block<3,1>(0, 3);

    const Eigen::Vector3f tmp = r*Eigen::Vector3f(1,0,0);
    const float rz = (180/M_PI)*std::atan2(tmp[1], tmp[0]);
    IncrementalPose incr = {
        Eigen::Vector3f(t[0], t[1], rz)
    };
    return incr;
}

IncrementalPose IncrementalPose::from4dAffineDiff(
    Eigen::Matrix4f const& from,
    Eigen::Matrix4f const& to){
    return from4dAffine(to) -= from4dAffine(from);
}


class Node;
class Arc;
typedef boost::shared_ptr<Node> Node_ptr;
typedef boost::shared_ptr<Arc> Arc_ptr;


struct Arc{
    Node_ptr from;
    Node_ptr to;
    IncrementalPose x; // (to - from)

    // IncrementalPose has Eigen::Vector3f
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

struct Node{
    location_ptr p;
    Arc_ptr parent_link;
    std::vector<Arc_ptr> child_links;
    // tag is used to store the ID of the disjoint set during construction of
    // the tree, then the depth of the node from the root once the tree has
    // been constructed
    int tag;
};


static void adoptSubTree(Node_ptr new_parent, Arc_ptr a, Node_ptr child){
    assert(child);
    assert(new_parent);

    Arc_ptr old_parent_link = child->parent_link;
    child->parent_link = a;
    // reverse direction of relationship:
    old_parent_link->x = -old_parent_link->x;
    assert(old_parent_link->to == child);
    old_parent_link->to = old_parent_link->from;
    old_parent_link->from = child;
    child->child_links.push_back(old_parent_link);
    
    new_parent = child;
    a = old_parent_link;
    child = old_parent_link->to;
    if(child){
        // recurse
        adoptSubTree(new_parent, a, child);
    }
}

static void retagByDepth(int depth, Node_ptr p){
    p->tag = depth;
    foreach(Arc_ptr const& a, p->child_links){
        // use this opportunity to sanity check the tree, too
        assert(a->from == p);
        retagByDepth(depth+1, a->to);
    }
}


static Node_ptr spanningTree(constraint_vec const& constraints){
    assert(constraints.size());

    std::map<location_ptr, Node_ptr> included_nodes;
    std::map<location_ptr, Node_ptr>::const_iterator i, j;

    int tag = 0;
    std::vector<Node_ptr> set_root;

    Node_ptr root = boost::make_shared<Node>();
    root->parent_link.reset();
    root->p = (*constraints.begin())->b;
    root->tag = tag;
    included_nodes[root->p] = root;

    // set_root[tag] = root node of disjoint set
    set_root.push_back(root);
    
    int non_spanning = 0;

    debug() << "spanning tree construction over" << constraints.size() << "constraints...";
    foreach(pose_constraint_ptr p, constraints){
        i = included_nodes.find(p->a);
        j = included_nodes.find(p->b);
        if(i != included_nodes.end()){
            if(j == included_nodes.end()){
                // add p->b relative to existing p->a
                Arc_ptr a(new Arc);
                a->from = i->second;
                a->from->child_links.push_back(a);
                a->to = boost::make_shared<Node>();
                a->to->p = p->b;
                a->to->parent_link = a;
                a->to->tag = a->from->tag;
                a->x = p->a_to_b;

                included_nodes[p->b] = a->to;
            }else{
                // if a is not connected to b, this arc connects them:
                if(set_root[i->second->tag] != set_root[j->second->tag]){
                    Arc_ptr a(new Arc);
                    if((!i->second->parent_link) || (!j->second->parent_link)){
                        if(i->second->parent_link){
                            a->from = i->second;
                            a->to = j->second;
                            a->to->parent_link = a;
                            a->x = p->a_to_b;
                        }else{
                            a->from = j->second;
                            a->to = i->second;
                            a->to->parent_link = a;
                            // NB unary -
                            a->x = -p->a_to_b;
                        }
                        set_root[a->to->tag] = set_root[a->from->tag];
                    }else{
                        // both sides of this constraint have parents already -
                        // need to reverse parent/child relations in one of the
                        // trees :(
                        a->from = i->second;
                        a->to = j->second;
                        a->x = p->a_to_b;
                        // Make i the new parent of tree j->second via arc a                        
                        adoptSubTree(i->second, a, j->second);
                        set_root[a->to->tag] = set_root[a->from->tag];
                    }
                    a->from->child_links.push_back(a);
                }else{
                    // already in the tree

                    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    // make a separate list of non-spanning constraints?
                    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    non_spanning++;
                }
            }
        }else{
            if(j != included_nodes.end()){
                // add p->a relative to existing p->b
                Arc_ptr a(new Arc);
                a->from = j->second;
                a->to = boost::make_shared<Node>();
                a->to->p = p->a;
                a->to->parent_link = a;
                a->to->tag = a->from->tag;
                // NB: unary minus here
                a->x = -p->a_to_b;

                included_nodes[p->a] = a->to;
            }else{
                // add new disjoint arc
                ++tag;

                Arc_ptr a(new Arc);
                a->from = boost::make_shared<Node>();
                a->from->parent_link.reset();
                a->from->tag = tag;
                a->from->p = p->a;
                a->to = boost::make_shared<Node>();
                a->to->parent_link = a;
                a->to->tag = tag;
                a->to->p = p->b;
                a->x = p->a_to_b;

                set_root.push_back(a->from);
            }
        }
    }
    debug() << "spanning tree construction complete:"
            << included_nodes.size() << "nodes, "
            << non_spanning << "non-spanning constraints";
    // spanning tree invariant:
    assert(non_spanning == int(constraints.size() - (included_nodes.size()-1)));

    // re-tag the tree so that tag counts the depth from the root node (this
    // lets common parent nodes be identified easily)
    retagByDepth(0, set_root[0]);

    return set_root[0];
}

/* Optimise a constraint graph.
 * !!! paper ref here (Olson et al, Grisetti et al)
 * !!! this could be provided by an external class like scan matching
 *     so that methods can be easily compared.
 */
void GraphOptimiserV1::optimiseGraph(
    constraint_vec const& constraints,
    constraint_vec const& new_constraints // currently ignored, could be used to choose tree parametrisation, or more intrusively
) const{
    const int max_iters = 1;

    Node_ptr root = spanningTree(constraints);

    for(int iter = 0; iter < max_iters; iter++){


    }
}


