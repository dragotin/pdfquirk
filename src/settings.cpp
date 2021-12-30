/*
  This file is part of pdfquirk.
  Copyright 2021, Klaas Freitag <kraft@freisturz.de>

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

#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

#include "settings.h"

Settings::Settings(QObject *parent)
    : QSettings(QString("%1/.config/pdfquirkrc").arg(QDir::homePath()), QSettings::IniFormat, parent)
{

}

namespace {

// note the relPath is relative to the app dir
QString findApp(const QString& appName, const QString&relPath = QString())
{
    // First check the host system
    QString app = QStandardPaths::findExecutable(appName);

    // ...and if that has no result, check relative
    if(app.isEmpty()) {
        QString myPath {QCoreApplication::applicationDirPath()};
        if (!relPath.isEmpty()) {
            myPath.append("/");
            myPath.append(relPath);
        }
        const QStringList appDirs {myPath};

        app= QStandardPaths::findExecutable(app, appDirs);
    }
    return app;

}

}

QString Settings::deskewBin() const
{
    return findApp("deskew");
}

QString Settings::img2pdfBin() const
{
    return findApp("img2pdf");
}

QString Settings::convertBin() const
{
    return findApp("convert");
}
