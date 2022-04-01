#include "translatablewidgetinterface.h"
#include <QEvent>
TranslatableWidgetInterface::TranslatableWidgetInterface(QWidget *parent)
    : QWidget(parent)
{

}

void TranslatableWidgetInterface::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    else
        QWidget::changeEvent(event);
}
