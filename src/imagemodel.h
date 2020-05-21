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

class ImageModel : public QAbstractListModel
{
public:
    ImageModel();

    QStringList files() const;

    static QSize ImageSize() { return QSize(200, 282); }

    int hasImages() { return _pixmaps.size() > 0; }
public slots:
    void addImageFile( const QString& file );
    void clear();

public:
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:

    QList<QPixmap> _pixmaps;
    QStringList _pathes;
};

#endif // IMAGEMODEL_H
