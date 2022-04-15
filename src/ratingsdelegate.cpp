/****************************************************************************\
   Copyright 2021 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/
#include "ratingsdelegate.h"
#include "ratingspinbox.h"

RatingsDelegate::RatingsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{ }

QWidget *RatingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    RatingSpinBox *result = new RatingSpinBox(parent);
    return result;
}

void RatingsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    RatingSpinBox *spin = qobject_cast<RatingSpinBox *>(editor);
    Q_ASSERT(spin);
    spin->setValue(index.data().toInt());
}

void RatingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    RatingSpinBox *spin = qobject_cast<RatingSpinBox *>(editor);
    Q_ASSERT(spin);
    model->setData(index, spin->value());
}

void RatingsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

QString RatingsDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.isNull() || value.toInt() == -1)
        return RatingSpinBox::nonRatedString();
    return QStyledItemDelegate::displayText(value, locale);
}
