#ifndef INITIALISATIONPAGE_H
#define INITIALISATIONPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class InitialisationPage;
}
class MainObject;
class InitialisationPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(InitialisationPage)
public:
    explicit InitialisationPage(QWidget *parent = nullptr);
    ~InitialisationPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();
protected:
    void retranslateUi() override;
private slots:
    void onInitialisationFailed();
    void onDownloadSetsMTGAHFailed();
    void retrySetsDownload();
private:
    Ui::InitialisationPage *ui;
    MainObject* m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // INITIALISATIONPAGE_H
