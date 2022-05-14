/****************************************************************************\
   Copyright 2022 Luca Beldi
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
#ifndef CUSTOMRATINGDELEGATE_H
#define CUSTOMRATINGDELEGATE_H

#include <QStyledItemDelegate>
#include <QPointer>
#include "ratingspinbox.h"
 
class CustomRatingsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CustomRatingsDelegate)
public:
    explicit CustomRatingsDelegate(QObject* parent);
    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    const QString text() const;
    void setText(const QString &newButtonText);
    const QIcon &icon() const;
    void setIcon(const QIcon &newButtonIcon);
signals:
    void clicked(const QModelIndex &index);
protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    virtual QStyleOptionButton buttonOptions(const QStyleOptionViewItem &option, bool skipRct=false) const;
    virtual QRect buttonRect(const QStyleOptionViewItem &option) const;
    virtual QSize spinSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    mutable QModelIndex currentIndex;
    mutable QPointer<QWidget> currentEditor;
    QString m_buttonText;
    QIcon m_buttonIcon;
private slots:
    void clickedHelper();
};

#endif // CUSTOMRATINGDELEGATE_H
