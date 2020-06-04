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
class QToolButton;

class ImageListDelegate;
class Executor;

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

    enum class ProcessStatus {
        Unknown,
        JustStarted,
        ImageScanned,
        ImageLoaded,
        PDFCreated,
        ScanFailed,
        Scanning,
        CreatingPdf,
        ConfigPage,
        AboutPage,
        NotConfigured,
        PDFCreatedFailed
    };

public slots:
    void accept() override;
    void reject() override;

private slots:
    void slotFromFile();
    void slotFromScanner();
    void slotButtonClicked(QAbstractButton *button);
    void pdfCreatorFinished(int success);
    void slotScanFinished(int exitCode);
    void slotListViewSize(QSize s);

    void startLengthyOperation();
    void endLengthyOperation();

    void showConfiguration();
    void showAbout();
    void showList();

private:
    void updateInfoText(ProcessStatus stat, const QString& saveFile = QString());
    void buildMenu(QToolButton *button);
    void startPdfCreation();

    const int _IndxListView {0};
    const int _IndxConfig {1};
    const int _IndxAbout {2};

    Ui::Dialog *ui;

    const QString _SettingsLastFilePath {"lastFilePath"};
    const QString _SettingsScanMono{"scanCmdMonochrome"};
    const QString _SettingsScanColor{"scanCmdColor"};

    ImageModel _model;
    QString _lastPath;
    QScopedPointer<QSettings> _settings;
    QScopedPointer<ImageListDelegate> _delegate;
    Executor *_scanner {nullptr};
    QStringList _scans;
};
#endif // DIALOG_H
