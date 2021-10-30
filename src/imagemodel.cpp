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

#include "imagemodel.h"

#include<QImage>
#include<QIcon>
#include<QFileInfo>
#include<QDebug>
#include<QMimeData>
#include <QUrl>
#include <QStandardItemModel>
#include <QDir>

ImageModel::ImageModel()
    :QAbstractListModel()
{

}

QStringList ImageModel::files() const
{
    QStringList re;

    for ( auto img : _images )
        re.append(img.fileName());

    return re;
}

Qt::DropActions ImageModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags ImageModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if (!index.isValid())
        flags |= Qt::ItemIsDropEnabled;
    if (rowCount() > 1)
        flags |= Qt::ItemIsDragEnabled;
    return flags;
}

bool ImageModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << "Drop Mime Data";

    if (!canDropMimeData(data, action, row, column, parent)) {
        return false;
    }

    if (action == Qt::IgnoreAction) {
        return true;
    }

    int userow;
    if (row != -1)
        userow = row;
    else if (parent.isValid())
        userow = parent.row();
    else
        userow = rowCount(QModelIndex());

    QList<QUrl> urls = data->urls();

    for (auto url : urls) {
        QModelIndex idx = index(userow++, 0, QModelIndex());
        PdfQuirkImage img;
        img.setImageFile(url.toLocalFile());
        addImageFile(img, idx.row());
    }
    return true;
}

void ImageModel::addImageFile(const PdfQuirkImage& image, int row)
{
    if (row == -1)
        row = _images.size(); // append

    beginInsertRows(QModelIndex(), row, row);

    _images.insert(row, image);
    endInsertRows();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    QVariant var;
    if (!index.isValid() || index.parent().isValid())
        return var;

    if (role == Qt::DecorationRole) {
        if (index.row() < _images.size()) {
            return _images.at(index.row()).pixmap();
        }
    }
    if (role == Qt::DisplayRole) {
        return tr("Page %1").arg(1+index.row());
    }

    return var;
}

bool ImageModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    for (int i = 0; i < count; i++) {
        _images.removeAt(row+i);
    }
    endRemoveRows();
    return true;
}


void ImageModel::refreshImage(int row)
{
    if (row < 0 || row > rowCount())
        return;
    PdfQuirkImage img = _images.at(row);

    QFileInfo fi(img.fileName());

    if (fi.exists()) {
        // the image was changed.
        img.setImageFile(img.fileName()); // refreshes the thumbnail

        QModelIndex idx = index(row, 0, QModelIndex());
        _images.replace(row, img);
        emit dataChanged(idx, idx);
    } else {
        // the image was removed.
        removeRows(row, 1, QModelIndex());
    }
}

QStringList ImageModel::mimeTypes() const
 {
    QStringList types;
    types << QStringLiteral("image/*");
    types << QStringLiteral("text/uri-list");
    return types;
}

bool ImageModel::canDropMimeData(const QMimeData *data,
                                 Qt::DropAction action, int row, int /*column*/, const QModelIndex& /*parent*/) const
{
    bool re { false };
    if ( action == Qt::MoveAction && data->hasUrls()) {
        re = true;
    }
    // qDebug() << "CanDrop on" << row << re;
    return re;
}

QMimeData* ImageModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QList<QUrl> urls;
    for (auto idx : indexes) {
        int r = idx.row();
        urls.append(QUrl::fromLocalFile(_images.at(r).fileName()));
    }

    mimeData->setUrls(urls);
    return mimeData;
}

int ImageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int rows = _images.size();

    return rows;
}

PdfQuirkImage ImageModel::imageAt(int number)
{
    if (number >= 0 && number < _images.size()) {
        return _images.at(number);
    }
    return PdfQuirkImage();
}

void ImageModel::clear()
{
    emit beginResetModel();
    for (auto img : _images) {
        QFileInfo fi(img.fileName());
        QDir d = fi.absoluteDir();
        const QString fileStr = fi.absoluteFilePath();
        QFile::remove(fileStr);
        if (d.isEmpty()) {
            d.removeRecursively();
        }
    }
    _images.clear();
    emit endResetModel();
}
