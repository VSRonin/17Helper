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
#include <QApplication>
#include <mainwindow.h>
#include <QVector>
#include <memory>
#ifdef QT_DEBUG
#    include <QLocale>
#    include <loggingtools/forceerrorwidget.h>
#endif
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#ifdef QT_DEBUG
    std::unique_ptr<MainWindow> w(nullptr);
    ForceErrorWidget feW;
    feW.show();
    feW.setGeometry(0, 20, feW.width(), feW.height());
    QObject::connect(&feW, &ForceErrorWidget::start, [&w] {
        w = std::make_unique<MainWindow>();
        w->show();
    });
#else
    MainWindow w;
    w.show();
#endif
    return app.exec();
}
