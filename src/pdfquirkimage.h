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

#ifndef PDFQUIRKIMAGE_H
#define PDFQUIRKIMAGE_H
#include <QPixmap>
#include <QString>

class PdfQuirkImage
{
public:
    PdfQuirkImage();
    void setImageFile(const QString& file, bool doCopy = false);

    static QSize ImageSize() { return QSize(200, 282); }

    QString createTempCopy() const;

    QString fileName() const { return _file; }
    QPixmap pixmap() const { return _image; }
    bool    isValid() const { return !_file.isEmpty(); }

private:
    QString _file;
    QString _lastBackup;
    QPixmap _image;
};

#endif // PDFQUIRKIMAGE_H
