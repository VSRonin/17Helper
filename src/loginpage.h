#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include "translatablewidgetinterface.h"

namespace Ui {
class LogInPage;
}
class MainObject;
class LogInPage : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(LogInPage)
public:
    explicit LogInPage(QWidget *parent = nullptr);
    ~LogInPage();
    MainObject *mainObject() const;
    void setMainObject(MainObject *newObject);
    void reset();
private slots:
    void onRememberPass(int state);
    void onLogin();
    void doLogin();
    void onLoginError(const QString &error);
    void onLoadUserPass(const QString &userName, const QString &password);
    void checkLoginEnabled();
protected:
    void retranslateUi() override;
private:
    void enableAll(bool enable);
    Ui::LogInPage *ui;
    MainObject* m_object;
    QVector<QMetaObject::Connection> m_objectConnections;
};

#endif // LOGINPAGE_H
