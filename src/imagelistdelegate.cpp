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
#include <QEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QStandardPaths>


ImageListDelegate::ImageListDelegate(QObject *parent)
    :QStyledItemDelegate(parent)
{
    _menu.reset(new QMenu(tr("Image Actions")));

    _flipAct = new QAction(QIcon::fromTheme("object-flip-vertical-symbolic"), tr("Flip image vertically"), parent);
    // _flipAct->setShortcuts(QKeySequence::F);
    connect(_flipAct, &QAction::triggered, this, &ImageListDelegate::flipImage);
    _deleteAct = new QAction(QIcon::fromTheme("edit-delete-symbolic"), tr("Remove Image"), parent);
    connect(_deleteAct, &QAction::triggered, this, &ImageListDelegate::deleteImage);
    _rotateLeftAct = new QAction(QIcon::fromTheme("object-flip-vertical-symbolic"), tr("Rotate image left"), parent);
    // _flipAct->setShortcuts(QKeySequence::F);
    connect(_rotateLeftAct, &QAction::triggered, this, &ImageListDelegate::rotateImageLeft);
    _rotateRightAct = new QAction(QIcon::fromTheme("object-flip-vertical-symbolic"), tr("Rotate image right"), parent);
    // _flipAct->setShortcuts(QKeySequence::F);
    connect(_rotateRightAct, &QAction::triggered, this, &ImageListDelegate::rotateImageRight);
    _deskewAct = new QAction(QIcon::fromTheme("object-flip-vertical-symbolic"), tr("Deskew image"), parent);
    // _flipAct->setShortcuts(QKeySequence::F);
    connect(_deskewAct, &QAction::triggered, this, &ImageListDelegate::DeskewImage);

    // check if deskew is installed.
    if (QStandardPaths::findExecutable("deskew").isEmpty()) {
        _deskewAct->setEnabled(false);
        _deskewAct->setToolTip(tr("The command line tool deskew is not installed."));
    }

    _menu->addSection(tr("ImageActions"));
    _menu->addAction(_flipAct);
    _menu->addAction(_rotateLeftAct);
    _menu->addAction(_rotateRightAct);
    _menu->addAction(_deskewAct);
    _menu->addAction(_deleteAct);
}

void ImageListDelegate::initStyleOption(QStyleOptionViewItem *o, const QModelIndex &idx) const
{
    QStyledItemDelegate::initStyleOption(o, idx);
}

void ImageListDelegate::showContextMenu(const QPoint& globalPos) {
    _menu->exec(globalPos);
}

bool ImageListDelegate::editorEvent(
        QEvent* event,
        QAbstractItemModel* model,
        const QStyleOptionViewItem& option,
        const QModelIndex& index )
{
    QMouseEvent* mouseEvent = NULL;

    if (event->type() == QEvent::MouseButtonRelease) {
        // This is only safe because we've checked the type first.
        mouseEvent = static_cast<QMouseEvent*>(event);
    }

    if (mouseEvent and mouseEvent->button() == Qt::RightButton) {
        showContextMenu(mouseEvent->globalPos());

        // Return true to indicate that we have handled the event.
        // Note: This means that we won't get any default behavior!
        return true;
    }

    return QAbstractItemDelegate::editorEvent(event, model, option, index);
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

    bool selected {false};
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.highlightedText());
        selected = true;
    } else {
        painter->setBrush(QBrush(Qt::green));
    }

    const QString pageStr = index.data().toString();
    qDebug() << "Item " << pageStr << "is selected:" << selected;
    int textH = option.fontMetrics.boundingRect(pageStr).height();
    int imgH = option.rect.height() - textH - 3;           // one pix above and below and one at bottom
    int desiredW = qRound( imgH / 1.41) -4;                // 2 pix margin each side.
    int imgWMargin = (option.rect.width() - desiredW) / 2; // same margin left and right
    int imgW = option.rect.width() - 2*imgWMargin;

    const QPixmap pix = index.data(Qt::DecorationRole).value<QPixmap>().scaled(imgW, imgH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter->drawPixmap(option.rect.topLeft().x()+imgWMargin, 1, pix);

    // a little background behind the "Page n" text
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
