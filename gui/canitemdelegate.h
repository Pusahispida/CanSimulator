/*!
* \file
* \brief canitemdelegate.h foo
*/

#ifndef CANITEMDELEGATE_H
#define CANITEMDELEGATE_H

#include "mainwindow.h"
#include <QWidget>
#include <QStyledItemDelegate>

class CANItemDelegate : public QStyledItemDelegate
{
public:
    explicit CANItemDelegate(QObject *parent = NULL);
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif // CANITEMDELEGATE_H
