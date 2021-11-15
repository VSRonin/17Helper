#include "textdatedelegate.h"
#include <QDateTime>
TextDateDelegate::TextDateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{ }

QString TextDateDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.isValid())
        return locale.toString(QDateTime::fromString(value.toString(), Qt::ISODate));
    return tr("Never", "Never updated before");
}
