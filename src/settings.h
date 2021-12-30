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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QObject>

class Settings : public QSettings
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);

    const QString SettingsLastFilePath {"lastFilePath"};
    const QString SettingsScanMono{"scanCmdMonochrome"};
    const QString SettingsScanColor{"scanCmdColor"};
    const QString SettingsPaperSize{"img2pdf/paperSize"};
    const QString SettingsPaperOrient{"img2pdf/paperOrientation"};
    const QString SettingsPageMargin{"img2pdf/paperMargin"};

    // Default-Values:
    const QString SettingsPaperSizeDefault {"A4       210mmx297mm"};
    const QString SettingsPaperOrientationDefault {"Portrait"};
    const QString SettingsPageMarginDefault {"5 Millimeter"};

    // return the path of the deskew binary. Empty if not installed.
    QString deskewBin() const;
    QString img2pdfBin() const;
    QString convertBin() const;
};

#endif // SETTINGS_H
