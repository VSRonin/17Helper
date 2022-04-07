#ifndef DOWNLOADPROGRESSPAGE_H
#define DOWNLOADPROGRESSPAGE_H

#include "translatablewidgetinterface.h"
#include <mainobject.h>
namespace Ui {
class DownloadProgressPage;
}
class QStringListModel;
class DownloadProgressPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadProgressPage)
public:
    explicit DownloadProgressPage(QWidget *parent = nullptr);
    ~DownloadProgressPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();

protected:
    void retranslateUi() override;
signals:
    void goBack();
    void goNext();
private slots:
    void onStartProgress(MainObject::Operations op, const QString &description, int max, int min);
    void onUpdateProgress(MainObject::Operations op, int val);
    void onIncreaseProgress(MainObject::Operations op, int val);
    void onEndProgress(MainObject::Operations op);
    void onSLDownloadFinished();
    void onSLDownloadFailed();
    void onNo17LRating(const QStringList &sets);

private:
    QStringListModel *m_noRatingsModel;
    Ui::DownloadProgressPage *ui;
    MainObject *m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADPROGRESSPAGE_H
