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


ImageModel::ImageModel()
{

}

QStringList ImageModel::files() const
{
    return _pathes;
}

void ImageModel::addImageFile( const QString& file )
{
    QFileInfo fi(file);
    if (!fi.exists()) {
        return;
    }
    QPixmap pix;
    pix.load(file);

    int rold = _pixmaps.size();
    QModelIndex fromIdx = createIndex(rold, 0);
    QModelIndex toIdx = createIndex(rold+1, 0);
         //emit a signal to make the view reread identified data
    emit dataChanged(fromIdx, toIdx, {Qt::DisplayRole});

    _pixmaps.append(pix.scaled(ImageSize()));
    _pathes.append(file);
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    QVariant var;

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
    emit endResetModel();
}
