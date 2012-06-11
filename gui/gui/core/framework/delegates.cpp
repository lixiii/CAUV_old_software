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

#include "delegates.h"

#include <QItemEditorCreator>
#include <QApplication>

#include "model/nodes/numericnode.h"
#include "widgets/neutralspinbox.h"
#include "widgets/onoff.h"
#include "widgets/graphbar.h"
#include "model/utils/sampler.h"
#include "framework/style.h"

using namespace cauv;
using namespace cauv::gui;

#include "nodepicker.h"


NodeDelegateMapper::NodeDelegateMapper(QObject *):
    m_hasSizeHint(false){
}

void NodeDelegateMapper::registerDelegate(node_type nodeType, boost::shared_ptr<QAbstractItemDelegate> delegate){
    m_default_delegates[nodeType] = delegate;
    connect(delegate.get(), SIGNAL(commitData(QWidget*)), this, SIGNAL(commitData(QWidget*)));
    connect(delegate.get(), SIGNAL(closeEditor(QWidget*)), this, SIGNAL(closeEditor(QWidget*)));
    connect(delegate.get(), SIGNAL(sizeHintChanged(QModelIndex)), this, SIGNAL(sizeHintChanged(QModelIndex)));
}

void NodeDelegateMapper::registerDelegate(boost::shared_ptr<Node> node, boost::shared_ptr<QAbstractItemDelegate> delegate){
    m_delegates[node] = delegate;
}

boost::shared_ptr<QAbstractItemDelegate> NodeDelegateMapper::getDelegate(boost::shared_ptr<Node> node) const {
    try {
        // specific delegate first
        return m_delegates.at(node);
    } catch (std::out_of_range){
        // general delegate next
        return m_default_delegates.at(node->type);
    }
}

QWidget * NodeDelegateMapper::createEditor ( QWidget * parent, const QStyleOptionViewItem & option,
                                             const QModelIndex & index ) const {
    const boost::shared_ptr<Node> node = static_cast<Node*>(index.internalPointer())->shared_from_this();
    try {
        boost::shared_ptr<QAbstractItemDelegate> delegate = getDelegate(node);
        return delegate->createEditor(parent, option, index);
    } catch (std::out_of_range){
        return QStyledItemDelegate::createEditor(parent,option, index);
    }
}

void NodeDelegateMapper::setEditorData(QWidget *editor, const QModelIndex &index) const{
    const boost::shared_ptr<Node> node = static_cast<Node*>(index.internalPointer())->shared_from_this();
    try {
        boost::shared_ptr<QAbstractItemDelegate> delegate = getDelegate(node);
        return delegate->setEditorData(editor, index);
    } catch (std::out_of_range){
        return QStyledItemDelegate::setEditorData(editor, index);
    }
}



