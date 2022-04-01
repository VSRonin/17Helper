#ifndef DOWNLOADMTGATEMPLATEPAGE_H
#define DOWNLOADMTGATEMPLATEPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class DownloadMTGATemplatePage;
}
class MainObject;
class DownloadMTGATemplatePage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DownloadMTGATemplatePage)
public:
    explicit DownloadMTGATemplatePage(QWidget *parent = nullptr);
    ~DownloadMTGATemplatePage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();
protected:
    void retranslateUi() override;
private slots:
    void onCustomRatingTemplateFailed();
    void retryTemplateDownload();
private:
    Ui::DownloadMTGATemplatePage *ui;
    MainObject* m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // DOWNLOADMTGATEMPLATEPAGE_H
