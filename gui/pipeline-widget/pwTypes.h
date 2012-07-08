/* Copyright 2011 Cambridge Hydronautics Ltd.
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

#ifndef __PW_TYPES_H__
#define __PW_TYPES_H__

/* enum definitions, forward declarations and pointer typedefs for all
 * PipelineWidget and Renderable types
 */

#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace cauv{
namespace gui{
namespace pw{

namespace drawtype_e{
enum e{
    no_flags = 0x0,
    picking  = 0x1
};
} // namespace drawtype_e

class PipelineWidget;

class Container;

class Renderable;
class Node;
class Menu;
class Arc;
class Text;
class NodeIOBlob;
class NodeInputBlob;
class NodeInputParamBlob;
class NodeOutputBlob;
class FloatingArcHandle;
class ImgNode;
class PVPairEditableBase;
template<typename value_T> class PVPair;


typedef PipelineWidget* pw_ptr_t;
typedef Container* container_ptr_t;

typedef boost::weak_ptr<Renderable> renderable_wkptr_t;
typedef boost::shared_ptr<Renderable> renderable_ptr_t;
typedef boost::weak_ptr<Node> node_wkptr_t;
typedef boost::shared_ptr<Node> node_ptr_t;
typedef boost::weak_ptr<Arc> arc_wkptr_t;
typedef boost::shared_ptr<Arc> arc_ptr_t;
typedef boost::shared_ptr<Menu> menu_ptr_t;
typedef boost::shared_ptr<Text> text_ptr_t;
typedef boost::shared_ptr<ImgNode> imgnode_ptr_t;

// TODO: this should really be synchronised with pipelineTypes.h!
typedef int32_t node_id;

// Types for OverKey:
namespace ok{

namespace keystate_e{
enum e{
    released = 0x0,
    pressed = 0x1,
    num_values = 0x2
};
} // namespace keystate_e

class Action;
class Key;
class OverKey;

typedef boost::shared_ptr<Action> action_ptr_t;
typedef boost::shared_ptr<Key> key_ptr_t;
typedef boost::shared_ptr<OverKey> overlay_ptr_t;

} // namespace ok

} // namespace pw
} // namespace gui
} // namespace cauv

#endif // ndef __PW_TYPES_H__

