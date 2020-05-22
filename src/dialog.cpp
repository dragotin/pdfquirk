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
#include "./ui_dialog.h"

#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QEventLoop>
#include <QDialogButtonBox>
#include <QSettings>
#include <QDir>
#include <QObject>
#include <QResizeEvent>
#include <QDebug>
#include <QtGlobal>
#include <QScrollBar>
#include <QMenu>

#include "imagelistdelegate.h"
#include "executor.h"
#include "version.h"

namespace {

QString aboutText()
{
    return QObject::tr( "<h2>About PDF Quirk %1</h2>"
               "<p>PDF Quirk is a simple app to easily create PDFs from scans or images.</p>"
               "<p></p>"
               "<p>It is free software released "
               "under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.de.html\">"
               "Gnu General Public License version 3</a>.</p>"
               "<p>Copyright 2020 Klaas Freitag &lt;kraft@freisturz.de&gt;,&nbsp;"
               "<a href=\"https://dragotin.github.io/quirksite/\">https://dragotin.github.io/quirksite/</a>.</p>"
               "Contributions are welcome, find the <a href=\"https://github.com/dragotin/pdfquirk\">sources here</a>"
               " or <a href=\"https://github.com/dragotin/pdfquirk/issues\">report bugs</a>.</p>").arg(VERSION);
}

} // end namespace


bool SizeCatcher::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        QResizeEvent *sizeEvent = static_cast<QResizeEvent *>(event);
        int h = sizeEvent->size().height();
        qDebug() << "caught height" << h;
        const QSize s(qRound(h/1.41), h);
        emit thumbSize(s);
    }
    return QObject::eventFilter(obj, event);
}

// ==================================================================

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    // Prepare a settings file
    const QString configHome { QString("%1/.config/pdfquirkrc").arg(QDir::homePath()) };
    _settings.reset(new QSettings(configHome, QSettings::IniFormat));

    // Thumbnail sizes
    int thumbWidth = 100;
    int thumbHeight = 141;

    ui->setupUi(this);

    ui->listviewThumbs->setModel(&_model);
    _delegate.reset(new ImageListDelegate());
    ui->listviewThumbs->setItemDelegate(_delegate.data());
    ui->listviewThumbs->setSelectionMode(QAbstractItemView::NoSelection);
    ui->listviewThumbs->setResizeMode(QListView::Adjust);
    ui->listviewThumbs->setMaximumHeight(300);
    ui->listviewThumbs->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    QFont f = ui->listviewThumbs->font();
    QFontMetrics fm(f);
    thumbWidth += 4;
    thumbHeight += 4 + fm.height() + 2;
    // ui->listviewThumbs->setFixedHeight(thumbHeight+6);

    _delegate->slotThumbSize(QSize(thumbWidth, thumbHeight));

    // size catcher
    SizeCatcher *sizeCatcher = new SizeCatcher;
    connect(sizeCatcher, &SizeCatcher::thumbSize, this, &Dialog::slotListViewSize);
    ui->listviewThumbs->installEventFilter(sizeCatcher);

    connect (ui->pbAddFromFile, &QPushButton::clicked, this, &Dialog::slotFromFile);
    connect (ui->pbAddFromScanner, &QPushButton::clicked, this, &Dialog::slotFromScanner);

    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &Dialog::slotButtonClicked);

    updateInfoText(ProcessStatus::JustStarted);
    buildMenu(ui->menuButton);

    ui->widgetStack->setCurrentIndex(_IndxListView);

    ui->labAbout->setText( aboutText() );
}

void Dialog::buildMenu(QToolButton *button)
{
    QMenu *menu   = new QMenu(this);
    QAction *actConfiguration = new QAction(tr("Configuration..."), this);
    QAction *actAbout = new QAction(tr("About PDF Quirk..."), this);

    menu->addAction(actConfiguration);
    menu->addAction(actAbout);
    button->setMenu(menu);
    button->setPopupMode(QToolButton::InstantPopup);
    connect(actConfiguration, &QAction::triggered, this, &Dialog::showConfiguration);
    connect(actAbout, &QAction::triggered, this, &Dialog::showAbout);
}


