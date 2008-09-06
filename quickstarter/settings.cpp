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

#include "settings.h"
// #include <KDebug>

Settings::Settings(QObject *parent)
  :QObject(parent)
  ,m_category("")
  ,m_iconName("kde")
  ,m_iconSize(32)
  ,m_viewMode(ItemView::ListMode)
  ,m_needsSaving(false)
{
}

Settings::~Settings()
{
}

void Settings::setCategory(const QString &category)
{
  if(category != m_category) {
    m_category = category;
    m_needsSaving = true;
    emit settingsChanged(Settings::Category);
  }
}

QString Settings::category() const
{
  return m_category;
}

void Settings::setIconName(const QString &name)
{
  if(name != m_iconName) {
    m_iconName = name;
    m_needsSaving = true;
    emit settingsChanged(Settings::IconName);
  }
}

QString Settings::iconName() const
{
  return m_iconName;
}
    
void Settings::setIconSize(const int &size)
{
  if(size != m_iconSize) {
    m_iconSize = size;
    m_needsSaving = true;
    emit settingsChanged(Settings::IconSize);
  }
}

int Settings::iconSize() const
{
  return m_iconSize;
}

void Settings::setViewMode(ItemView::ViewMode mode)
{
  if(mode != m_viewMode) {
    m_viewMode = mode;
    m_needsSaving = true;
    emit settingsChanged(Settings::ViewMode);
  }
}

ItemView::ViewMode Settings::viewMode()
{
  return m_viewMode;
}
    
void Settings::setShowToolTips(bool show)
{
  if(show != m_showToolTips) {
    m_showToolTips = show;
    m_needsSaving = true;
    emit settingsChanged(Settings::ToolTips);
  }
}

bool Settings::showToolTips()
{
  return m_showToolTips;
}

bool Settings::needsSaving()
{
  return m_needsSaving;
}

void Settings::readSettings(KConfigGroup *cg)
{
  m_category = cg->readEntry("category", "");
  m_iconName = cg->readEntry("icon", "kde");
  m_iconSize = cg->readEntry("iconSize", 32);
  m_viewMode = (ItemView::ViewMode)cg->readEntry("ViewMode", 0);
  m_showToolTips = cg->readEntry("ToolTips", false);
}

void Settings::saveSettings(KConfigGroup *cg)
{
  cg->writeEntry("category", m_category);
  cg->writeEntry("icon", m_iconName);
  cg->writeEntry("iconSize", m_iconSize);
  cg->writeEntry("ViewMode", (int)m_viewMode);
  cg->writeEntry("ToolTips", m_showToolTips);
  m_needsSaving = false;
}
  
#include "settings.moc"
