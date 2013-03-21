/* Copyright 2011-2013 Cambridge Hydronautics Ltd.
 *
 * See license.txt for details.
 */


#include "processnode.h"

#include <QtGui>

#include <debug/cauv_debug.h>

using namespace cauv;
using namespace cauv::gui;

ProcessNode::ProcessNode(const nid_t id) : BooleanNode(id){
}

void ProcessNode::setStats(float cpu, float mem, unsigned int threads, const std::string& status)
{
    findOrCreate<NumericNode<float> >("cpu")->typedUpdate(cpu);
    findOrCreate<NumericNode<float> >("mem")->typedUpdate(mem);
    findOrCreate<NumericNode<unsigned int> >("threads")->typedUpdate(threads);
    findOrCreate<StringNode>("status")->update(status);
}

ProcessNode::~ProcessNode(){
    info() << "~ProcessNode()";
}

