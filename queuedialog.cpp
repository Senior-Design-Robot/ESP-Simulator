#include "queuedialog.h"
#include "ui_queuedialog.h"

QueueDialog::QueueDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueueDialog)
{
    ui->setupUi(this);
}

QueueDialog::~QueueDialog()
{
    delete ui;
}

void QueueDialog::on_addElemButton_clicked()
{
    int newVal = ui->elemValueSpin->value();
    queue.push_back(newVal);
    ui->countLabel->setText(QString::number(queue.size()));
}

void QueueDialog::on_popButton_clicked()
{
    int popVal;
    if( queue.pop(popVal) )
    {
        ui->popLabel->setText(QString::number(popVal));
    }
    else
    {
        ui->popLabel->setText("N/A (was empty)");
    }

    ui->countLabel->setText(QString::number(queue.size()));
}
