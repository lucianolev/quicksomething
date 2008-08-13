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

#ifndef Settings_HEADER
#define Settings_HEADER

#include <QObject>

#include <KConfigGroup>

#include "itemview.h"

class Settings : public QObject
{
  Q_OBJECT
  public:
    
    enum SettingsType {
      IconName = 0,
      IconSize = 1,
      //CustomLabel = 2,
      //ToolTips = 3,
      ViewMode = 2,
      Category = 3,
      All = 4
    };
    
    Settings(QObject *parent = 0);
    ~Settings();
    
    void setCategory(const QString &category);
    QString category() const;

    void setIconName(const QString &name);
    QString iconName() const;
    
    void setIconSize(const int &size);
    int iconSize() const;
    
    void setViewMode(ItemView::ViewMode mode);
    ItemView::ViewMode viewMode();

//     void setCustomLabel(const QString &label);
//     QString customLabel() const;
//     
//     void setShowCustomLabel(bool show);
//     bool showCustomLabel();
//     
//     void setShowToolTips(bool show);
//     bool showToolTips();
    
    bool needsSaving();
    
    void saveSettings(KConfigGroup *cg);
    void readSettings(KConfigGroup *cg);
      
  signals:
    void settingsChanged(Settings::SettingsType type);
    
  private:
    
    QString m_category;
    QString m_iconName;
    int m_iconSize;
    ItemView::ViewMode m_viewMode;
    //QString m_customLabel;
    //bool m_showCustomLabel;
    //bool m_showToolTips;
    bool m_needsSaving;
};

#endif //Settings_HEADER
