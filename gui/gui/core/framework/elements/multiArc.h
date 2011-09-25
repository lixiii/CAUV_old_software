#ifndef __CAUV_ELEMENT_MULTIARC_H__
#define __CAUV_ELEMENT_MULTIARC_H__

#include <QObject>
#include <QGraphicsPathItem>

#include "fluidity/managedElement.h"

#include "arc.h"

namespace liquid {
    struct ArcStyle;
}

namespace cauv{
namespace gui{

class MultiArcEnd;

class MultiArc : public QObject,
                 public QGraphicsPathItem,
                 public ManagedElement{
        Q_OBJECT
    public:
        // From is used as parent. Magics are done so that coordinates are all
        // relative to parent for drawing.
        MultiArc(ManagedElement const& m,
                 liquid::ArcStyle const& style,
                 ConnectableInterface *from,
                 ConnectableInterface *to=NULL);
        
        liquid::ArcStyle const& style() const;

        void addTo(ConnectableInterface *to);
        void removeTo(ConnectableInterface *to);
        
    // QGraphicsItem:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual QRectF boundingRect() const;

    protected Q_SLOTS:
        void updateLayout();

    protected:
        QList<ConnectableInterface*> m_to;
        ConnectableInterface *m_from;

        QGraphicsPathItem *m_front;

        MultiArcEnd *m_ephemeral_arc_end;

        liquid::ArcStyle const& m_style;
};

} // namespace gui
} // namespace cauv


#endif // ndef __CAUV_ELEMENT_MULTIARC_H__
