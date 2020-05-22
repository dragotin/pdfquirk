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

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>

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
        delete _process;
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

void Executor::setCommand(const QString& cmd)
{
    _cmd = cmd;
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
        args.append(argstr.split(' '));
        args.append(_outputFile);

        _process = new QProcess;
        connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &Executor::slotFinished);

        _process->start("convert", args);
    } else {
        slotFinished(-3, QProcess::ExitStatus::NormalExit);
    }
}

bool Executor::scan()
{
    QTemporaryDir dir;
    dir.setAutoRemove(false);

    _outputFile = dir.filePath("scan.png");
    _process = new QProcess;
    _process->setStandardOutputFile(_outputFile);

    qDebug() << "Starting" << _cmd;

    connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &Executor::slotFinished);

    _process->start(_cmd);

    return true;
}


void Executor::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (_process) {
        qDebug() << "stderr output: " << _process->readAllStandardError();
    }
    emit finished(exitCode); // FIXME
}



