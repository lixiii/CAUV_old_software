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

#ifndef __LIQUID_ARC_SOURCE_H__
#define __LIQUID_ARC_SOURCE_H__

#include <QGraphicsObject>
#include <QGraphicsLayoutItem>
#include <QGraphicsSceneMouseEvent>

namespace liquid {

struct ArcStyle;
class AbstractArcSink;
class Arc;

class AbstractArcSource: public QGraphicsObject{
    Q_OBJECT
    public:
        AbstractArcSource(ArcStyle const& of_style,
                          void* sourceDelegate,
                          Arc* arc);
        virtual ~AbstractArcSource();

        Arc* arc() const;
        ArcStyle const& style() const;
    
    Q_SIGNALS:
        void geometryChanged();

    protected:
        // !!! not implementing the GraphicsItem required functions: this is an
        // abstract base

        // Handles creating new arcs by dragging:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *e);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *e);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *e);
        
        // used to create the temporary ConnectionSink* to which the arc is
        // connected during a drag operation: this may be overridden by derived
        // classes if they want to drastically change the style
        virtual AbstractArcSink* newArcEnd();
        
        void removeHighlights();
        void checkAndHighlightSinks(QPointF scene_pos);

     protected:
        // ...style, currently highlighted scene item, connection source (which
        // may be this), ...
        ArcStyle const& m_style;  
        Arc *m_arc;
        void *m_sourceDelegate;
        AbstractArcSink *m_ephemeral_sink;
        QSet<AbstractArcSink*> m_highlighted_items;
};

class ArcSource: public AbstractArcSource,
                 public QGraphicsLayoutItem{
    public:
        ArcSource(void* sourceDelegate,
                  Arc* arc);
        
        virtual QSizeF sizeHint(Qt::SizeHint which,
                                const QSizeF &constraint=QSizeF()) const;
        virtual void setGeometry(QRectF const& rect);
    protected:
        virtual QRectF boundingRect() const;
        virtual void paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *opt,
                           QWidget *widget=0);
        
    private:
        QGraphicsLineItem *m_front_line;
        QGraphicsLineItem *m_back_line;
};

} // namespace liquid

#endif // __LIQUID_ARC_SOURCE_H__

