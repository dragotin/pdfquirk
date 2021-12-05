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

#include "dialog.h"

#include <QApplication>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationName("pdfquirk");
    a.setOrganizationDomain("");
    a.setApplicationDisplayName("PDF Quirk");

    QTranslator translator;
    const QString locale = QLocale().bcp47Name();
    qDebug() << "Starting application" << a.applicationName();
    const QString qmFile { QString(":/i18n/%1_%2.qm").arg(a.applicationName()).arg(locale) };
    qDebug() << "Loading QM resource:" << qmFile;
    if (translator.load(qmFile)) {
         QCoreApplication::installTranslator(&translator);
         qDebug() << "Success.";
    }

    Dialog w;
    w.show();
    return a.exec();
}
