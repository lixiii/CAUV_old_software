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

#ifndef __CAUV_AIMESSAGEGENERATORS_H__
#define __CAUV_AIMESSAGEGENERATORS_H__

#include <QObject>

#include <generated/types/message.h>

#include <boost/shared_ptr.hpp>

#include <gui/core/model/node.h>

#include <gui/core/model/messagegenerators.h>

namespace cauv {
    namespace gui {

        class Node;

        class AiTaskMessageGenerator : public MessageGenerator {
            Q_OBJECT
        public Q_SLOTS:
            boost::shared_ptr<const Message> generate(boost::shared_ptr<Node> attachedTo);
        };

        class AiConditionMessageGenerator : public MessageGenerator {
            Q_OBJECT
        public Q_SLOTS:
            boost::shared_ptr<const Message> generate(boost::shared_ptr<Node> attachedTo);
        };

    } // namespace gui
} // namesapce cauv

#endif // __CAUV_AIMESSAGEGENERATORS_H__
