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

#include "executor.h"
#include "pdfquirkimage.h"

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>
#include <QRegExp>

namespace {

// QProcess is a bit of a false friend when it comes to passing the command as
// string. If the command uses quotes it goes wrong with the string command.
// The command arguments have to be passed as StringList.
// The following method parses the command string and returns a Stringlist
// trying to handle the quotes properly.

QStringList parseStringArgs(const QString& cmd)
{
    bool openSingle {false};
    QStringList l = cmd.split(QRegExp(" +"));
    QStringList re;
    QString escaped;
    for (auto part : l) {
        if (part.startsWith(QChar('\''))) {
            if (openSingle) {
                qDebug() << "ERROR: Open single found twice.";
                return QStringList();
            }
            openSingle = true;
            escaped = part.mid(1); // skip the '
        } else if (part.endsWith(QChar('\''))) {
            if (!openSingle) {
                qDebug() << "ERROR: Close single but no open.";
                return QStringList();
            }
            openSingle = false;
            escaped.append(" ");
            part.chop(1);
            escaped.append(part);
            re.append(escaped);
            escaped.clear();
        } else {
            if (openSingle) {
                escaped.append(" ");
                escaped.append(part);
            } else {
                re.append(part);
            }
        }
    }
    return re;
}

}


Executor::Executor(QObject *parent)
    : QObject(parent),
      _outputFile {"/tmp/foo.pdf"}
{

}

Executor::~Executor()
{
    if (_process != nullptr) {
        if (_process->state() == QProcess::Running) {
            _process->kill();
            qDebug() << "KILLED the process!";
        }
    }
}

QString Executor::outputFile()
{
    return _outputFile;
}

void Executor::setOutputFile(const QString& fileName)
{
    _outputFile = fileName;
}

void Executor::stop()
{
    if (_process)
        _process->terminate();
}

void Executor::buildPdf(const QStringList& files)
{
    if (!files.isEmpty() && !_outputFile.isEmpty()) {
        QStringList args;
        const QString argstr {"-resize 1240x1753 -gravity center -units PixelsPerInch -density 150x150 -background white -extent 1240x1753"};

        args.append(files);
        args.append(parseStringArgs(argstr));
        args.append(_outputFile);

        _process = new QProcess;
        connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &Executor::slotFinished);

        _process->start("convert", args);
    } else {
        slotFinished(-3, QProcess::ExitStatus::NormalExit);
    }
}

bool Executor::scan(const QString& cmd)
{
    bool result {false};

    QStringList args  = parseStringArgs(cmd);
    if (args.size() > 0) {
        _process = new QProcess;

        // if the args contain the %OUTFILE tag, replace that with the temp file
        // otherwise set the standard out file accordingly.
        int outPos = args.indexOf(OutfileTag);
        if (outPos > -1 ) {
            args.replace(outPos, _outputFile);
        } else {
            _process->setStandardOutputFile(_outputFile);
        }

        const QString bin = args.takeFirst();
        connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &Executor::slotFinished);

        qDebug() << "Starting" << bin << args;
        _process->start(bin, args);
        result = true;
    }
    return result;
}

bool Executor::flipImage(const PdfQuirkImage& img)
{
    if (!img.isValid())
        return false;

    const QString newName = img.keepBackup();
    bool re {false};
    // flip the image from the new, hidden file to the original file
    if (!newName.isEmpty()) {
        QStringList args;
        args << "-flip";
        args << newName;
        args << img.fileName();
        if (QProcess::execute("convert", args) == 0)
            re = true;
    }
    return re;
}

bool Executor::rotate(const PdfQuirkImage& img, int degree)
{
    if (!img.isValid() || degree == 0 || degree > 359)
        return false;

    const QString newName = img.keepBackup();
    bool re {false};
    // rotate the image from the new, hidden file to the original file
    if (!newName.isEmpty()) {
        QStringList args;
        args << "-rotate";
        args << QString::number(degree);
        args << newName;
        args << img.fileName();
        if (QProcess::execute("convert", args) == 0)
            re = true;
    }
    return re;
}

bool Executor::removeImage( const PdfQuirkImage& img)
{
    bool re{false};

    if (img.isValid()) {
        const QString backupName = img.keepBackup();
        QFileInfo fi(img.fileName());

        if (!backupName.isEmpty() && fi.isWritable()) {
            re = QFile::remove(img.fileName());
        }
    }

    return re;
}

void Executor::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    if (_process) {
        qDebug() << "stderr output: " << _process->readAllStandardError();
        _process->deleteLater();
    }
    emit finished(exitCode); // FIXME
}



