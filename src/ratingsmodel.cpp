#include "ratingsmodel.h"
#include "mtgahcard.h"
#ifdef QT_DEBUG
#    include <QDebug>
#    include <QSqlError>
#endif
RatingsModel::RatingsModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{ }

void RatingsModel::setTable(const QString &)
{
    QSqlTableModel::setTable(QStringLiteral("Ratings"));
    select();
}

int RatingsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return QSqlTableModel::columnCount(parent);
    return qMax(int(rmcCount), QSqlTableModel::columnCount(parent));
}

QVariant RatingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Vertical)
        return section + 1;
    switch (section) {
    case rmcSet:
        return tr("Set");
    case rmcName:
        return tr("Name");
    case rmcArenaId:
        return tr("Id Arena");
    case rmcRating:
        return tr("Rating");
    case rmcNote:
        return tr("Notes");
    default:
        return QVariant();
    }
}

Qt::ItemFlags RatingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.parent().isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags result = QSqlTableModel::flags(index);
    switch (index.column()) {
    case rmcSet:
    case rmcName:
    case rmcArenaId:
        result &= ~Qt::ItemIsEditable;
        break;
    default:
        break;
    }
    return result;
}
