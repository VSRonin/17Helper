#include "ratingsdelegate.h"
#include <QSpinBox>
RatingsDelegate::RatingsDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

QWidget *RatingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QSpinBox *result = new QSpinBox(parent);
    result->setRange(0,10);
    return result;
}

void RatingsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
    Q_ASSERT(spin);
    spin->setValue(index.data().toInt());
}

void RatingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
    Q_ASSERT(spin);
    model->setData(index,spin->value());
}

void RatingsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}


