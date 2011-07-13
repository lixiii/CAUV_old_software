#ifndef PIPELINECAUVWIDGET_H
#define PIPELINECAUVWIDGET_H

#include <boost/shared_ptr.hpp>

#include <QWidget>

#include <gui/core/cauvbasicplugin.h>

#include <generated/message_observers.h>

#include <common/cauv_utils.h>

namespace Ui {
    class PipelineCauvWidget;
}

namespace cauv {
    
    namespace pw {
        class PipelineWidget;
        class PipelineGuiMsgObs;
    }

    namespace gui {
        
        class PipelineListingObserver : public QObject, public MessageObserver {
            Q_OBJECT
        public:
            PipelineListingObserver(boost::shared_ptr<CauvNode> node);
            virtual void onMembershipChangedMessage(MembershipChangedMessage_ptr m);
            virtual void onPipelineDiscoveryResponseMessage(PipelineDiscoveryResponseMessage_ptr m);
            virtual void onPipelineDiscoveryRequestMessage(PipelineDiscoveryRequestMessage_ptr m);
            
        protected:
            boost::shared_ptr<CauvNode> m_node;
            RateLimiter m_rate_limiter;
            
        Q_SIGNALS:
            void pipelineDiscovered(std::string name);
            void searchStarted();
        };
        
        
        
        class PipelineCauvWidget : public QWidget, public CauvBasicPlugin
        {
            Q_OBJECT
            Q_INTERFACES(cauv::gui::CauvInterfacePlugin)
            
        public:
                    PipelineCauvWidget();
            virtual ~PipelineCauvWidget();
            
            virtual const QString name() const;
            virtual const QList<QString> getGroups() const;
            virtual void initialise(boost::shared_ptr<AUV>, boost::shared_ptr<CauvNode> node);
            
        protected Q_SLOTS:
            void send(boost::shared_ptr<const Message> message);
            void addPipeline(std::string name);
            void clearPipelines();
            
        protected:
            pw::PipelineWidget * m_pipeline;
            boost::shared_ptr< pw::PipelineGuiMsgObs> m_observer;
            
        private:
            Ui::PipelineCauvWidget * ui;
        };
    }  // namespace gui
    
} // namespace cauv

#endif // PIPELINECAUVWIDGET_H