void Dialog::showConfiguration()
{
    const QString defltCmd;
    const QString colorCmd = _settings->value(_SettingsScanColor, defltCmd).toString();
    const QString monoCmd = _settings->value(_SettingsScanMono, defltCmd).toString();

    ui->leMonoScanCmd->setText(monoCmd);
    ui->leColorScanCmd->setText(colorCmd);

    ui->widgetStack->setCurrentIndex(_IndxConfig);
    updateInfoText(ProcessStatus::ConfigPage);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)->setEnabled(true);
}

void Dialog::showAbout()
{
    ui->widgetStack->setCurrentIndex(_IndxAbout);
    updateInfoText(ProcessStatus::AboutPage);

    ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)->setEnabled(false);
}

void Dialog::showList()
{
    ui->widgetStack->setCurrentIndex(_IndxListView);
    updateInfoText(ProcessStatus::Unknown);

    bool showSave = _model.hasImages();
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)->setEnabled(showSave);
}

void Dialog::slotListViewSize(QSize s)
{
    int spacing = ui->listviewThumbs->contentsMargins().top() + ui->listviewThumbs->contentsMargins().bottom();
    int h = s.height();
    if (ui->listviewThumbs->horizontalScrollBar()->isVisible()) {
        h = s.height() - ui->listviewThumbs->horizontalScrollBar()->height();
    }
    s.setHeight(h - spacing);
    _delegate->slotThumbSize(s);
}

void Dialog::slotButtonClicked(QAbstractButton *button)
{
    if (!button) return;
    const QString path = _settings->value(_SettingsLastFilePath, QDir::homePath()).toString();

    if( button == ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)) {
        int currIndx = ui->widgetStack->currentIndex();
        if ( currIndx == _IndxListView || currIndx == _IndxAbout) {
            QStringList files = _model.files();

            const QString saveFile = QFileDialog::getSaveFileName(this, tr("Save PDF File"), path, "PDF (*.pdf)");
            if (!saveFile.isEmpty()) {
                Executor *creator = new Executor;
                connect(creator, &Executor::finished, this, &Dialog::pdfCreatorFinished);
                creator->setOutputFile(saveFile);
                QApplication::setOverrideCursor(Qt::WaitCursor);
                updateInfoText(ProcessStatus::CreatingPdf);
                creator->buildPdf(files);
            }
        } else if (currIndx == _IndxConfig) {
            const QString colorCmd = ui->leColorScanCmd->text();
            if (colorCmd.isEmpty()) {
                _settings->remove(_SettingsScanColor);
            } else {
                _settings->setValue(_SettingsScanColor, colorCmd);
            }
            const QString monoCmd = ui->leMonoScanCmd->text();
            if (monoCmd.isEmpty()) {
                _settings->remove(_SettingsScanMono);
            } else {
                _settings->setValue(_SettingsScanMono, monoCmd);
            }

            showList();
        }
    }
}

void Dialog::accept()
{
    // do nothing to not close the dialog.
}

void Dialog::reject()
{
    int currIndx = ui->widgetStack->currentIndex();
    if ( currIndx == _IndxConfig || currIndx == _IndxAbout) {
        showList();
    } else {
        if (_scanner != nullptr) {
            _scanner->stop();
        }
        QDialog::reject();
    }
}

void Dialog::pdfCreatorFinished(bool success)
{
    QApplication::restoreOverrideCursor();

    // get the result file name from the creator object.
    QString resultFile;
    Executor *creator = static_cast<Executor*>(sender());
    if (creator) {
        resultFile = creator->outputFile();
        creator->deleteLater();
    }
    if (success) {
        _model.clear();
    }
    updateInfoText(ProcessStatus::PDFCreated, resultFile);
}

