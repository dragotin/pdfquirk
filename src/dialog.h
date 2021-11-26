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

#include "imagemodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class QAbstractButton;
class QToolButton;

class ImageListDelegate;
class Executor;
class Settings;

class SizeCatcher : public QObject
{
    Q_OBJECT
public:
    SizeCatcher(QObject *parent);

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

    enum class ImageOperation {
        FlipImage,
        RotateLeft,
        RotateRight,
        Deskew,
        Remove
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

    void slotDeleteImage();
    void slotFlipImage();
    void slotRotateImageLeft();
    void slotRotateImageRight();
    void slotDeskewImage();

private:
    void execOpOnSelected(ImageOperation op);
    void updateInfoText(ProcessStatus stat, const QString& saveFile = QString());
    void buildMenu(QToolButton *button);
    void startPdfCreation();
    int getSelectedImageRow();

    const int _IndxListView {0};
    const int _IndxConfig {1};
    const int _IndxAbout {2};

    Ui::Dialog *ui;

    ImageModel _model;
    QString _lastPath;
    Settings *_settings;
    QScopedPointer<ImageListDelegate> _delegate;
    Executor *_executor {nullptr};
};
#endif // DIALOG_H
