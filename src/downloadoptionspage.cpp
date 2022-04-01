#include "downloadoptionspage.h"
#include "ui_downloadoptionspage.h"
#include <mainobject.h>
#include <QButtonGroup>
DownloadOptionsPage::DownloadOptionsPage(QWidget *parent) :
    TranslatableWidgetInterface(parent),
    ui(new Ui::DownloadOptionsPage)
{
    ui->setupUi(this);
    QButtonGroup *timeGroup=new QButtonGroup(this);
    timeGroup->addButton(ui->noLimitRadio);
    timeGroup->addButton(ui->timeSpanRadio);
    timeGroup->addButton(ui->lastPeriodRadio);
}

DownloadOptionsPage::~DownloadOptionsPage()
{
    delete ui;
}
MainObject *DownloadOptionsPage::mainObject() const
{
    return m_object;
}

void DownloadOptionsPage::setMainObject(MainObject *newObject)
{
    if(m_object == newObject)
        return;
    for(auto&& i : qAsConst(m_objectConnections))
        disconnect(i);
    m_objectConnections.clear();
    m_object = newObject;
    if(!m_object)
        return;
    ui->setsView->setModel(m_object->setsModel());
    ui->formatCombo->setModel(m_object->formatsModel());
    m_objectConnections = QVector<QMetaObject::Connection>{

    };
}

void DownloadOptionsPage::retranslateUi()
{
    ui->retranslateUi(this);
}
