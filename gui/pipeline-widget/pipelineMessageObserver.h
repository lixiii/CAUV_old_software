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

#ifndef __PIPELINE_MESSAGE_OBSERVER_H__
#define __PIPELINE_MESSAGE_OBSERVER_H__

#include <common/cauv_node.h>
#include <generated/message_observers.h>

#include <QObject>

#include "pwTypes.h"

namespace cauv{
namespace gui{
namespace pw{

struct DBGLevelObserver: MessageObserver
{
    void onDebugLevelMessage(DebugLevelMessage_ptr m);
};

class PipelineGuiMsgObs: public BufferedMessageObserver{
    public:
        PipelineGuiMsgObs(PipelineWidget *p);

        virtual void onNodeAddedMessage(NodeAddedMessage_ptr m);
        virtual void onNodeRemovedMessage(NodeRemovedMessage_ptr m);
        virtual void onNodeParametersMessage(NodeParametersMessage_ptr m);
        virtual void onArcAddedMessage(ArcAddedMessage_ptr m);
        virtual void onArcRemovedMessage(ArcRemovedMessage_ptr m);
        virtual void onGraphDescriptionMessage(GraphDescriptionMessage_ptr m);
        virtual void onStatusMessage(StatusMessage_ptr m);
        virtual void onInputStatusMessage(InputStatusMessage_ptr m);
        virtual void onOutpuStatusMessage(OutputStatusMessage_ptr m);
        virtual void onGuiImageMessageBuffered(GuiImageMessage_ptr m);

    private:
        template<typename message_T>
        bool _nameMatches(boost::shared_ptr<const message_T> msg);

        PipelineWidget *m_widget;
};

} // namespace pw
} // namespace gui
} // namespace cauv

#endif // ndef __PIPELINE_MESSAGE_OBSERVER_H__
