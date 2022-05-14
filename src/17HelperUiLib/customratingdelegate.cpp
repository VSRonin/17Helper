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
#include "customratingdelegate.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTableView>
#include <QLineEdit>
#include <QApplication>

CustomRatingsDelegate::CustomRatingsDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
    , currentEditor(nullptr)
{}

QString CustomRatingsDelegate::displayText(const QVariant &value, const QLocale &locale) const{
    if(!value.isValid() || value.toInt()==-1)
        return RatingSpinBox::nonRatedString();
    return QStyledItemDelegate::displayText(value,locale);
}

void CustomRatingsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    QStyleOptionButton buttonOption = buttonOptions(opt);
    style->drawControl(QStyle::CE_PushButton, &buttonOption, painter, widget);
}

QSize CustomRatingsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QSize baseSize = spinSizeHint(opt,index);
    const QRect butRect = buttonRect(opt);
    return QSize(baseSize.width()+butRect.width(),qMax(butRect.height(),baseSize.height()));
}

QWidget *CustomRatingsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QWidget* result = new QWidget(parent);
    result->setGeometry(option.rect);
    RatingSpinBox* baseEditor = new RatingSpinBox(result);
    result->setFocusProxy(baseEditor);
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    const QRect butRect = buttonRect(opt);
    baseEditor->setObjectName(QStringLiteral("baseEditor"));
    baseEditor->setGeometry(0,0,opt.rect.width()-butRect.width(),opt.rect.height());
    QPushButton* myButton = new QPushButton(result);
    myButton->setObjectName(QStringLiteral("myButton"));
    myButton->setText(m_buttonText);
    myButton->setIcon(m_buttonIcon);
    myButton->setGeometry(opt.rect.width()-butRect.width(), 0, butRect.width(),butRect.height());
    currentEditor = result;
    connect(myButton, &QPushButton::clicked, this, &CustomRatingsDelegate::clickedHelper);
    return result;
}

void CustomRatingsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    currentIndex = index;
    RatingSpinBox* baseEditor = editor->findChild<RatingSpinBox*>(QStringLiteral("baseEditor"));
    Q_ASSERT(baseEditor);
    const QVariant editdata = index.data();
    if(editdata.isValid())
        baseEditor->setValue(editdata.toInt());
    else
        baseEditor->setValue(-1);
}

void CustomRatingsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    RatingSpinBox* baseEditor = editor->findChild<RatingSpinBox*>(QStringLiteral("baseEditor"));
    Q_ASSERT(baseEditor);
    model->setData(index,baseEditor->value());
}

void CustomRatingsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.showDecorationSelected = true;
    const QRect butRect = buttonRect(opt);
    editor->setGeometry(opt.rect);
    QWidget* baseEditor = editor->findChild<QWidget*>(QStringLiteral("baseEditor"));
    Q_ASSERT(baseEditor);
    baseEditor->setGeometry(0,0,opt.rect.width()-butRect.width(),opt.rect.height());
    QWidget* myButton = editor->findChild<QWidget*>(QStringLiteral("myButton"));
    Q_ASSERT(myButton);
    myButton->setGeometry(opt.rect.width()-butRect.width(), 0, butRect.width(),butRect.height());
}

const QString CustomRatingsDelegate::text() const
{
    return m_buttonText;
}

void CustomRatingsDelegate::setText(const QString &newButtonText)
{
    m_buttonText = newButtonText;
}

const QIcon &CustomRatingsDelegate::icon() const
{
    return m_buttonIcon;
}

void CustomRatingsDelegate::setIcon(const QIcon &newButtonIcon)
{
    m_buttonIcon = newButtonIcon;
}

