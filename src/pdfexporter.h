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
#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include <QObject>

#include "settings.h"

class PDFExporter : public QObject
{
    Q_OBJECT
public:
    explicit PDFExporter(const Settings& settings, QObject *parent = nullptr);

    void setOutputFile(const QString&);
    void buildPdf(const QStringList&);
signals:

private:
    const Settings& _settings;

    QString _saveFile;

};

#endif // PDFEXPORTER_H
