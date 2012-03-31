#ifndef HELPER_H
#define HELPER_H

#include <QAbstractItemModel>
#include <KDebug>

namespace Helper {
    void printModel(QAbstractItemModel *model, const QModelIndex &parent = QModelIndex(), int level = 0)
    {
         if (!level)
             kDebug() << "------------------start--------------------";
         QString prefix;
         for (int i = 0; i < level; i++) {
             prefix.append("-");
         }
         for(int q = 0; q < model->rowCount(parent); q++) {
             const QModelIndex &index = model->index(q, 0, parent);
             kDebug() << prefix << index << index.data(Qt::DisplayRole) << model->columnCount(index);
             if (model->hasChildren(index)) {
                 printModel(model, index, level+1);
             }
         }
         if (!level)
             kDebug() << "------------------end--------------------";
     };
 }
 
 #endif
