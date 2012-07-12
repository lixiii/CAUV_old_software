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

#ifndef REGISTRY_H
#define REGISTRY_H

#include <QtGui>

#include <boost/shared_ptr.hpp>

#include <gui/core/model/nodes/groupingnode.h>
#include <gui/core/model/nodes/vehiclenode.h>

namespace cauv {

    class Message;
    class MessageObserver;

    namespace gui {

        class VehicleRegistry : public GroupingNode
        {
            Q_OBJECT
            public:
                static boost::shared_ptr<VehicleRegistry> instance()
                {
                    static boost::shared_ptr<VehicleRegistry> m_instance(new VehicleRegistry());
                    return m_instance;
                }

                template <class T> boost::shared_ptr<T> registerVehicle(std::string name){
                    boost::shared_ptr<Vehicle> vehicle(new T(name));
                    connect(vehicle.get(), SIGNAL(observerAttached(boost::shared_ptr<MessageObserver>)),
                            this, SIGNAL(observerAttached(boost::shared_ptr<MessageObserver>)));
                    connect(vehicle.get(), SIGNAL(observerDetached(boost::shared_ptr<MessageObserver>)),
                            this, SIGNAL(observerDetached(boost::shared_ptr<MessageObserver>)));
                    connect(vehicle.get(), SIGNAL(messageGenerated(boost::shared_ptr<const Message>)),
                            this, SIGNAL(messageGenerated(boost::shared_ptr<const Message>)));
                    vehicle->initialise();
                    addChild(vehicle);
                    Q_EMIT vehicleAdded(vehicle);
                    return boost::static_pointer_cast<T>(vehicle);
                }

                const std::vector<boost::shared_ptr<Vehicle> > getVehicles() const;
                const boost::shared_ptr<Node> getNode(QUrl url);

            Q_SIGNALS:
                void messageGenerated(boost::shared_ptr<const Message>);
                void observerAttached(boost::shared_ptr<MessageObserver>);
                void observerDetached(boost::shared_ptr<MessageObserver>);
                void vehicleAdded(boost::shared_ptr<Vehicle>);

            private:
                VehicleRegistry();
                VehicleRegistry(VehicleRegistry const&);
                void operator=(VehicleRegistry const&);
        };

    } // namespace gui
} // namespace cauv


#endif // REGISTRY_H