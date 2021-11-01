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
#include <QDate>
#include <QObject>
#include <QResizeEvent>
#include <QDebug>
#include <QtGlobal>
#include <QScrollBar>
#include <QMenu>
#include <QTemporaryDir>
#include <QMessageBox>

#include "imagelistdelegate.h"
#include "executor.h"
#include "pdfquirkimage.h"
#include "version.h"

namespace {

QString aboutText()
{
    const QDate d = QDate::currentDate();

    return QObject::tr( "<h2>About PDF Quirk %1</h2>"
               "<p>PDF Quirk is a simple app to easily create PDFs from scans or images.</p>"
               "<p></p>"
               "<p>It is free software released "
               "under the <a href=\"https://www.gnu.org/licenses/gpl-3.0.de.html\">"
               "Gnu General Public License version 3</a>.</p>"
               "<p>Copyright %2 Klaas Freitag &lt;kraft@freisturz.de&gt;,&nbsp;"
               "<a href=\"https://dragotin.github.io/quirksite/\">https://dragotin.github.io/quirksite/</a>.</p>"
               "Contributions are welcome, find the <a href=\"https://github.com/dragotin/pdfquirk\">sources here</a>"
               " or <a href=\"https://github.com/dragotin/pdfquirk/issues\">report bugs</a>.</p>").arg(VERSION).arg(d.year());
}

} // end namespace


SizeCatcher::SizeCatcher(QObject *parent)
    :QObject(parent)
{

}

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
    ui->listviewThumbs->setResizeMode(QListView::Adjust);
    ui->listviewThumbs->setMaximumHeight(300);
    ui->listviewThumbs->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);

    ui->listviewThumbs->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listviewThumbs->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    ui->listviewThumbs->setDragEnabled(true);
    ui->listviewThumbs->setAcceptDrops(true);
    ui->listviewThumbs->setDragDropMode(QAbstractItemView::InternalMove);
    ui->listviewThumbs->setAcceptDrops(true);
    ui->listviewThumbs->setDropIndicatorShown(true);

    connect( _delegate.data(), &ImageListDelegate::flipImage, this, &Dialog::slotFlipImage);
    connect( _delegate.data(), &ImageListDelegate::deleteImage, this, &Dialog::slotDeleteImage);
    connect( _delegate.data(), &ImageListDelegate::rotateImageLeft, this, &Dialog::slotRotateImageLeft);
    connect( _delegate.data(), &ImageListDelegate::rotateImageRight, this, &Dialog::slotRotateImageRight);
    connect( _delegate.data(), &ImageListDelegate::DeskewImage, this, &Dialog::slotDeskewImage);

    QFont f = ui->listviewThumbs->font();
    QFontMetrics fm(f);
    thumbWidth += 4;
    thumbHeight += 4 + fm.height() + 2;
    // ui->listviewThumbs->setFixedHeight(thumbHeight+6);

    _delegate->slotThumbSize(QSize(thumbWidth, thumbHeight));

    // size catcher
    SizeCatcher *sizeCatcher = new SizeCatcher(this);
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

    if( button == ui->buttonBox->button(QDialogButtonBox::StandardButton::Save)) {
        int currIndx = ui->widgetStack->currentIndex();
        if ( currIndx == _IndxListView || currIndx == _IndxAbout) {
            startPdfCreation();
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
        if (_executor != nullptr) {
            _executor->stop();
        }
        QDialog::reject();
    }
}

int Dialog::getSelectedImageRow()
{
    qDebug() << "Get selected image.";
    QItemSelectionModel *selectionModel = ui->listviewThumbs->selectionModel();

    QModelIndexList selectedItemsIdx = selectionModel->selectedIndexes();

    int imageNo = -1;
    if (selectedItemsIdx.count() > 0) {
        imageNo = selectedItemsIdx.first().row();
    }

    return imageNo;
}

void Dialog::execOpOnSelected(ImageOperation op)
{
    int selected = getSelectedImageRow();
    if (selected == -1) {
        qDebug() << "Flip: Invalid selected row";
        return;
    }
    PdfQuirkImage image = _model.imageAt(selected);

    if (image.isValid()) {
        bool result {false};

        if (op == ImageOperation::FlipImage) {
            qDebug() << "Flipping image" << image.fileName();
            result = _executor->flipImage(image);
        } else if (op == ImageOperation::RotateLeft || op == ImageOperation::RotateRight) {
            qDebug() << "Rotating image";
            result = _executor->rotate(image, op == ImageOperation::RotateLeft ? 270 : 90);
        } else if (op == ImageOperation::Remove) {
            const QString text( tr("Do you want to remove the selected image\n"));
            if (QMessageBox::StandardButton::Yes == QMessageBox::question(this, tr("Remove Image"), text)) {
                result = _executor->removeImage(image);
            }
        } else if (op == ImageOperation::Deskew) {
            startLengthyOperation();
            result = _executor->deskewImage(image);
            endLengthyOperation();
        }

        if (result) {
            qDebug() << "Image operation successfull";
            _model.refreshImage(selected);
        } else {
            qDebug() << "Image operation failed!";
        }
    }
}

