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

#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include <QSize>

#include "pdfquirkimage.h"

class ImageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ImageModel();

    QStringList files() const;

    int hasImages() { return _images.size() > 0; }

    PdfQuirkImage imageAt(int number);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column,
                      const QModelIndex &parent) override;

    bool removeRows(int row, int count, const QModelIndex &parent) override;
    QStringList mimeTypes() const override;
    bool canDropMimeData(const QMimeData *data,
                         Qt::DropAction action, int /*row*/, int /*column*/,
                         const QModelIndex& /*parent*/) const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;

public slots:

    void addImageFile(const PdfQuirkImage& image, int row = -1);
    void refreshImage(int row);
    void clear();

private:
    QVector<PdfQuirkImage> _images;
};

#endif // IMAGEMODEL_H
