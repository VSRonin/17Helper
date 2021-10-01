#include "ratingsmodel.h"
#include "mtgahcard.h"

RatingsModel::RatingsModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_ratingsTemplate(nullptr)
{ }

int RatingsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_ratingsTemplate)
        return 0;
    return m_ratingsTemplate->size();
}

int RatingsModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return rmcCount;
}

QVariant RatingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.parent().isValid() || role != Qt::DisplayRole || index.row() >= rowCount())
        return QVariant();
    auto i = m_ratingsTemplate->begin();
    for (int j = index.row(); j > 0; --j)
        ++i;
    switch (index.column()) {
    case rmcSet:
        return i->set;
    case rmcName:
        return i->name;
    case rmcArenaId:
        return i->id_arena;
    case rmcRating:
        return static_cast<int>(i->rating);
    case rmcNote:
        return i->note;
    default:
        return QVariant();
    }
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

void RatingsModel::setRatingsTemplate(QMultiHash<QString, MtgahCard> *tmplt)
{
    beginResetModel();
    m_ratingsTemplate = tmplt;
    endResetModel();
}

bool RatingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole)
        role = Qt::DisplayRole;
    if (!index.isValid() || index.parent().isValid() || role != Qt::DisplayRole || index.row() >= rowCount())
        return false;
    auto i = m_ratingsTemplate->begin();
    for (int j = index.row(); j > 0; --j)
        ++i;
    switch (index.column()) {
    case rmcRating:
        i->rating = static_cast<decltype(i->rating)>(value.toInt());
        break;
    case rmcNote:
        i->note = value.toString();
        break;
    default:
        return false;
    }
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

Qt::ItemFlags RatingsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.parent().isValid())
        return Qt::NoItemFlags;
    Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
    switch (index.column()) {
    case rmcRating:
    case rmcNote:
        result |= Qt::ItemIsEditable;
        break;
    default:
        break;
    }
    return result;
}
