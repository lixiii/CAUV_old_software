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

#include "view.h"

#include <QDebug>
#include <QGraphicsRectItem>
#include <QAction>

#include <boost/make_shared.hpp>

#include <liquid/button.h>
//#include <liquid/arc.h>
//#include <liquid/arcSource.h>
//#include <liquid/arcSink.h>

#include <utility/string.h>
#include <generated/types/NodeType.h>

#include "elements/style.h"
#include "style.h"

#include "fluidity/fNode.h"
#include "fluidity/managedElement.h"
#include "fluidity/manager.h"
#include "fluidity/menu.h"

#include <debug/cauv_debug.h>

using namespace cauv;
using namespace cauv::gui::f;
using namespace liquid;

FView::FView(boost::shared_ptr<CauvNode> node, QWidget* parent)
    : LiquidView(parent),
      m_cauv_node(node),
      m_manager(),
      m_contextmenu_actions(){
    buildMenus();

    QGraphicsScene *s = new QGraphicsScene(this);

    // !!! is this really what we want to do?
    // items aren't added or removed a lot, just updated
    //s->setItemIndexMethod(QGraphicsScene::NoIndex);
    s->setSceneRect(-200,-200,400,400);

    setScene(s);
    m_manager = boost::make_shared<Manager>(s, &(*m_cauv_node), "default");
    m_manager->init();
    
    setMinimumSize(1280, 720);
    setWindowTitle("QGraphicsScene Element Test");
    
    Button *b;
    b = new Button(QRectF(0,0,24,24), QString(":/resources/icons/dup_button"));
    b->setPos(170,176);
    s->addItem(b);
    
    b = new Button(QRectF(0,0,24,24), QString(":/resources/icons/x_button"));
    b->setPos(146,176);
    s->addItem(b);
    
    b = new Button(QRectF(0,0,24,24), QString(":/resources/icons/collapse_button"));
    b->setPos(122,176);
    s->addItem(b);
    
    b = new Button(QRectF(0,0,24,24), QString(":/resources/icons/reexec_button"));
    b->setPos(98,176);
    s->addItem(b);
}

void FView::buildMenus(){
    for(int i = 0; i < NodeType::NumValues; i++){
        QString n = QString::fromStdString(mkStr() << NodeType::e(i));
        n.replace("NodeType::","");
        n += "Node";
        QAction* a = new QAction(n, this);
        // ... TODO: connect signals
        m_contextmenu_actions << a;
    }
    /*
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);
    */
}

void FView::contextMenuEvent(QContextMenuEvent *event){
    debug() << "contextMenuEvent";

    cauv::gui::f::Menu menu(this);
    foreach(QAction* a, m_contextmenu_actions)
        menu.addAction(a);

    menu.exec(event->globalPos());
}

