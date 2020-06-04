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

ImageModel::ImageModel()
    :QAbstractListModel()
{

}

QStringList ImageModel::files() const
{
    return _pathes;
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
        addImageFile(url.toLocalFile(), idx.row());
    }
    return true;
}

void ImageModel::addImageFile( const QString& file, int row )
{
    QFileInfo fi(file);
    if (!fi.exists()) {
        return;
    }

    QPixmap pix;
    pix.load(file);

    if (row == -1)
        row = _pixmaps.size();

    beginInsertRows(QModelIndex(), row, row);
    _pixmaps.insert(row, pix.scaled(ImageSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    _pathes.insert(row, file);
    endInsertRows();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    QVariant var;
    if (!index.isValid() || index.parent().isValid())
        return var;

    if (role == Qt::DecorationRole) {
        if (index.row() < _pixmaps.size()) {
            return _pixmaps.at(index.row());
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
        _pixmaps.removeAt(row+i);
        _pathes.removeAt(row+i);
    }
    endRemoveRows();
    return true;
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
        urls.append(QUrl::fromLocalFile(_pathes.at(r)));
    }

    mimeData->setUrls(urls);
    return mimeData;
}

int ImageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int rows = _pixmaps.size();

    return rows;
}

void ImageModel::clear()
{
    emit beginResetModel();
    _pixmaps.clear();
    _pathes.clear();
    emit endResetModel();
}
