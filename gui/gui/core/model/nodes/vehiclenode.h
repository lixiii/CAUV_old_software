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

#ifndef GUI_VEHICLENODE_H
#define GUI_VEHICLENODE_H

#include <gui/core/model/messaging.h>

#include <generated/types/message.h>

namespace cauv {
    namespace gui {

        class BaseMessageGenerator;

        class Vehicle : public Node
        {
            Q_OBJECT

        public:
            friend class VehicleRegistry;

        protected:
            Vehicle(std::string name) : Node(name, nodeType<Vehicle>()) {
            }

            virtual void initialise() = 0;

        Q_SIGNALS:
            void messageGenerated(boost::shared_ptr<const Message>);
            void observerAttached(boost::shared_ptr<MessageObserver>);
            void observerDetached(boost::shared_ptr<MessageObserver>);

        public:

            typedef std::set<boost::shared_ptr<BaseMessageGenerator> > generator_set_t;
            typedef std::set<boost::shared_ptr<MessageObserver> > observer_set_t;

            void attachGenerator(boost::shared_ptr<BaseMessageGenerator> generator)
            {
                connect(generator->node().get(), SIGNAL(detachedFrom(boost::shared_ptr<Node>)),
                        this, SLOT(nodeRemoved()));

                generator->connect(generator.get(), SIGNAL(messageGenerated(boost::shared_ptr<const Message>)),
                                   this, SIGNAL(messageGenerated(boost::shared_ptr<const Message>)));
                if(dynamic_cast<MessageObserver*>(generator.get())) {
                    attachObserver(generator->node(), boost::dynamic_pointer_cast<MessageObserver>(generator));
                }

                m_generators[generator->node()].insert(generator);
            }

            void detachGenerators(boost::shared_ptr<Node> node){
                info() << "detach generators";
                foreach(boost::shared_ptr<BaseMessageGenerator> const& generator, m_generators[node]){
                    info() << "generator found";
                    m_generators[node].erase(generator);
                }
                m_generators.erase(node);
            }

            void attachObserver(boost::shared_ptr<Node> node,
                                boost::shared_ptr<MessageObserver> observer) {
                m_observers[node].insert(observer);
                Q_EMIT observerAttached(observer);
            }

            void detachObservers(boost::shared_ptr<Node> node) {
                info() << "detach observers";
                foreach(boost::shared_ptr<MessageObserver> const& observer, m_observers[node]){
                    info() << "observer found";
                    m_observers[node].erase(observer);
                    Q_EMIT observerDetached(observer);
                }
                m_observers.erase(node);
            }

        protected Q_SLOTS:
            void nodeRemoved() {
                info() << "node removed";
                if(Node* node = dynamic_cast<Node*>(sender())) {
                    detachGenerators(node->shared_from_this());
                    detachObservers(node->shared_from_this());
                }
            }

        protected:
            std::map<boost::shared_ptr<Node>,  generator_set_t > m_generators;
            std::map<boost::shared_ptr<Node>,  observer_set_t > m_observers;
        };

    } //namespace gui
} // namespace cauv

#endif // GUI_VEHICLENODE_H