/*
  This file is part of pdfquirk.
  Copyright 2020, Klaas Freitag <kraft@freisturz.de>

  pdfquirk is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  pdfquirk is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with pdfquirk.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMAGELISTDELEGATE_H
#define IMAGELISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QMenu>

class ImageListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    ImageListDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &  option ,
                   const QModelIndex &  index ) const override;

    void setDeskewEnabled(bool enabled);
signals:
    void flipImage();
    void deleteImage();
    void rotateImageLeft();
    void rotateImageRight();
    void DeskewImage();

public slots:
    void slotThumbSize(const QSize& size);

protected:
    void initStyleOption(QStyleOptionViewItem *o, const QModelIndex &idx) const override;

    virtual bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                const QModelIndex& index ) override;
private:
    void showContextMenu(const QPoint& globalPos);
    QSize _size;
    
    QScopedPointer<QMenu> _menu;
    QAction *_flipAct;
    QAction *_deleteAct;
    QAction *_rotateLeftAct;
    QAction *_rotateRightAct;
    QAction *_deskewAct;
};

#endif // IMAGELISTDELEGATE_H
