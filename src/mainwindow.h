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
#include <QWidget>
class QLineEdit;
class QListView;
class QStandardItemModel;
class QLabel;
class QGroupBox;
class QComboBox;
class QPushButton;
class Worker;
class MainWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)
public:
    explicit MainWindow(QWidget* parent = nullptr);
protected:
    void changeEvent(QEvent *event) override;
    void retranslateUi();
private:
    QGroupBox *m_mtgahelperGroup;
    QLabel *m_usernameLabel;
    QLineEdit *m_usernameEdit;
    QLabel *m_pwdLabel;
    QLineEdit *m_pwdEdit;
    QPushButton *m_mtgahLoginButton;
    QLabel *m_setsLabel;
    QListView *m_setsView;
    QStandardItemModel *m_setsModel;
    QComboBox* m_formatCombo;
    QLabel *m_formatLabel;
    QGroupBox *m_downloadGroup;
    QPushButton *m_startButton;
    Worker *m_worker;
private slots:
    void doLogin();
    void fillSets(const QStringList& sets);
    void fillSetNames(const QHash<QString,QString>& setNames);
};

#endif
