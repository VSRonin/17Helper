#ifndef NOCHECKPROXY_H
#define NOCHECKPROXY_H
#include "17helperlib_global.h"
#include <QIdentityProxyModel>
class SHLIB_EXPORT NoCheckProxy : public QIdentityProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NoCheckProxy)
public:
    using QIdentityProxyModel::QIdentityProxyModel;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (role == Qt::CheckStateRole)
            return QVariant();
        return QIdentityProxyModel::data(index, role);
    }
};
#endif // NOCHECKPROXY_H
