#ifndef HUBWIDGET_H
#define HUBWIDGET_H

#include "translatablewidgetinterface.h"
class MainObject;
class QStackedWidget;
class LogInPage;
class InitialisationPage;
class DownloadMTGATemplatePage;
class DownloadOptionsPage;
class QPushButton;
class HubWidget : public TranslatableWidgetInterface
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(HubWidget);
public:
    explicit HubWidget(QWidget *parent = nullptr);
    ~HubWidget();
protected:
    void retranslateUi() override;
private slots:
    void onSetsMTGAHDownloaded();
    void onLoggedIn();
    void onCustomRatingsTemplateDown();
private:
    enum StackPages{
        spInitPage
        ,spLogInPage
        ,spRtgTemplatePage
        , spDownloadOptPage

    };
    MainObject *m_object;
    QStackedWidget* m_stack;
    LogInPage* m_logInPage;
    InitialisationPage* m_initPage;
    DownloadMTGATemplatePage* m_downloadTemplatePage;
    DownloadOptionsPage* m_downloadOptionsPage;
};

#endif // HUBWIDGET_H
