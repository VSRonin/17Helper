#ifndef CUSTOMRATINGSPAGE_H
#define CUSTOMRATINGSPAGE_H

#include "translatablewidgetinterface.h"
#include <QVector>
namespace Ui {
class CustomRatingsPage;
}
class MainObject;
class QModelIndex;
class CustomRatingsDelegate;
class CustomRatingsPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(CustomRatingsPage)
public:
    explicit CustomRatingsPage(QWidget *parent = nullptr);
    ~CustomRatingsPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
signals:
    void applyCustomRatings();

protected:
    void retranslateUi() override;
private slots:
    void updateRatingsFiler();
    void onSetsFilter(const QModelIndex &, const QModelIndex &, const QVector<int> &roles);
    void onCustomRatingCleared(const QModelIndex &idx);

private:
    Ui::CustomRatingsPage *ui;
    MainObject *m_object = nullptr;
    QVector<QMetaObject::Connection> m_objectConnections;
    CustomRatingsDelegate *customRatingsDelegate;
};

#endif // CUSTOMRATINGSPAGE_H
