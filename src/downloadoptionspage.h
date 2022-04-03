#ifndef DOWNLOADOPTIONSPAGE_H
#define DOWNLOADOPTIONSPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class DownloadOptionsPage;
}
class MainObject;
class DownloadOptionsPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadOptionsPage)
public:
    explicit DownloadOptionsPage(QWidget *parent = nullptr);
    ~DownloadOptionsPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
signals:
    void logOut();
protected:
    void retranslateUi() override;
private slots:
    void onLoadDownloadFormat(const QString &format);
    void onShowOnlyDraftableSetsChanged(bool showOnly);
private:
    Ui::DownloadOptionsPage *ui;
    MainObject* m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADOPTIONSPAGE_H
