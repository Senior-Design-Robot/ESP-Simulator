#pragma once

#include "kqueue.h"
#include <QDialog>

namespace Ui {
class QueueDialog;
}

class QueueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QueueDialog(QWidget *parent = nullptr);
    ~QueueDialog();

private slots:
    void on_addElemButton_clicked();

    void on_popButton_clicked();

private:
    Ui::QueueDialog *ui;
    KQueue<int> queue;
};
