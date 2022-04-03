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
    connect(ui->logOutButton,&QPushButton::clicked,this,&DownloadOptionsPage::logOut);
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
        connect(m_object,&MainObject::loadDownloadFormat,this,&DownloadOptionsPage::onLoadDownloadFormat)
        ,connect(m_object,&MainObject::showOnlyDraftableSetsChanged,this,&DownloadOptionsPage::onShowOnlyDraftableSetsChanged)
    };
}

void DownloadOptionsPage::retranslateUi()
{
    ui->retranslateUi(this);
}

void DownloadOptionsPage::onLoadDownloadFormat(const QString &format)
{
    ui->formatCombo->setCurrentIndex(ui->formatCombo->findData(format, Qt::UserRole));
}

void DownloadOptionsPage::onShowOnlyDraftableSetsChanged(bool showOnly)
{
    ui->draftableSetsCheck->setChecked(showOnly);
}
