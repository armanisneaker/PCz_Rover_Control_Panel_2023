#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSinglePointEvent>
#include <QDebug>
#include "connection.h"
#include <cmath>
#include <QDebug>

void MainWindow::on_pushButton_add_pos_clicked()
{

    navigation.pos_X.push_back(ui->lineEditPosX->text().toDouble());
    navigation.pos_Y.push_back(ui->lineEditPosY->text().toDouble());
    ui->tableWidget_nav->insertRow(ui->tableWidget_nav->rowCount());

    navigation.itemID.push_back(new QTableWidgetItem);
    navigation.itemRow.push_back(new QTableWidgetItem);
    navigation.itemColumn.push_back(new QTableWidgetItem);


    navigation.itemID[navigation.index_number]->setText(QString::number(navigation.index_number+1));
    navigation.itemRow[navigation.index_number]->setText(QString::number(navigation.pos_X[navigation.index_number],'g',12));
    navigation.itemColumn[navigation.index_number]->setText(QString::number(navigation.pos_Y[navigation.index_number],'g',12));

    //qDebug() << "Precision: " << navigation.pos_X[navigation.index_number];


    ui->tableWidget_nav->setItem( navigation.index_number, 0, navigation.itemID[navigation.index_number]);
    ui->tableWidget_nav->setItem( navigation.index_number, 1, navigation.itemRow[navigation.index_number]);
    ui->tableWidget_nav->setItem( navigation.index_number, 2, navigation.itemColumn[navigation.index_number]);

    ui->graphicsView->centerOn(500000,500000);
    mapmodule->addPoint(navigation.pos_X[navigation.index_number],navigation.pos_Y[navigation.index_number]);
    navigation.index_number++;
}

void MainWindow::on_tableWidget_nav_cellClicked(int row, int column)
{
    navigation.choosen_index = row;
    ui->labelChoosenIndex->setText(QString::number(navigation.choosen_index));
    mapmodule->getChoosenPoint(navigation.choosen_index);
}

void MainWindow::on_pushButtonDeletePoint_clicked()
{
    navigation.pos_X.erase(navigation.pos_X.begin() + navigation.choosen_index);
    navigation.pos_Y.erase(navigation.pos_Y.begin() + navigation.choosen_index);

    navigation.itemID.erase(navigation.itemID.begin() + navigation.choosen_index);
    navigation.itemRow.erase(navigation.itemRow.begin() + navigation.choosen_index);
    navigation.itemColumn.erase(navigation.itemColumn.begin() + navigation.choosen_index);

    ui->tableWidget_nav->removeRow(navigation.choosen_index);

    navigation.index_number--;

    //zmienia wartosci w kolumnie ID żeby wszytsko się zgadzało
    for(int i = navigation.choosen_index; i < navigation.index_number; i++)
    {
        navigation.itemID[i]->setText(QString::number(i+1));
        //ui->tableWidget_nav->setItem( navigation.index_number, 0, navigation.itemID[i]);
    }

    //ustawia na index poprzedni i zmienia przy okazji item na scenie
    if(navigation.choosen_index>0)
    {
        navigation.choosen_index--;
        ui->labelChoosenIndex->setText(QString::number(navigation.choosen_index));
        mapmodule->getChoosenPoint(navigation.choosen_index);
    }

    mapmodule->deleteCurrentPoint();
}
