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

#ifndef __LIQUID_NODE_H__
#define __LIQUID_NODE_H__

#include <QGraphicsObject>

class QGraphicsLayoutItem;
class QGraphicsLinearLayout;

namespace liquid {

struct NodeStyle;

class Button;
class ResizeHandle;
class NodeHeader;
class RequiresCutout;
class Shadow;

class LiquidNode: public QGraphicsObject{
    Q_OBJECT

public:
    LiquidNode(NodeStyle const& style, QGraphicsItem *parent=0);
    virtual ~LiquidNode();

Q_SIGNALS:
    void closed(LiquidNode *);
public Q_SLOTS:
    virtual void close();

protected:
    virtual QRectF boundingRect() const;
    virtual QPainterPath shape() const;
    virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget *w=0);
    
    // override to limit node positions to whole numbers of px - so that thin
    // lines and text are drawn nicely
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);

public:
    virtual QSizeF size() const;
    virtual void setSize(QSizeF const&);
    
    virtual void addButton(QString name, Button * button);
    virtual void addItem(QGraphicsLayoutItem *item);
    virtual void removeItem(QGraphicsLayoutItem *item);

    virtual void setClosable(bool);
    virtual void setResizable(bool);

    virtual NodeStyle style() const;
    virtual NodeHeader* header() const;

    enum Status{ NotOK, OK };
    Status status() const;
    virtual void status(Status const& s, std::string const& status_information="");

protected:
    void layoutChanged();

protected Q_SLOTS:
    void updateLayout();
    void resized();

private:
    void setSizeFromContents();

protected:
    QSizeF m_size;

    NodeHeader            *m_header;
    QGraphicsWidget       *m_buttonsWidget;
    ResizeHandle          *m_resizeHandle;
    QGraphicsWidget       *m_contentWidget;
    QGraphicsLinearLayout *m_contentLayout;
    QGraphicsPathItem     *m_back;
    Shadow                *m_status_highlight;
    // ...
    //QVector<QGraphicsLineItem*> m_separators;
    QList<RequiresCutout*> m_items_requiring_cutout;

    NodeStyle const& m_style;

    Status m_status;

    static QSizeF Minimum_Size;
};

} // namespace liquid

#endif // __LIQUID_NODE_H__
