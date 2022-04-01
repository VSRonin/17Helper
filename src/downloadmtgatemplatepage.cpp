#include "downloadmtgatemplatepage.h"
#include "ui_downloadmtgatemplatepage.h"
#include <mainobject.h>
DownloadMTGATemplatePage::DownloadMTGATemplatePage(QWidget *parent) :
    TranslatableWidgetInterface(parent),
    ui(new Ui::DownloadMTGATemplatePage)
{
    ui->setupUi(this);
    reset();
    connect(ui->retryButton, &QPushButton::clicked, this, &DownloadMTGATemplatePage::retryTemplateDownload);
}

void DownloadMTGATemplatePage::reset()
{
    ui->retryButton->hide();
    ui->dowloadFailedLabel->hide();
    ui->downloadingLabel->show();
}

DownloadMTGATemplatePage::~DownloadMTGATemplatePage()
{
    delete ui;
}
MainObject *DownloadMTGATemplatePage::mainObject() const
{
    return m_object;
}

void DownloadMTGATemplatePage::setMainObject(MainObject *newObject)
{
    if(m_object == newObject)
        return;
    for(auto&& i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if(!m_object)
        return;
    m_objectConnections = QVector<QMetaObject::Connection>{
            connect(m_object, &MainObject::customRatingTemplateFailed, this, &DownloadMTGATemplatePage::onCustomRatingTemplateFailed)
};
}


void DownloadMTGATemplatePage::retranslateUi()
{
    ui->retranslateUi(this);
}
void DownloadMTGATemplatePage::onCustomRatingTemplateFailed(){
    ui->retryButton->show();
    ui->dowloadFailedLabel->show();
    ui->downloadingLabel->hide();
}
void DownloadMTGATemplatePage::retryTemplateDownload(){
    ui->retryButton->hide();
    ui->dowloadFailedLabel->hide();
    ui->downloadingLabel->show();
    if(m_object)
        m_object->getCustomRatingTemplate();
}
