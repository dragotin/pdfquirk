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

#ifndef PDFCREATOR_H
#define PDFCREATOR_H

#include <QObject>
#include <QProcess>

class QProcess;
class PdfQuirkImage;

class Executor : public QObject
{
    Q_OBJECT
public:
    explicit Executor(QObject *parent = nullptr);
    ~Executor();

    void buildPdf(const QStringList& files);
    bool scan(const QString &cmd);

    void setOutputFile(const QString& fileName);
    QString outputFile();

    bool flipImage( const PdfQuirkImage& img);
    bool rotate(const PdfQuirkImage& img, int degree);
    bool removeImage( const PdfQuirkImage& img);

public slots:
    void stop();

signals:
    void finished(bool);
    void error(const QString&);

private slots:
    void slotFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    const QString OutfileTag {"%OUTFILE"};
    QString _outputFile;
    QProcess *_process {nullptr};
};

#endif // PDFCREATOR_H
