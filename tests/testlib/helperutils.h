#ifndef HELPER_H
#define HELPER_H

#include <QAbstractItemModel>
#include <KDebug>
#include <Akonadi/EntityTreeModel>

namespace Helper {
    void printModel(QAbstractItemModel *model, const QModelIndex &parent = QModelIndex(), int level = 0)
    {
         if (!level)
             qDebug() << "------------------start--------------------";
         QString prefix;
         for (int i = 0; i < level; i++) {
             prefix.append("-");
         }
         for(int q = 0; q < model->rowCount(parent); q++) {
             const QModelIndex &index = model->index(q, 0, parent);
             qDebug() << prefix << index << index.data(Qt::DisplayRole) << model->columnCount(index) << index.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>().url();
             if (model->hasChildren(index)) {
                 printModel(model, index, level+1);
             }
         }
         if (!level)
             qDebug() << "------------------end--------------------";
     };
 }
 
 #endif