void Dialog::slotFromFile()
{
    QString path = _settings->value(_SettingsLastFilePath, QDir::homePath()).toString();

    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add files to PDF"), path, "Images (*.png *.jpeg *.jpg)");

    startLengthyOperation();
    for (const QString& file : files) {
        QFileInfo fi(file);
        qApp->processEvents();
        _model.addImageFile(file);

        path = fi.path();
    }
    endLengthyOperation();
    updateInfoText(ProcessStatus::ImageLoaded);
    _settings->setValue(_SettingsLastFilePath, path);
    _settings->sync();
}

void Dialog::updateInfoText(ProcessStatus stat, const QString& saveFile)
{
    int numPages = _model.rowCount();

    QString str;
    bool openExternal {false};
    switch(stat) {
    case ProcessStatus::Unknown:
        str = QString();
                break;
    case ProcessStatus::JustStarted:
        str =  tr("No images loaded. Load from scanner or file using the buttons above.");
        break;
    case ProcessStatus::ImageScanned:
        str = tr("%1 image(s) scanned. Press Save to create the PDF.").arg(numPages);
        break;
    case ProcessStatus::ImageLoaded:
        str = tr("%1 image(s) loaded. Press Save to create the PDF.").arg(numPages);
        break;
    case ProcessStatus::PDFCreated:
        str = tr("PDF file was saved to <a href=\"file:%1\">%1</a>").arg(saveFile);
        openExternal = true;
        break;
    case ProcessStatus::ScanFailed:
        str = tr("The scan command hasn't succeeded, check the configuration.");
        break;
    case ProcessStatus::Scanning:
        str = tr("Scanning...");
        break;
    case ProcessStatus::CreatingPdf:
        str = tr("Creating the PDF...");
        break;
    case ProcessStatus::ConfigPage:
        str = tr("Handle config parameters and click the Save button.");
        break;
    case ProcessStatus::NotConfigured:
        str = tr("The scan command is not configured. Check Config first!");
        break;
    case ProcessStatus::AboutPage:
        str = tr("Click the Close button to continue.");
        break;
    }

    ui->labInfo->setOpenExternalLinks(openExternal);
    ui->labInfo->setText(str);
    ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)->setEnabled( _model.rowCount() > 0 );
}


void Dialog::slotFromScanner()
{
    QString scanCmd;
    // cmd for network brother MFD scanner:
    // scanimage --mode 'Grey' --resolution 150 -l 0 -t 0 -x 210 -y 297 --format png
    const QString defltCmd{""};

    bool color = ui->cbColorScan->isChecked();
    if (color)
        scanCmd = _settings->value(_SettingsScanColor, defltCmd ).toString();
    else
        scanCmd = _settings->value(_SettingsScanMono, defltCmd ).toString();

    if (scanCmd == defltCmd) {
        updateInfoText(ProcessStatus::NotConfigured);
        return;
    }
    _scanner = new Executor;

    connect(_scanner, &Executor::finished, this, &Dialog::slotScanFinished);
    startLengthyOperation();
    if (!_scanner->scan(scanCmd)) {
        slotScanFinished(false);
    } else {
        updateInfoText(ProcessStatus::Scanning);
    }

}

void Dialog::slotScanFinished(int exitCode)
{
    // get the result file name from the creator object.
    QString resultFile;
    if (_scanner) {
        resultFile = _scanner->outputFile();
        delete _scanner;
        _scanner = nullptr;
    }
    if (exitCode == 0) {
        _model.addImageFile(resultFile);
    }
    endLengthyOperation();

    if (_model.rowCount() && exitCode == 0)
        updateInfoText(ProcessStatus::ImageScanned, resultFile);
    else
        updateInfoText(ProcessStatus::ScanFailed);
}

void Dialog::startLengthyOperation()
{
    if (_lengthyOpRunning) return;
    ui->pbAddFromFile->setEnabled(false);
    ui->pbAddFromScanner->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    _lengthyOpRunning = true;
}

void Dialog::endLengthyOperation()
{
    if (!_lengthyOpRunning) return;
    ui->pbAddFromFile->setEnabled(true);
    ui->pbAddFromScanner->setEnabled(true);
    QApplication::restoreOverrideCursor();
    _lengthyOpRunning = true;
}

Dialog::~Dialog()
{
    delete ui;
}

