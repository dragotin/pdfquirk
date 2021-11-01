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
#include <QDebug>
#include <QRandomGenerator>

#include "pdfquirkimage.h"

namespace {

QString getRandomString()
{
   const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
   const int randomStringLength = 12; // assuming you want random strings of 12 characters

   QString randomString;
   for(int i=0; i<randomStringLength; ++i)
   {
       quint32 value = QRandomGenerator::global()->generate();
       int index = value % possibleCharacters.length();
       QChar nextChar = possibleCharacters.at(index);
       randomString.append(nextChar);
   }
   return randomString;
}


QString copyFile(const QString& srcFile, const QString& targetFileName = QString())
{
    QString targetFN { targetFileName };

    if (srcFile.isEmpty())
        return QString();

    QString targetDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir tDir(targetDir);
    if (!tDir.exists()) {
        tDir.mkpath(targetDir);
    }

    if (targetFN.isEmpty()) { // create a temporar file
        targetFN = getRandomString();
    }

    QFileInfo fi(srcFile);
    const QString backupFileName = QString("%1.%2").arg(targetFN).arg(fi.completeSuffix());
    const QString backupFile = tDir.absoluteFilePath(backupFileName);

    QString resultingFile;

    if (!backupFile.isEmpty() ){
        QFileInfo tdFi(backupFile);

        if (tdFi.exists() && tdFi.isWritable()) {
            QFile::remove(backupFile);
        }
        if (QFile::copy(srcFile, backupFile)) {
            resultingFile = backupFile;
        }
    }

    return resultingFile;
}

} // --- end namespace ---

PdfQuirkImage::PdfQuirkImage()
{

}

QString PdfQuirkImage::createTempCopy() const
{
    const QString f= copyFile(_file);

    return f;
}

void PdfQuirkImage::setImageFile(const QString &file, bool doCopy)
{
    _file = file;
    if (doCopy) {
        const QByteArray ba = QCryptographicHash::hash(file.toUtf8(),QCryptographicHash::Sha1).toHex();
        // create a copy of the file. Use the SHA1 of the filename as name.
        qDebug() << "Copying to" << ba;
        _file = copyFile(_file, ba);
    }

    const QPixmap pix(_file);
    _image = pix.scaled(ImageSize(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

