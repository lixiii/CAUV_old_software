#include "cauvgui.h"

#include <model/auv_controller.h>
#include <model/auv_model.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include "widgets/datastreamdisplays.h"
#include "widgets/pipelinecauvwidget.h"
#include "widgets/motorcontrols.h"

#include <common/cauv_global.h>
#include <common/cauv_utils.h>
#include <debug/cauv_debug.h>

#include "ui_mainwindow.h"
#include "cauvinterfaceelement.h"

using namespace cauv;

CauvGui::CauvGui(const QApplication& app) : CauvNode("CauvGui"), m_application(app), ui(new Ui::MainWindow){
    ui->setupUi(this);
    joinGroup("control");
    joinGroup("pl_gui");
}

void CauvGui::addInterfaceElement(boost::shared_ptr<CauvInterfaceElement> widget){
    connect(widget->actions().get(), SIGNAL(messageGenerated(boost::shared_ptr<Message>)), this, SLOT(send(boost::shared_ptr<Message>)));
    connect(widget->actions().get(), SIGNAL(centralViewRegistered(QWidget*,QString&)), this, SLOT(addCentralTab(QWidget*,QString&)));
    connect(widget->actions().get(), SIGNAL(dockViewRegistered(QDockWidget*,Qt::DockWidgetArea)), this, SLOT(addDock(QDockWidget*,Qt::DockWidgetArea)));
    widget->initialise();
}

void CauvGui::addCentralTab(QWidget* tab, QString& name){
    info() << "Registering central screen [" << name.toStdString() << "]";
    ui->tabWidget->addTab(tab, name);
}

void CauvGui::addDock(QDockWidget* dock, Qt::DockWidgetArea area){
    info() << "Registering dock widget";
    addDockWidget(area, dock);
}

void CauvGui::closeEvent(QCloseEvent*){
    hide();
    CauvNode::stopNode();
}

int CauvGui::send(boost::shared_ptr<Message> message){
    debug(0) << "Sending message: " << *message;
    return CauvNode::send(message);
}

void CauvGui::onRun()
{
    CauvNode::onRun();

    // currently the gui can only connect to the auv when it starts up.
    // this should really be made ocnfigurable via a shiny interface
    // this would required some changes to cauv node classes to be possible


    // set up the auv object along with a controller to update the state
    // from the network messages
    m_auv = boost::make_shared<AUV>();
    m_auv_controller = boost::make_shared<AUVController>(m_auv);
    // connect up message inputs and outputs
    addMessageObserver(m_auv_controller);
    m_auv_controller->onMessageGenerated.connect(boost::bind(&CauvGui::send, this, _1));

    // populate the interface
    boost::shared_ptr<DataStreamDisplayArea> graphArea(new DataStreamDisplayArea("Stream Visualisation", m_auv, this, shared_from_this()));
    addInterfaceElement(boost::static_pointer_cast<CauvInterfaceElement>(graphArea));

    boost::shared_ptr<PipelineCauvWidget> pipelineArea(new PipelineCauvWidget("Pipeline Editor", m_auv, this, shared_from_this()));
    addInterfaceElement(boost::static_pointer_cast<CauvInterfaceElement>(pipelineArea));

    boost::shared_ptr<DataStreamPicker> dataStreamPicker(new DataStreamPicker("Data Streams", m_auv, this, shared_from_this()));
    addInterfaceElement(boost::static_pointer_cast<CauvInterfaceElement>(dataStreamPicker));

    boost::shared_ptr<MotorControls> motorControls(new MotorControls("Navigation", m_auv, this, shared_from_this()));
    addInterfaceElement(boost::static_pointer_cast<CauvInterfaceElement>(motorControls));

    show();
    m_application.exec();
}
