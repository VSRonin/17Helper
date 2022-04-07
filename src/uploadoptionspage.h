#ifndef UPLOADOPTIONSPAGE_H
#define UPLOADOPTIONSPAGE_H
#include <globals.h>
#include "translatablewidgetinterface.h"

namespace Ui {
class UploadOptionsPage;
}
class MainObject;
class NoCheckProxy;
class UploadOptionsPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UploadOptionsPage)
public:
    explicit UploadOptionsPage(QWidget *parent = nullptr);
    ~UploadOptionsPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);

protected:
    void retranslateUi() override;
private slots:
    void onShowOnlyRatiosChanged(bool showOnly);
    void selectAllMetrics();
    void selectNoMetrics();
    void onLoadUploadRating(GEnums::SLMetrics ratingBase);
    void doMtgahUpload(bool clear);

private:
    Ui::UploadOptionsPage *ui;
    MainObject *m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
    NoCheckProxy *m_SLMetricsProxy;
};

#endif // UPLOADOPTIONSPAGE_H
