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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSettings>

#include "imagemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class QAbstractButton;
class SizeCatcher : public QObject
{
    Q_OBJECT
signals:
     void thumbSize(const QSize&);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

// ================================================================

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

public slots:
    void accept() override;

private slots:
    void slotFromFile();
    void slotFromScanner();
    void slotButtonClicked(QAbstractButton *button);
    void pdfCreatorFinished(bool success);
    void slotScanFinished(bool success);

private:
    void updateInfoText(const QString& saveFile = QString());

    Ui::Dialog *ui;

    const QString _SettingsLastFilePath {"lastFilePath"};
    const QString _SettingsScanBW{"scanCmdMonochrome"};

    ImageModel _model;
    QString _lastPath;
    QScopedPointer<QSettings> _settings;
};
#endif // DIALOG_H
