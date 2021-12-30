/*
  This file is part of pdfquirk.
  Copyright 2021, Klaas Freitag <kraft@freisturz.de>

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

#include "pdfexporter.h"

#include <QPainter>
#include <QPdfWriter>

namespace  {
QPageSize stringToQtSize(const QString& size) {
    if (size.startsWith("A1"))
        return QPageSize(QPageSize::A1);
    else if (size.startsWith("A2"))
        return QPageSize(QPageSize::A2);
    else if (size.startsWith("A3"))
        return QPageSize(QPageSize::A3);
    else if (size.startsWith("A4"))
        return QPageSize(QPageSize::A4);
    else if (size.startsWith("A5"))
        return QPageSize(QPageSize::A5);
    else if (size.startsWith("A6"))
        return QPageSize(QPageSize::A6);
    else if (size.startsWith("Legal"))
        return QPageSize(QPageSize::Legal);
    else if (size.startsWith("Letter"))
        return QPageSize(QPageSize::Letter);
    else if (size.startsWith("Tabloid"))
        return QPageSize(QPageSize::Tabloid);
    else
        return QPageSize(QPageSize::LastPageSize);
}
}

PDFExporter::PDFExporter(const Settings& settings, QObject *parent) :
    QObject(parent),
    _settings(settings)
{

}

void PDFExporter::setOutputFile(const QString& saveFile)
{
    _saveFile = saveFile;
}

void PDFExporter::buildPdf(const QStringList& files)
{
    const QString size = _settings.value(_settings.SettingsPaperSize, _settings.SettingsPaperSizeDefault).toString();
    const QString orientationStr = _settings.value(_settings.SettingsPaperOrient, _settings.SettingsPaperOrientationDefault).toString();
    const QString marginStr = _settings.value(_settings.SettingsPageMargin, _settings.SettingsPageMarginDefault).toString();

    QScopedPointer<QPdfWriter> pdfWriter(new QPdfWriter(_saveFile));

    float margin {0.0};
    int indx = marginStr.indexOf(' ');
    if (indx > -1) {
        margin = marginStr.left(indx).toFloat();
    }

    pdfWriter->setResolution(150);
    QMarginsF margins (margin, margin, margin, margin);
    QPageLayout::Orientation orientation {QPageLayout::Landscape};
    if (orientationStr == "Portrait")
        orientation = QPageLayout::Portrait;
    QPageSize ps = stringToQtSize(size);
    QPageLayout layout(ps, orientation, margins, QPageLayout::Millimeter);
    layout.setMode(QPageLayout::Mode::FullPageMode);
    pdfWriter->setPageLayout(layout);

    // debug
    float pxPerMM = pdfWriter->resolution()/25.4;
    int marginOffset = margin * pxPerMM;

    QPainter p(pdfWriter.data());
    int windowW = p.device()->width();
    int windowH = p.device()->height();

    int h = windowH - (2*marginOffset);
    int w = windowW - (2*marginOffset);
    // For some reason, this setPen call is important to scale the image
    // correctly.
    p.setPen(QPen(Qt::blue, 2.0, Qt::SolidLine, Qt::RoundCap));

    bool doNewPage {false};
    for( const auto file:files) {
        if (doNewPage) {
            pdfWriter->newPage();
        }
        // draw little points to mark the margins.
        p.drawPoint(QPoint(marginOffset, marginOffset));
        p.drawPoint(QPoint(windowW-marginOffset, marginOffset));

        p.drawPoint(QPoint(marginOffset, windowH-marginOffset));
        p.drawPoint(QPoint(windowW-marginOffset, windowH-marginOffset));

        const QImage img(file);
        QImage scaledImage = img.scaled(w, h,
                                        Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaledImage.setDotsPerMeterX(pdfWriter->resolution()/ 25.4 * 1000.0);
        scaledImage.setDotsPerMeterY(pdfWriter->resolution()/ 25.4 * 1000.0);
        // p.drawRect(marginOffset, marginOffset, w, h);
        p.drawImage(QPoint(marginOffset, marginOffset), scaledImage);
        doNewPage = true;
    }
    p.end();
}