void NodeDelegateMapper::updateEditorGeometry(QWidget *editor,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const{
    const boost::shared_ptr<Node> node = static_cast<Node*>(index.internalPointer())->shared_from_this();
    try {
        boost::shared_ptr<QAbstractItemDelegate> delegate = getDelegate(node);
        return delegate->updateEditorGeometry(editor, option, index);
    } catch (std::out_of_range){
        return QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    }
}

void NodeDelegateMapper::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const{
    // display the node
    void* ptr = index.internalPointer();
    if(!ptr){
        error() << "NodeDelegateMapper:: nothing to paint!";
        return;
    }
    const boost::shared_ptr<Node> node = static_cast<Node*>(ptr)->shared_from_this();
    if (node) {
        try {
            boost::shared_ptr<QAbstractItemDelegate> delegate = getDelegate(node);
            delegate->paint(painter, option, index);
        } catch (std::out_of_range){
            //debug() << "NodeDelegateMapper::paint() - not column one"
            QStyledItemDelegate::paint(painter, option, index);
        }
    } else QStyledItemDelegate::paint(painter, option, index);
}

QSize NodeDelegateMapper::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const{
    if (m_hasSizeHint){
        return m_sizeHint;
    } else {
        const boost::shared_ptr<Node> node = static_cast<Node*>(index.internalPointer())->shared_from_this();
        try {
            boost::shared_ptr<QAbstractItemDelegate> delegate = getDelegate(node);
            return delegate->sizeHint(option, index);
        } catch (std::out_of_range){
            return QStyledItemDelegate::sizeHint(option, index);
        }
    }
}

void NodeDelegateMapper::setSizeHint(const QSize sizeHint) {
    m_sizeHint = sizeHint;
    m_hasSizeHint = true;
}


NumericDelegate::NumericDelegate(QObject * parent) : QStyledItemDelegate(parent) {
    QItemEditorFactory * factory = new QItemEditorFactory();
    setItemEditorFactory(factory);
    factory->registerEditor(QVariant::Bool, new QItemEditorCreator<OnOffSlider>("checked"));
    factory->registerEditor(QVariant::Int, new QItemEditorCreator<NeutralSpinBox>("value"));
    factory->registerEditor(QVariant::UInt, new QItemEditorCreator<NeutralSpinBox>("value"));
    factory->registerEditor(QVariant::Double, new QItemEditorCreator<NeutralDoubleSpinBox>("value"));
    factory->registerEditor((QVariant::Type)qMetaTypeId<float>(), new QItemEditorCreator<NeutralDoubleSpinBox>("value"));
    factory->registerEditor((QVariant::Type)qMetaTypeId<BoundedFloat>(), new QItemEditorCreator<NeutralDoubleSpinBox>("value"));

    debug() << "NumericDelegate()";
}


void NumericDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{

    NumericNodeBase * node = dynamic_cast<NumericNodeBase*>((Node*)index.internalPointer());

    if (node && (unsigned)node->get().type() == (unsigned)qMetaTypeId<bool>()){
        StyleOptionOnOff onOffOption;
        onOffOption.rect = option.rect;
        onOffOption.position = node->get().value<bool>() ? 1 : 0;
        onOffOption.marked = node->isMutable();
        QApplication::style()->drawControl(QStyle::CE_CheckBox,
                                           &onOffOption, painter);
    }
    else if (node) {
        double value = node->asNumber().toDouble();
        QString text = QString::number( value, 'g', node->getPrecision() );

        QStyleOptionProgressBarV2 progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.text = text.append(QString::fromStdString(node->getUnits()));
        progressBarOption.textVisible = true;
        progressBarOption.invertedAppearance = node->isInverted();
        progressBarOption.palette.setColor(QPalette::Text, Qt::black);
        if(!node->isMutable())
            progressBarOption.palette.setColor(QPalette::Text, QColor(100,100,100));

        progressBarOption.maximum = (node->isMaxSet() && node->isMinSet()) ? 1000 : 0;
        progressBarOption.minimum = 0;
        progressBarOption.progress = (int)(fabs(pivot(node->getMin().toDouble(),
                                                      node->getNeutral().toDouble(),
                                                      node->getMax().toDouble(),
                                                      node->asNumber().toDouble()))*progressBarOption.maximum);

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                           &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}


QWidget * NumericDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    QWidget *retval = QStyledItemDelegate::createEditor(parent, option, index);
    if (OnOffSlider * slider = dynamic_cast<OnOffSlider*>(retval)) {
        info() << "create editor";
        slider->setAnimation(true);
        connect(retval, SIGNAL(switched()), this, SLOT(commit()));
        NumericNode<bool> * node = dynamic_cast<NumericNode<bool>*>((Node*)index.internalPointer());
        node->set(!node->get());
    }
    return retval;

}

void NumericDelegate::commit() {
    info() << "commit()";
    QWidget *editor = static_cast<QWidget *>(sender());
    emit commitData(editor);
}

void NumericDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{

    QStyledItemDelegate::setEditorData(editor, index);

    NumericNodeBase * node = dynamic_cast<NumericNodeBase*>((Node*)index.internalPointer());

    if(node && node->isMaxSet() && node->isMinSet()) {
        if(NeutralSpinBox * neutral = qobject_cast<NeutralSpinBox*>(editor)){
            neutral->setMinimum(node->getMin().toInt());
            neutral->setMaximum(node->getMax().toInt());
            neutral->setWrapping(node->getWraps());
            neutral->setNeutral(node->getNeutral().toInt());
            neutral->setValue(node->asNumber().toInt());
            neutral->setInverted(node->isInverted());
        }

        if(NeutralDoubleSpinBox * neutral = qobject_cast<NeutralDoubleSpinBox*>(editor)){
            neutral->setMinimum(node->getMin().toDouble());
            neutral->setMaximum(node->getMax().toDouble());
            neutral->setWrapping(node->getWraps());
            neutral->setNeutral(node->getNeutral().toDouble());
            neutral->setDecimals(node->getPrecision());
            neutral->setValue(node->asNumber().toDouble());
            neutral->setInverted(node->isInverted());
        }
    }
}