bool CustomRatingsDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_ASSERT(event);
    Q_ASSERT(model);
    Qt::ItemFlags flags = model->flags(index);
    if ((option.state & QStyle::State_Enabled) && (flags & Qt::ItemIsEnabled))
    {
        switch (event->type()){
        case QEvent::MouseButtonRelease:{
            QStyleOptionViewItem viewOpt(option);
            initStyleOption(&viewOpt, index);
            const QRect butRect = buttonRect(viewOpt);
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton && butRect.contains(me->pos())){
                currentIndex=index;
                clickedHelper();
            }
        }
            break;
        default:
            break;
        }
    }
    return QStyledItemDelegate::editorEvent(event,model,option,index);
}

QStyleOptionButton CustomRatingsDelegate::buttonOptions(const QStyleOptionViewItem &option, bool skipRct) const
{
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    int buttonIconSize = style->pixelMetric(QStyle::PM_ButtonIconSize, 0, widget);
    QStyleOptionButton buttonOption;
    buttonOption.text = m_buttonText;
    buttonOption.icon = m_buttonIcon;
    buttonOption.iconSize = (QSize(buttonIconSize,buttonIconSize));
    buttonOption.rect = skipRct ? QRect() : buttonRect(option);
    buttonOption.features = QStyleOptionButton::None;
    buttonOption.direction = option.direction;
    buttonOption.fontMetrics = option.fontMetrics;
    buttonOption.palette = option.palette;
    buttonOption.styleObject = option.styleObject;
    buttonOption.state=option.state;
    return buttonOption;
}

QRect CustomRatingsDelegate::buttonRect(const QStyleOptionViewItem &option) const
{
    int w = 0, h = 0;
    QStyleOptionButton buttonOption = buttonOptions(option, true);
    if(!buttonOption.icon.isNull()){
        w += buttonOption.iconSize.width()+4;
        h= qMax(h,buttonOption.iconSize.height());
    }
    const QString buttonText = buttonOption.text;
    if(!buttonText.isEmpty()){
        const QSize sz = buttonOption.fontMetrics.size(Qt::TextShowMnemonic, buttonText);
        w += sz.width();
        h = qMax(h, sz.height());
    }
    buttonOption.rect.setSize(QSize(w, h));
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    QSize buttonSize = style->sizeFromContents(QStyle::CT_PushButton, &buttonOption, QSize(w,h), widget);
    //buttonSize.setWidth(qMin(buttonSize.width(),option.rect.width()/2));
    return QRect(option.rect.left()+option.rect.width()-buttonSize.width(),option.rect.top(),buttonSize.width(),qMax(buttonSize.height(),option.rect.height()));
}

QSize CustomRatingsDelegate::spinSizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const{
    const QSize itemSize = QStyledItemDelegate::sizeHint(option,index);
    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    QStyleOptionSpinBox opt;
    opt.direction =option.direction;
    opt.fontMetrics =option.fontMetrics;
    opt.palette =option.palette;
    opt.state =option.state;
    opt.buttonSymbols= QAbstractSpinBox::PlusMinus;
    opt.activeSubControls = QStyle::SC_None;
    opt.subControls = QStyle::SC_SpinBoxEditField | QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;
    if (style->styleHint(QStyle::SH_SpinBox_ButtonsInsideFrame, nullptr, widget))
        opt.subControls |= QStyle::SC_SpinBoxFrame;
    opt.stepEnabled =QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;
    opt.frame=true;
    int w = qMax(0, opt.fontMetrics.horizontalAdvance(QStringLiteral("-1 ")));
    w = qMax(w, opt.fontMetrics.horizontalAdvance(QStringLiteral("10 ")));
    w = qMax(w, opt.fontMetrics.horizontalAdvance(RatingSpinBox::nonRatedString()));
    w += 2;
    return style->sizeFromContents(QStyle::CT_SpinBox, &opt, QSize(w,itemSize.height()), widget);
}

void CustomRatingsDelegate::clickedHelper()
{
    if(currentEditor)
        emit closeEditor(currentEditor, QAbstractItemDelegate::SubmitModelCache);
    clicked(currentIndex);
}
