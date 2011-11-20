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

#ifndef GUI_MODEL_H
#define GUI_MODEL_H

#include <QAbstractItemModel>

#include "node.h"

#include <generated/types/message.h>

#include "../model/nodes/groupingnode.h"



namespace cauv {
    namespace gui {

        class MessageGenerator;

        class Vehicle : public GroupingNode
        {
            Q_OBJECT

        public:
            friend class VehicleRegistry;

        protected:
            Vehicle(std::string name) : GroupingNode(name) {
            }

            virtual void initialise() = 0;

        Q_SIGNALS:
            void messageGenerated(boost::shared_ptr<const Message>);

        protected:
            void addGenerator(boost::shared_ptr<MessageGenerator> generator);

            std::vector<boost::shared_ptr<MessageGenerator> > m_generators;
        };


        class RedHerring : public Vehicle
        {
            Q_OBJECT

        public:
            friend class VehicleRegistry;

        protected:
            RedHerring(std::string name);
            virtual void initialise();

        protected Q_SLOTS:
            void setupMotor(boost::shared_ptr<NodeBase>);
            void setupAutopilot(boost::shared_ptr<NodeBase> node);
        };














        class VehicleItemModel : public QAbstractItemModel {
            Q_OBJECT

        public:
            VehicleItemModel(boost::shared_ptr<NodeBase> root, QObject * parent = 0) :
                QAbstractItemModel(parent), m_root(root){
            }

            Qt::ItemFlags flags(const QModelIndex &index) const{
                void * ptr = index.internalPointer();
                NodeBase * node = static_cast<NodeBase *>(ptr);
                if(node->isMutable()) return QAbstractItemModel::flags(index) & Qt::ItemIsEditable;
                else return QAbstractItemModel::flags(index);
            }

            QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const {

                void * ptr = index.internalPointer();
                NodeBase * node = static_cast<NodeBase *>(ptr);

                switch (role){
                case Qt::DisplayRole:
                    return QVariant(QString::fromStdString(node->nodeName()));
                break;
                case Qt::DecorationRole:
                break;
                case Qt::EditRole:
                    return QVariant(QString::fromStdString(node->nodePath()));
                break;
                case Qt::ToolTipRole:
                    return QVariant(QString::fromStdString(node->nodePath()));
                break;
                case Qt::StatusTipRole:
                break;
                case Qt::WhatsThisRole:
                break;
                case Qt::SizeHintRole:
                break;
                case Qt::FontRole:
                break;
                case Qt::TextAlignmentRole:
                break;
                case Qt::BackgroundRole:
                break;
                case Qt::ForegroundRole:
                break;
                case Qt::CheckStateRole:
                break;
                case Qt::AccessibleTextRole:
                break;
                case Qt::AccessibleDescriptionRole:
                break;
                default: break;
                }
                return QVariant();
            }

            QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const {

                Q_UNUSED(orientation);

                switch (role){
                case Qt::DisplayRole:
                    return QString(section);
                break;
                case Qt::DecorationRole:
                break;
                case Qt::EditRole:
                    return QString(section);
                break;
                case Qt::ToolTipRole:
                    return QString(section);
                break;
                case Qt::StatusTipRole:
                break;
                case Qt::WhatsThisRole:
                break;
                case Qt::SizeHintRole:
                break;
                case Qt::FontRole:
                break;
                case Qt::TextAlignmentRole:
                break;
                case Qt::BackgroundRole:
                break;
                case Qt::ForegroundRole:
                break;
                case Qt::CheckStateRole:
                break;
                case Qt::AccessibleTextRole:
                break;
                case Qt::AccessibleDescriptionRole:
                break;
                default: break;
                }
                return QVariant();
            }


            int rowCount ( const QModelIndex & parent = QModelIndex() ) const {
                NodeBase *parentItem;
                if (!parent.isValid())
                    parentItem = m_root.get();
                else
                    parentItem = static_cast<NodeBase*>(parent.internalPointer());

                return parentItem->getChildren().size();
            }

            int columnCount(const QModelIndex &/*parent*/) const
            {
                // !!! todo: variable column sizes
                return 2;
            }

            QModelIndex parent(const QModelIndex &child) const {
                if (!child.isValid())
                    return QModelIndex();

                NodeBase * childNode = static_cast<NodeBase *>(child.internalPointer());

                try {
                    // work out the row of the parent with respect to its siblings
                    NodeBase * parent = childNode->getParent().get();
                    NodeBase * parentsParent;
                    try {
                        parentsParent = parent->getParent().get();
                    } catch (std::out_of_range){
                        parentsParent = m_root.get();
                    }

                    int row = 0;
                    foreach (boost::shared_ptr<NodeBase> const & c, parentsParent->getChildren()){
                        if (c.get() == parent) break;
                        row++;
                    }
                    // column is 0 because other columns don't ever have children in this
                    // model
                    return createIndex(row, 0, parent);

                } catch (std::out_of_range ex){
                        return QModelIndex();
                }
            }

            QModelIndex index(int row, int column, const QModelIndex &parent) const {
                if (!hasIndex(row, column, parent))
                    return QModelIndex();

                NodeBase * parentNode;

                if(!parent.isValid())
                    parentNode = m_root.get();
                else parentNode = static_cast<NodeBase *>(parent.internalPointer());

                try {
                    NodeBase * childNode = parentNode->getChildren().at(row).get();
                    return createIndex(row, column, childNode);
                } catch (std::out_of_range){
                    return QModelIndex();
                }
            }

        protected:
            boost::shared_ptr<NodeBase> m_root;

        };















    } // namespace gui
} // namespace cauv

#endif // GUI_MODEL_H
