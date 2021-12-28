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
#include "settings.h"
#include "pdfquirkimage.h"

#include <QObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>
#include <QSettings>

namespace {

// QProcess is a bit of a false friend when it comes to passing the command as
// string. If the command uses quotes it goes wrong with the string command.
// The command arguments have to be passed as StringList.
// The following method parses the command string and returns a Stringlist
// trying to handle the quotes properly.

QStringList parseStringArgs(const QString& cmd)
{
    bool openSingle {false};
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStringList l = cmd.split(" ", QString::SkipEmptyParts);
#else
    QStringList l = cmd.split(" ", Qt::SkipEmptyParts);
#endif
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


Executor::Executor(Settings& settings, QObject *parent)
    : QObject(parent),
      _settings {settings}
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

#define D(X) qRound(150*X)

void Executor::buildPdf(const QStringList& files)
{
    const QMap<QString, int> ShortSides {{"A1", D(23.4)}, {"A2", D(16.5)}, {"A3", D(11.7)}, {"A4", D(8.3) }, {"A5", D(5.8)}, {"A6", D(4.1)}, {"Legal", D(8.5) }, {"Letter", D(8.5) }, {"Tabloid", D(11.0)}, };
    const QMap<QString, int> LongSides { {"A1", D(33.1)}, {"A2", D(23.4)}, {"A3", D(16.5)}, {"A4", D(11.7)}, {"A5", D(8.3)}, {"A6", D(5.8)}, {"Legal", D(14.0)}, {"Letter", D(11.0)}, {"Tabloid", D(17.0)}, };

    // bind ot img2pdf for now
    const QString toolToUse {"convert"};
    QString toolFile;

    const QString size = _settings.value(_settings.SettingsPaperSize, _settings.SettingsPaperSizeDefault).toString();
    const QString orientation = _settings.value(_settings.SettingsPaperOrient, _settings.SettingsPaperOrientationDefault).toString();
    const QString margin = _settings.value(_settings.SettingsPageMargin, _settings.SettingsPageMarginDefault).toString();

    if (!files.isEmpty() && !_outputFile.isEmpty()) {

        const QString sizeIndx = size.split(" ").at(0);
        QStringList args;
        if (toolToUse == "convert") {

            // all is calculated for a density of 150 pixel per inch
            // set the page size
            int pShort = ShortSides[sizeIndx];
            int pLong  = LongSides[sizeIndx];

            if (orientation == "Landscape") {
                int i = pLong;
                pLong = pShort;
                pShort = i;
            }
            args.append("-page");
            args.append(QString("%1x%2").arg(pShort).arg(pLong));

            args.append(files);

            // substract the border size for resizing.
            QString m = margin.split(" ").at(0);
            bool ok;
            int marginInMM = m.toInt(&ok);

            int marginPx = 0;
            if (ok) {
                // 1 inch = 150px = 25.4mm => 1mm = 150/25.4 px
                marginPx = 2 * qRound(marginInMM * 150 / 25.4);
            }
            int imgShort = pShort - marginPx;
            int imgLong  = pLong - marginPx;
            const QString resizeStr = QString("%1x%2>").arg(imgShort).arg(imgLong);
            args.append("-resize");
            args.append(resizeStr);

            args.append("-gravity");
            args.append("Center");

            args.append("-extent");
            args.append(resizeStr);

            args.append("-units");
            args.append("PixelsPerInch");

            args.append("-density");
            args.append("150x150");

            args.append("-background");
            args.append("white");

            args.append(_outputFile);
            const QString dbg = args.join(" ");
            toolFile = _settings.convertBin();
        } else if (toolToUse == "img2pdf") {
            args.append("--output");
            args.append(_outputFile);
            args.append("--pagesize");
            QString s = size.split(" ").at(0);
            if (orientation == "Landscape")
                s.append("^T");

            args.append(s);
            args.append("--fit");
            args.append("into");
            args.append("--border");
            QString m = margin.split(" ").at(0);
            m.append("mm");
            args.append(m);
            args.append(files);
            toolFile = _settings.img2pdfBin();
        }

        _process = new QProcess;
        connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, &Executor::slotFinished);
        connect(_process, &QProcess::errorOccurred, this, &Executor::slotErrorOccurred);

        _process->start(toolFile, args);
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
        connect(_process, &QProcess::errorOccurred, this, &Executor::slotErrorOccurred);

        qDebug() << "Starting" << bin << args;
        _process->start(bin, args);
        result = true;
    }
    return result;
}

void Executor::slotErrorOccurred(QProcess::ProcessError error)
{
    int exitCode = 1; // general error
    if (error == QProcess::FailedToStart) {
        exitCode = NotFoundExitCode;
    }
    slotFinished(exitCode, QProcess::CrashExit);
}

void Executor::slotFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)
    if (_process) {
        qDebug() << "stderr output: " << _process->readAllStandardError();
    }
    emit finished(exitCode);
}

bool Executor::flipImage(const PdfQuirkImage& img)
{
    if (!img.isValid())
        return false;

    const QString newName = img.createTempCopy();
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

    const QString tmpFile = img.createTempCopy();
    bool re {false};
    // rotate the image from the new, hidden file to the original file
    if (!tmpFile.isEmpty()) {
        QStringList args;
        args << "-rotate";
        args << QString::number(degree);
        args << tmpFile;
        args << img.fileName();
        if (QProcess::execute("convert", args) == 0) {
            re = true;
            QFile::remove(tmpFile);
        }
    }
    return re;
}

bool Executor::deskewImage(PdfQuirkImage& img)
{
    if (!img.isValid())
        return false;

    const QString deskewApp = _settings.deskewBin();
    if (deskewApp.isEmpty())
        return false;

    const QString tmpFile = img.createTempCopy();
    bool re {false};
    // rotate the image from the new, hidden file to the original file
    if (!tmpFile.isEmpty()) {
        QStringList args;
        args << "-o";
        args << img.fileName();
        args << "-b";
        args << "FFFFFF"; // create a white background
        args << tmpFile;
        if (QProcess::execute(deskewApp, args) == 0) {
            re = true;
            QFile::remove(tmpFile);
        }
    }
    return re;

}

bool Executor::removeImage( const PdfQuirkImage& img)
{
    bool re{false};

    if (img.isValid()) {
        QFileInfo fi(img.fileName());

        if (!img.fileName().isEmpty() && fi.isWritable()) {
            re = QFile::remove(img.fileName());
        }
    }

    return re;
}





