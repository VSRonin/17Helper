#ifndef TRANSLATABLEWIDGETINTERFACE_H
#define TRANSLATABLEWIDGETINTERFACE_H

#include <QWidget>

class TranslatableWidgetInterface : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(TranslatableWidgetInterface)
public:
    explicit TranslatableWidgetInterface(QWidget *parent = nullptr);
protected:
    void changeEvent(QEvent *event) override;
    virtual void retranslateUi()=0;

};

#endif // TRANSLATABLEWIDGETINTERFACE_H
