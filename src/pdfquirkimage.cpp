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

#include <QPixmap>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QBuffer>
#include <QDir>

#include "pdfquirkimage.h"

PdfQuirkImage::PdfQuirkImage()
    :_ourfile {false}
{

}

void PdfQuirkImage::setImageFile(const QString &file)
{
    const QPixmap pix(file);
    _image = pix.scaled(ImageSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    _file = file;
}

QString PdfQuirkImage::backupFile() const
{
    if (_file.isEmpty())
        return QString();

    QString targetDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir tDir(targetDir);
    if (!tDir.exists()) {
        tDir.mkpath(targetDir);
    }
    QCryptographicHash hash(QCryptographicHash::Sha1);
    QBuffer f;
    f.setData(_file.toUtf8());
    hash.addData(&f);
    const QString backupFile = tDir.absoluteFilePath(QString(hash.result().toHex()));

    return backupFile;
}

// copy the image to a secret place to be able to restore later.
QString PdfQuirkImage::keepBackup() const
{
    QString resultingFile;
    const QString backup = backupFile();
    QFileInfo tdFi(backup);
    if (!backup.isEmpty() ){
        if (tdFi.exists() && tdFi.isWritable()) {
            QFile::remove(backup);
        }
        if (QFile::copy(_file, backup)) {
            resultingFile = backup;
        }
    }

    return resultingFile;
}
