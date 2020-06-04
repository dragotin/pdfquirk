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
}

void ImageListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
    else
        painter->fillRect(option.rect, option.palette.alternateBase());

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (option.state & QStyle::State_Selected)
        painter->setBrush(option.palette.highlightedText());
    else
        painter->setBrush(QBrush(Qt::black));

    const QString pageStr = index.data().toString();
    int textH = option.fontMetrics.boundingRect(pageStr).height();
    int imgH = option.rect.height() - textH - 3;           // one pix above and below and one at bottom
    int desiredW = qRound( imgH / 1.41) -4;                // 2 pix margin each side.
    int imgWMargin = (option.rect.width() - desiredW) / 2; // same margin left and right
    int imgW = option.rect.width() - 2*imgWMargin;

    const QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>().scaled(imgW, imgH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(option.rect.topLeft().x()+imgWMargin, 1, pix);

    // a little background behinde the "Page n" text
    QRect r (option.rect.topLeft().x()+2, 2+imgH, option.rect.width()-4, textH);
    QColor color(0xC3C3C3);
    painter->fillRect(r, color);

    // ..and the page number
    painter->drawText(r, Qt::AlignHCenter, pageStr );

    painter->restore();
}

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem &  option ,
                                  const QModelIndex &  index ) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return _size;
}

void ImageListDelegate::slotThumbSize(const QSize& size)
{
    // qDebug() << "Got size" << size;
    _size = size;

}
