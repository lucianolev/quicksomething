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

//QT includes
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>

//KDE includes
#include <KFileItemDelegate>

//local includes
#include "popupdialog.h"
#include "applicationmodel.h"
#include "itemview/flipanimator.h"

PopupDialog::PopupDialog(Settings *settings, QWidget * parent, Qt::WindowFlags f)
  :ResizeDialog(parent, f)
  ,m_view(0)
  ,m_settings(settings)
{
  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Popup);
  
  //layout dialog
  QVBoxLayout *l_layoutVertical = new QVBoxLayout(this);
  l_layoutVertical->setSpacing( 0 );
  l_layoutVertical->setMargin( 1 );
  
  setLayout(l_layoutVertical);


  //create the FlipScrollView
  m_view = new ItemView(this);
  KFileItemDelegate *delegate = new KFileItemDelegate(this);
  m_view->setItemDelegate(delegate);

  m_model = new ApplicationModel(this);
  m_model->setRoot(m_settings->category());
  m_view->setModel(m_model);
  m_view->setAnimator(new FlipAnimator(m_view, this));
  l_layoutVertical->addWidget(m_view);
  
  connect(m_settings, SIGNAL(settingsChanged(Settings::SettingsType)),
	  this, SLOT(applySettings(Settings::SettingsType)));
  
}

PopupDialog::~PopupDialog()
{
  m_view->deleteLater();
}

void PopupDialog::applySettings(Settings::SettingsType type)
{
  switch(type){
    case Settings::IconSize:
      m_view->setIconSize(QSize(m_settings->iconSize(), m_settings->iconSize()));
      break;
    case Settings::ViewMode:
      m_view->setViewMode(m_settings->viewMode());
      break;
    case Settings::Category:
      m_model->setRoot(m_settings->category());
      break;
    default:
      m_view->setViewMode(m_settings->viewMode());
      m_view->setIconSize(QSize(m_settings->iconSize(), m_settings->iconSize()));
  }
}

void PopupDialog::hideEvent ( QHideEvent * event )
{
  //m_current = m_start;
  m_view->setRootIndex(QModelIndex());
  m_view->selectionModel()->clear();
  m_view->verticalScrollBar()->setValue(0);
  //m_label->setFileItem(m_current);
  //m_backButton->hide();
  QWidget::hideEvent( event );
  //emit signal_hide();
}

// void PopupDialog::setIconSize(int i)
// {
//   if(m_view->iconSize().height() != i){
//     m_view->setIconSize(QSize(i, i));   
//   }
// }


