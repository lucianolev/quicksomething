/***************************************************************************
 *   Copyright (C) 2008 by Mark Herbert <wirrkpf@googlemail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef QuickStarter_HEADER
#define QuickStarter_HEADER

//Plasma includes
#include <Plasma/Applet>
#include <plasma/widgets/icon.h>

//locale includes
#include "popupdialog.h"
#include "ui_quickstarterConfig.h"
#include "settings.h"

class QuickStarter : public Plasma::Applet
{
Q_OBJECT
  public:
    QuickStarter(QObject *parent, const QVariantList &args);
    ~QuickStarter();

    void init();
  
  protected:
    void createConfigurationInterface(KConfigDialog *parent);

  private slots:
    void slot_iconClicked();
    void configAccepted();
    void showcategories();
    void applySettings(Settings::SettingsType type);

  private:
    PopupDialog *dialog();
    
  private:
    Plasma::Icon *m_icon;
    PopupDialog *m_dialog;
    Settings *m_settings;
    
    QSize m_dialogSize;
    Ui::QuickStarterConfig ui;
};
 
K_EXPORT_PLASMA_APPLET(quickstarter, QuickStarter)
#endif
