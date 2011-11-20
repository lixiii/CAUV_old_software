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

#ifndef DATASTREAMDISPLAYS_H
#define DATASTREAMDISPLAYS_H

#include <QTreeWidget>
#include <QKeyEvent>

#include <debug/cauv_debug.h>

#include "../model/model.h"


namespace Ui {
    class NodePicker;
}


namespace cauv {
    namespace gui {

        class NodeTreeItemBase;


        class NodePathFilter : public QObject, public NodeFilterInterface {
            Q_OBJECT
        public:
            NodePathFilter(QObject * parent = NULL);

        public Q_SLOTS:
            void setText(QString const& string);
            QString getText();
            bool filter(boost::shared_ptr<Node> const& node);

        protected:
            QString m_text;
            bool containsText(boost::shared_ptr<Node> const& node);

        Q_SIGNALS:
            void filterChanged();
        };


        class NodeListView : public QTreeWidget{
            Q_OBJECT
        public:
            NodeListView(QWidget * parent);
            virtual void registerListFilter(boost::shared_ptr<NodeFilterInterface> const& filter);
            void mousePressEvent(QMouseEvent *event);
            void mouseMoveEvent(QMouseEvent *event);

        private Q_SLOTS:
            void editStarted(QTreeWidgetItem* item, int column);
            void itemEdited(QTreeWidgetItem* item, int column);
            void applyFilters();
            void applyFilters(NodeTreeItemBase *);
            bool applyFilters(boost::shared_ptr<Node> const&);

        Q_SIGNALS:
            void onKeyPressed(QKeyEvent *event);

        protected:
            QPoint m_dragStartPosition;
            std::vector<boost::shared_ptr<NodeFilterInterface> > m_filters;
            void keyPressEvent(QKeyEvent *event);
        };



        class NodePicker : public QWidget {
            Q_OBJECT

        public:
            NodePicker(boost::shared_ptr<Vehicle> const& auv);
            virtual ~NodePicker();

        protected Q_SLOTS:
            void redirectKeyboardFocus(QKeyEvent* key);

        protected:
            boost::shared_ptr<NodeTreeItemBase> m_root;

        private:
            Ui::NodePicker *ui;
        };

    } // namespace gui
} // namespace cauv


#endif // DATASTREAMDISPLAYS_H