void Dialog::slotDeleteImage()
{
    execOpOnSelected(ImageOperation::Remove);
}

void Dialog::slotFlipImage()
{
    execOpOnSelected(ImageOperation::FlipImage);
}

void Dialog::slotRotateImageLeft()
{
    execOpOnSelected(ImageOperation::RotateLeft);
}

void Dialog::slotRotateImageRight()
{
    execOpOnSelected(ImageOperation::RotateRight);
}

void Dialog::slotDeskewImage()
{
    execOpOnSelected(ImageOperation::Deskew);
}


void Dialog::startPdfCreation()
{
    const QStringList files = _model.files();
    const QString path = _settings->value(_SettingsLastFilePath, QDir::homePath()).toString();

    const QString saveFile = QFileDialog::getSaveFileName(this, tr("Save PDF File"), path, "PDF (*.pdf)");
    if (!saveFile.isEmpty()) {
        Executor *creator = new Executor;
        connect(creator, &Executor::finished, this, &Dialog::pdfCreatorFinished);
        creator->setOutputFile(saveFile);
        startLengthyOperation();
        updateInfoText(ProcessStatus::CreatingPdf);
        creator->buildPdf(files);
    }
}

void Dialog::pdfCreatorFinished(int success)
{
    QApplication::restoreOverrideCursor();

    // get the result file name from the creator object.
    QString resultFile;
    Executor *creator = static_cast<Executor*>(sender());
    if (creator) {
        resultFile = creator->outputFile();
        creator->deleteLater();
    }

    if (success == 0) {
        // cleanup: remove the scanned pages
        _model.clear();
        updateInfoText(ProcessStatus::PDFCreated, resultFile);
    } else {
        updateInfoText(ProcessStatus::PDFCreatedFailed);
    }
    endLengthyOperation();
}

void Dialog::slotFromFile()
{
    QString path = _settings->value(_SettingsLastFilePath, QDir::homePath()).toString();

    QStringList files = QFileDialog::getOpenFileNames(this, tr("Add files to PDF"), path, "Images (*.png *.jpeg *.jpg)");

    startLengthyOperation();

    // These are all not our files. Copy as they are added.
    for (const QString& file : files) {
        QFileInfo fi(file);
        qApp->processEvents();

        PdfQuirkImage image;
        qDebug() << "Copying file" << file;
        image.setImageFile(file, true); // copy and add.

        _model.addImageFile(image); // add a file but it is not ours

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
    case ProcessStatus::PDFCreatedFailed:
        str = tr("The PDF could not be created.");
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
    _executor = new Executor;

    QTemporaryDir dir;
    dir.setAutoRemove(false);
    const QString outputFile = dir.filePath("scan.png");

    _executor->setOutputFile(outputFile);

    connect(_executor, &Executor::finished, this, &Dialog::slotScanFinished);
    startLengthyOperation();
    if (!_executor->scan(scanCmd)) {
        slotScanFinished(false);
    } else {
        updateInfoText(ProcessStatus::Scanning);
    }

}

void Dialog::slotScanFinished(int exitCode)
{
    // get the result file name from the creator object.
    QString resultFile;
    if (_executor) {
        resultFile = _executor->outputFile();
        delete _executor;
        _executor = nullptr;
    }
    // exitCode 0 is success
    if (exitCode == 0 && !resultFile.isEmpty()) {
        PdfQuirkImage img;
        img.setImageFile(resultFile);
        _model.addImageFile(img);

        if (_model.rowCount()) {
            updateInfoText(ProcessStatus::ImageScanned, resultFile);
        }

    } else {
        updateInfoText(ProcessStatus::ScanFailed);
    }
    endLengthyOperation();
}

void Dialog::startLengthyOperation()
{
    ui->pbAddFromFile->setEnabled(false);
    ui->pbAddFromScanner->setEnabled(false);
    if (QApplication::overrideCursor() == nullptr)
        QApplication::setOverrideCursor(Qt::WaitCursor);
 }

void Dialog::endLengthyOperation()
{
    if (QApplication::overrideCursor() != nullptr) {
        QApplication::restoreOverrideCursor();
    }
    ui->pbAddFromFile->setEnabled(true);
    ui->pbAddFromScanner->setEnabled(true);
}

Dialog::~Dialog()
{
    _model.clear();
    delete ui;
}

