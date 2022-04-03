#ifndef LOGOUTPAGE_H
#define LOGOUTPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class LogOutPage;
}
class MainObject;
class LogOutPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LogOutPage)
public:
    explicit LogOutPage(QWidget *parent = nullptr);
    ~LogOutPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();
protected:
    void retranslateUi() override;
private slots:
    void retryLogout();
    void onLogOutFailed(const QString& err);
private:
    Ui::LogOutPage *ui;
    MainObject* m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // LOGOUTPAGE_H
