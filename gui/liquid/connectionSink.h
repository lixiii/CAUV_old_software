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

#ifndef __LIQUID_CONNECTION_SINK_H__
#define __LIQUID_CONNECTION_SINK_H__

namespace liquid {

class ConnectionSink{
    public:
        // called whilst a drag operation is in progress to test & highlight things
        virtual bool willAcceptConnection(void* from_source) = 0;

        // called when the connection is dropped, may return:
        enum ConnectionStatus{Rejected, Accepted, Pending};
        virtual ConnectionStatus doAcceptConnection(void* from_source) = 0;
};

} // namespace liquid

#endif //  __LIQUID_CONNECTION_SINK_H__
