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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "17helperuilib_global.h"
#include <QMainWindow>
#include <QVector>
namespace Ui {
class MainWindow;
}
class QTranslator;
class SHUILIB_EXPORT MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onUpdatedUploadedStatus(const QString &card);
    void changeLanguage(const QLocale &loc);
    void onAboutQt();
    void onChangeLanguageAction(const QLocale &loc);

private:
    Ui::MainWindow *ui;
    QVector<std::shared_ptr<QTranslator>> translators;
};

#endif // MAINWINDOW_H