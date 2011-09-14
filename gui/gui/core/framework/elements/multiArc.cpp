#include "multiArc.h"

#include <QPainterPath>
#include <QPointF>
#include <QPen>
#include <QGraphicsSceneMouseEvent>

#include <utility/rounding.h>
#include <utility/qt_streamops.h>

#include <cassert>

#include <debug/cauv_debug.h>

#include "multiArcEnd.h"

using namespace cauv;
using namespace cauv::gui;

const static QColor Line_Colour = QColor(134, 217, 241);
const static qreal Thickness = 4.0;
const static qreal Lead_In_Length = 8.0;


MultiArc::MultiArc(ConnectableInterface *from, ConnectableInterface *to)
    : m_to(),
      m_from(from),
      m_ephemeral_arc_end(NULL){
    assert(from);
    QGraphicsObject *from_as_go = from->asQGraphicsObject();
    assert(from_as_go);
    setParentItem(from_as_go);

    connect(from_as_go, SIGNAL(boundriesChanged()), this, SLOT(updateLayout()));
    if(to){
        addTo(to);
    }
    updateLayout();
}

QColor MultiArc::startColour(){
    return Line_Colour;
}

QColor MultiArc::endColour(){
    return Line_Colour;
}

void MultiArc::addTo(ConnectableInterface* to){
    assert(to);
    m_to.push_back(to);
    QGraphicsObject *to_as_go = to->asQGraphicsObject();
    connect(to_as_go, SIGNAL(boundriesChanged()), this, SLOT(updateLayout()));
    connect(to_as_go, SIGNAL(disconnected()), this, SLOT(updateLayout()));
    updateLayout();
}

void MultiArc::removeTo(ConnectableInterface* to){
    m_to.removeOne(to);
    assert(m_to.indexOf(to) == -1);
    QGraphicsObject *to_as_go = to->asQGraphicsObject();    
    disconnect(to_as_go, 0, this, 0);
    updateLayout();
}

void MultiArc::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->button() & Qt::LeftButton){
        assert(!m_ephemeral_arc_end);
        m_ephemeral_arc_end = new MultiArcEnd(this, true);
        // in item (this) coordinates (this is now parent of new end):
        m_ephemeral_arc_end->setPos(event->pos() - m_ephemeral_arc_end->boundingRect().center()); 
        // accepted -> forward to base explicitly
        //QGraphicsObject::mousePressEvent(event); 
    }else{
        event->ignore();
    }
}

void MultiArc::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_ephemeral_arc_end){
        m_ephemeral_arc_end->mouseMoveEvent(event);
    }else{
        event->ignore();
    }
}

void MultiArc::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(event->button() & Qt::LeftButton){
        if(m_ephemeral_arc_end)
            m_ephemeral_arc_end->mouseReleaseEvent(event);
        // deletion arranged by mouseReleaseEvent
        m_ephemeral_arc_end = NULL;
        // accepted -> forward to base explicitly
        QGraphicsObject::mouseReleaseEvent(event);    
    }else{
        event->ignore();
    }
}

QRectF MultiArc::boundingRect() const{
    return QGraphicsPathItem::boundingRect() | childrenBoundingRect();
}

void MultiArc::updateLayout(){
    // we draw in m_from's coordinate system (it is this's parent)
    QPointF start_point = m_from->connectionPoint();
    QPointF split_point = start_point + QPointF(8,0);
    
    QBrush brush(Line_Colour);
    setPen(QPen(brush, Thickness));

    QPainterPath path(start_point);
    path.lineTo(split_point);

    if(m_to.size()){
        foreach(ConnectableInterface* ci, m_to){
            QGraphicsObject* o = ci->asQGraphicsObject();
            path.moveTo(split_point);
            // mapToItem maps connection point in 'to' object into m_from's
            // coordinates 
            QPointF end_point(o->mapToItem(m_from->asQGraphicsObject(), ci->connectionPoint()));
            QPointF c1(split_point + QPointF(5+std::fabs(end_point.x() - split_point.x())/2, 0));
            QPointF c2(end_point   - QPointF(5+std::fabs(end_point.x() - split_point.x())/2, 0));
            path.cubicTo(c1, c2, end_point);
            //debug() << "f=" << start_point << "s=" << split_point << "t=" << end_point;
            //path.lineTo(end_point);
        }
    }else{
        path.lineTo(split_point + QPointF(4,0));
    }

    setPath(path);
}
