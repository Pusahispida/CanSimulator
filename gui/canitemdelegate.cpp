/*!
* \file
* \brief canitemdelegate.cpp foo
*/

#include "canitemdelegate.h"
#include "datatablemodel.h"
#include "stdstring_to_qstring.h"
#include "value.h"
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>

CANItemDelegate::CANItemDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
}

/*!
 * \brief CANItemDelegate::createEditor
 * Create a combobox to edit enums in table and line edit for the rest.
 * Overloaded QStyledItemDelegate function
 * \param parent: Parent object
 * \param: Unused parameter (editor appearance)
 * \param index: Which table item we are editing
 * \return Pointer to created editor widget
 */
QWidget * CANItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    // Check if it's an enum that we're trying to edit. If yes, then show a
    // combo box, otherwise a text edit
    const DataTableModel *model = static_cast<const DataTableModel *>(index.model());
    const TableItem &item = model->getData(index);

    // Do not provide editor for filter column
    if (index.column() == FILTER_COLUMN) {
        return Q_NULLPTR;
    } else if (item.isEnum()) {
        QComboBox *editor = new QComboBox(parent);
        const TableItem::AliasMap &enumItems = item.getValueDescriptions();

        int current = 0;
        for(const auto &i : enumItems) {
            QString item(qsFromSs(i.second));
            editor->addItem(item);

            // Focus on the current value. It's more sensible to do it here
            // instead of setModelData, because combobox indexes are not
            // directly coupled to actual CAN values. For the most part they
            // should be (that's what should happen when enum values are neatly
            // ordered with no gaps), but you can definitely not rely on it!
            const int canValue = i.first;
            if (canValue == model->getData(index).getValue().toInt()) {
                editor->setCurrentIndex(current);
            }

            current++;
        }
        return editor;

    } else {
        return new QLineEdit(parent);
    }
}

/*!
 * \brief CANItemDelegate::paint
 * Paint the item
 * \param painter: Painter
 * \param option: Options used for the item
 * \param index: Which table item triggered the event
 */
void CANItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == FILTER_COLUMN) {
        Qt::CheckState state = (Qt::CheckState)index.data(Qt::CheckStateRole).toInt();
        QRect r = option.rect;
        int x,y,w,h;

        painter->save();
        y = r.top();
        w = r.width() / 2;
        h = r.height();
        switch (state) {
            case Qt::Unchecked:
                x = r.left();
                painter->setBrush(QColor(0x42, 0xA1, 0x3F, 0xFF));
                painter->setPen(QColor(Qt::darkGreen));
                break;
            case Qt::PartiallyChecked:
                break;
            case Qt::Checked:
                x = r.left() + r.width() / 2;
                painter->setBrush(QColor(Qt::red));
                painter->setPen(QColor(Qt::darkRed));
                break;
        }
        r = QRect(x, y, w, h);
        painter->drawRect(r);
        painter->restore();
    } else {
        return QStyledItemDelegate::paint(painter, option, index);
    }
}

/*!
 * \brief CANItemDelegate::setModelData
 * Forward new data from editor to table, this works differently between enums
 * and the rest
 * \param editor: The editor widget in use
 * \param model: The model being edited
 * \param index: Which table item is edited
 */
void CANItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    // Check if it's an enum that we're trying to edit. If yes, then we cannot
    // use the default setModelData behavior. We'll instead forward the string
    // representation of the value, and let the table model work it out.
    const DataTableModel *castModel = static_cast<const DataTableModel *>(model);
    if (castModel->isEnum(index)) {

        QVariant data;
        QComboBox *comboBox = static_cast<QComboBox *>(editor);

        data.setValue(comboBox->currentText());
        model->setData(index, data, Qt::EditRole);

    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

/*!
 * \brief CANItemDelegate::setEditorData
 * Used to setup editor with the previous value
 * \param editor: The editor widget in use
 * \param index: Which table item is edited
 */
void CANItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    const DataTableModel *model = static_cast<const DataTableModel *>(index.model());
    const Value &oldData = model->getData(index).getValue();

    // Old value will be set to comboboxes representing enums already in
    // createEditor, because that's where it's easiest to do
    if (model->isEnum(index)) {
        QStyledItemDelegate::setEditorData(editor, index);
    } else {
        QLineEdit *ed = static_cast<QLineEdit *>(editor);
        ed->setText(qsFromSs(oldData.toString()));
    }
}

/*!
 * \brief CANItemDelegate::editorEvent
 * Event from editor
 * \param event: Editor event
 * \param model: The model of the event
 * \param option: Options used for the item
 * \param index: Which table item triggered the event
 */
bool CANItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonRelease && index.column() == FILTER_COLUMN) {
        Qt::CheckState state = (Qt::CheckState)index.data(Qt::CheckStateRole).toInt();
        model->setData(index, state, Qt::CheckStateRole);
        return true;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}
