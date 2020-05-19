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

#include "imagelistdelegate.h"

#include <QPainter>
#include <QModelIndex>
#include <QRect>
#include <QDebug>

ImageListDelegate::ImageListDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{

}

void ImageListDelegate::initStyleOption(QStyleOptionViewItem *o, const QModelIndex &idx) const
{
  QStyledItemDelegate::initStyleOption(o, idx);
  // to hide the display role all we need to do is remove the HasDisplay feature
  // just hides the display role
  if (!_displayRoleEnabled)
    o->features &= ~QStyleOptionViewItem::HasDisplay;
}

void ImageListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (option.state & QStyle::State_Selected)
        painter->setBrush(option.palette.highlightedText());
    else
        painter->setBrush(QBrush(Qt::black));

    int boxH = 28;

    const QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>().scaled(_size.width()-4, _size.height() -4 -boxH);
    painter->drawPixmap( option.rect.topLeft().x()+2, 2, pix);

    // a little background behinde the "Page n" text
    int h = _size.height()-boxH;

    QRect r ( option.rect.topLeft().x()+2, 3+h, _size.width()-4, 17);
    QColor color(0xC3C3C3);
    painter->fillRect(r, color);

    // ..and the page number
    painter->drawText(r, Qt::AlignHCenter, index.data().toString());
    painter->restore();
}

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                                  const QModelIndex &  index ) const
{
    return _size;
}

void ImageListDelegate::slotThumbSize(const QSize& size)
{
    // qDebug() << "Got size" << size;
    _size = size;

}
