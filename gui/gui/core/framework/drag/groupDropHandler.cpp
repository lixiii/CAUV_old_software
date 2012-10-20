#include "groupDropHandler.h"

#include <liquid/node.h>
#include <liquid/nodeHeader.h>
#include <liquid/water/dataSeries.h>

#include "model/node.h"
#include "model/model.h"
#include "model/nodes/groupingnode.h"

#include "framework/elements/style.h"
#include "framework/nodepicker.h"

using namespace cauv;
using namespace cauv::gui;

GroupDropHandler::GroupDropHandler(boost::shared_ptr<NodeItemModel> model) :
    m_model(model){
}

bool GroupDropHandler::accepts(boost::shared_ptr<Node> const& node){
    return node->type == nodeType<GroupingNode>();
}

QGraphicsItem * GroupDropHandler::handle(boost::shared_ptr<Node> const& node) {

    liquid::LiquidNode * ln = new liquid::LiquidNode(AI_Node_Style());
    ln->header()->setTitle(QString::fromStdString(node->nodeName()));
    ln->header()->setInfo(QString::fromStdString(node->nodePath()));
    NodeTreeView * view = new NodeTreeView(true);
    view->setModel((QAbstractItemModel*)m_model.get());
    view->setRootIndex(m_model->indexFromNode(node));
    QGraphicsProxyWidget * proxy = new QGraphicsProxyWidget();
    proxy->setWidget(view);

    ln->addItem(proxy);

    return ln;
}
