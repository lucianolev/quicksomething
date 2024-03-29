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

#ifndef PopupDialog_HEADER
#define PopupDialog_HEADER

//locale includes
#include "itemview.h"
#include "dialog/resizedialog.h"
#include "settings.h"
#include "applicationmodel.h"

class PopupDialog : public ResizeDialog
{
  Q_OBJECT
  
  public:
    PopupDialog(Settings *settings, QWidget * parent = 0, Qt::WindowFlags f =  Qt::Window);
    ~PopupDialog();

  public slots:
    void applySettings(Settings::SettingsType);
    void openApp(const QModelIndex &index);

  protected:
    //reimplemented from QWidget
    virtual void hideEvent ( QHideEvent * event );

  private:
    ItemView *m_view;
    ApplicationModel *m_model;
    Settings *m_settings;
};

#endif
