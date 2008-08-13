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

#include "quickstarter.h"
#include "applicationmodel.h"

//Qt includes
#include <QGraphicsLinearLayout>
#include <QTimer>
#include <QTreeView>

//KDE includes
#include <KDebug>
#include <KConfigDialog>
#include <KFileItemDelegate>

QuickStarter::QuickStarter(QObject *parent, const QVariantList &args)
  :Plasma::Applet(parent, args)
  ,m_icon(new Plasma::Icon())
  ,m_dialog(0)
  ,m_settings(new Settings(this))
  ,m_dialogSize(QSize())
{
  setHasConfigurationInterface(true);
  resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
}
 
QuickStarter::~QuickStarter()
{
  if (hasFailedToLaunch()) {
    // Do some cleanup here
  } else 
  {
    KConfigGroup cg = config();
    if(m_settings->needsSaving()) {
      m_settings->saveSettings(&cg);
    }
    if(m_dialog) {
      if(m_dialog->size() != m_dialogSize) {
	m_dialogSize = m_dialog->size();
	cg.writeEntry("dialogSize", m_dialogSize);
	emit configNeedsSaving();
      }
	delete m_dialog;
    }
  }
  delete m_icon;
  delete m_settings;
}
 
void QuickStarter::init()
{
 
  /*// A small demonstration of the setFailedToLaunch function
  if (m_icon.isNull()) {
    setFailedToLaunch(true, "No world to say hello");
  } else {

  }*/

  //setup the layout
  QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  connect(m_icon, SIGNAL(clicked()), this, SLOT(slot_iconClicked()));
  layout->addItem(m_icon);
  
  //read the settings
  KConfigGroup cg = config();
  m_dialogSize = cg.readEntry("dialogSize", QSize(300,400));
  m_settings->readSettings(&cg);
  
  connect(m_settings, SIGNAL(settingsChanged(Settings::SettingsType)), this, SLOT(applySettings(Settings::SettingsType)));
  m_icon->setIcon(m_settings->iconName());
  update();

  registerAsDragHandle(m_icon);
  setAspectRatioMode(Plasma::Square);
}

void QuickStarter::applySettings(Settings::SettingsType type)
{
  if(type == Settings::IconName || type == Settings::All) {
    m_icon->setIcon(m_settings->iconName());
    update();
  }
//   if(!m_saveTimer->isActive()) {
//     m_saveTimer->start(600000); //10 minutes
//   }
}

PopupDialog *QuickStarter::dialog()
{
  if(!m_dialog) {
    m_dialog = new PopupDialog(m_settings);
    m_dialog->resize(m_dialogSize);
    m_dialog->applySettings(Settings::All);
  }
  return m_dialog;
}

void QuickStarter::createConfigurationInterface(KConfigDialog *parent)
{
  QWidget *widget = new QWidget;
  ui.setupUi(widget);
  
  ui.iconbutton->setIcon(m_settings->iconName());
  ui.iconbutton->setIconType(KIconLoader::NoGroup, KIconLoader::Application);
  
  ui.iconSizeCombo->setCurrentIndex(ui.iconSizeCombo->findText( QString::number(m_settings->iconSize()) ));
    
  if(m_settings->viewMode() == ItemView::ListMode) {
    ui.viewModeCombo->setCurrentIndex(0);
  } else {
    ui.viewModeCombo->setCurrentIndex(1);
  }
  ui.viewModeCombo->setItemIcon(0, KIcon("view-list-details"));
  ui.viewModeCombo->setItemIcon(1, KIcon("view-list-icons"));
  
//   if(!m_settings->showCustomLabel()) {
//     ui.customLabelBox->setChecked(false);
//   } else {
//     ui.customLabelBox->setChecked(true);
//     ui.customLabel->setEnabled(true);
//     ui.customLabelEdit->setEnabled(true);
//     ui.customLabelEdit->setText(m_settings->customLabel());
//   }
  
  parent->addPage(widget, parent->windowTitle(), icon());
  parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
  connect(ui.categorySelector, SIGNAL(clicked()), this, SLOT(showcategories(/*QWidget *parent*/)));
}

//SLOTS:
void QuickStarter::configAccepted()
{
  m_settings->setCategory(ui.categoryLabel->text());
  m_settings->setIconName(ui.iconbutton->icon());
  m_settings->setIconSize(ui.iconSizeCombo->currentText().toInt());
  //m_settings->setShowCustomLabel(ui.customLabelBox->isChecked());
  //m_settings->setCustomLabel(ui.customLabelEdit->text());
  //m_settings->setShowToolTips(ui.tooltipBox->isChecked());
  
  if(ui.viewModeCombo->currentIndex() == 0) {
    m_settings->setViewMode(ItemView::ListMode);
  } else {
    m_settings->setViewMode(ItemView::IconMode);
  }
}

void QuickStarter::showcategories()
{
  KDialog *dialog = new KDialog();
  dialog->setCaption( "Choose a category" );
  dialog->setButtons( KDialog::Ok | KDialog::Cancel );
  QTreeView *view = new QTreeView(dialog);
  view->setSelectionMode(QAbstractItemView::SingleSelection);
  KFileItemDelegate *delegate = new KFileItemDelegate(dialog);
  view->setItemDelegate(delegate);
  ApplicationModel *model = new ApplicationModel(dialog);
  view->setModel(model);
  
  dialog->setMainWidget(view);
  if(dialog->exec()) {
    ////TODO: get the selected category and save it
  }
  delete dialog;
}


void QuickStarter::slot_iconClicked()
{
  if(!dialog()->isVisible()) {
    dialog()->move(popupPosition(dialog()->size()));
    dialog()->show();
  }
}

#include "quickstarter.moc"
