/****************************************************************************\
   Copyright 2021 Luca Beldi
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
\****************************************************************************/
#include "forceerrorwidget.h"
#ifdef QT_DEBUG
#    include <QCheckBox>
#    include <QPushButton>
#    include <QVBoxLayout>
#    include "globals.h"
ForceErrorWidget::ForceErrorWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("Force failure"));
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    QCheckBox *failInitCheckBox = new QCheckBox(QStringLiteral("Initialisation"), this);
    connect(failInitCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailInit = state == Qt::Checked; });
    mainLay->addWidget(failInitCheckBox);
    QCheckBox *failLoginCheckBox = new QCheckBox(QStringLiteral("Log In"), this);
    connect(failLoginCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailLogin = state == Qt::Checked; });
    mainLay->addWidget(failLoginCheckBox);
    QCheckBox *failLogoutCheckBox = new QCheckBox(QStringLiteral("Log Out"), this);
    connect(failLogoutCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailLogout = state == Qt::Checked; });
    mainLay->addWidget(failLogoutCheckBox);
    QCheckBox *failSetsMTGAHCheckBox = new QCheckBox(QStringLiteral("Sets MTGAH"), this);
    connect(failSetsMTGAHCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailSetsMTGAH = state == Qt::Checked; });
    mainLay->addWidget(failSetsMTGAHCheckBox);
    QCheckBox *failSetsScryfallCheckBox = new QCheckBox(QStringLiteral("Sets Scryfall"), this);
    connect(failSetsScryfallCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailSetsScryfall = state == Qt::Checked; });
    mainLay->addWidget(failSetsScryfallCheckBox);
    QCheckBox *failCustomRatingTemplateCheckBox = new QCheckBox(QStringLiteral("Custom Rating Template"), this);
    connect(failCustomRatingTemplateCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailCustomRatingTemplate = state == Qt::Checked; });
    mainLay->addWidget(failCustomRatingTemplateCheckBox);
    QCheckBox *fail17LRatingsCheckBox = new QCheckBox(QStringLiteral("17L Ratings"), this);
    connect(fail17LRatingsCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFail17LRatings = state == Qt::Checked; });
    mainLay->addWidget(fail17LRatingsCheckBox);
    QCheckBox *failUploadRatingCheckBox = new QCheckBox(QStringLiteral("Upload Rating"), this);
    connect(failUploadRatingCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailUploadRating = state == Qt::Checked; });
    mainLay->addWidget(failUploadRatingCheckBox);
    QCheckBox *failRatingCalculationCheckBox = new QCheckBox(QStringLiteral("Rating Calculation"), this);
    connect(failRatingCalculationCheckBox, &QCheckBox::stateChanged, this, [](int state) { dtFailRatingCalculation = state == Qt::Checked; });
    mainLay->addWidget(failRatingCalculationCheckBox);
    mainLay->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));
    QPushButton *startButton = new QPushButton(QStringLiteral("Start"), this);
    connect(startButton, &QPushButton::clicked, this, [startButton]() { startButton->setEnabled(false); });
    connect(startButton, &QPushButton::clicked, this, &ForceErrorWidget::start);
    mainLay->addWidget(startButton);
}
#endif
