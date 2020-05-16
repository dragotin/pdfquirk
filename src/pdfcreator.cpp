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

#include "pdfcreator.h"

#include <QObject>
#include <QProcess>

PdfCreator::PdfCreator(QObject *parent)
    : QObject(parent),
      _outputFile {"/tmp/foo.pdf"}
{

}

QString PdfCreator::outputFile()
{
    return _outputFile;
}

void PdfCreator::setOutputFile(const QString& fileName)
{
    _outputFile = fileName;
}

void PdfCreator::buildPdf(const QStringList& files)
{
    if (!files.isEmpty() && !_outputFile.isEmpty()) {
        QStringList args;
        const QString argstr {"-resize 1240x1753 -gravity center -units PixelsPerInch -density 150x150 -background white -extent 1240x1753"};

        args.append(files);
        args.append(argstr.split(' '));
        args.append(_outputFile);

        QProcess *process = new QProcess;
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &PdfCreator::slotFinished);
        process->start("convert", args);
    } else {
        slotFinished(-3, QProcess::ExitStatus::NormalExit);
    }
}

void PdfCreator::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    emit finished(exitCode >= 0); // FIXME

    sender()->deleteLater();
}



