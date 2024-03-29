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



#include "quickaccess.h"

//Qt includes
#include <QGraphicsLinearLayout>
#include <QTimer>


//KDE includes
#include <KIconLoader>
//#include <KDebug>
#include <KUrl>
#include <KConfigDialog>
#include <konq_operations.h>
#include <KMessageBox>
#include <KRun>

//Plasma


QuickAccess::QuickAccess(QObject *parent, const QVariantList &args)
  :Plasma::Applet(parent, args)
  ,m_settings(new Settings(this))
  ,m_icon(new Plasma::Icon(this))
  ,m_dialog(0)
  ,m_dialogSize(QSize())
  ,m_dragOver(false)
  ,m_saveTimer(new QTimer(this))
{
  if(args.size() == 1){
    m_settings->setUrl(args.first().toString());
  }
  setHasConfigurationInterface(true);
  setAcceptDrops(true);
  m_saveTimer->setSingleShot(true);
  resize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));

}
 
 
QuickAccess::~QuickAccess()
{
  if (hasFailedToLaunch()) {
    // Do some cleanup here
  } else {
    saveSettings();
    if(m_dialog) {
      delete m_dialog;
    }
  }
  delete m_icon;
  delete m_settings;
  delete m_saveTimer;
}
 
void QuickAccess::init()
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
  m_icon->installEventFilter(this);
  
  //read the config
  KConfigGroup cg = config();
  m_dialogSize = cg.readEntry("dialogSize", QSize(300,400));
  
  m_settings->readSettings(&cg);
  connect(m_settings, SIGNAL(settingsChanged(Settings::SettingsType)), this, SLOT(applySettings(Settings::SettingsType)));
  m_icon->setIcon(m_settings->iconName());
  update();
  

  registerAsDragHandle(m_icon);
  setAspectRatioMode(Plasma::ConstrainedSquare);
  
  connect(m_saveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));
}

void QuickAccess::applySettings(Settings::SettingsType type)
{
  if(type == Settings::IconName) {
    m_icon->setIcon(m_settings->iconName());
  }
  
  if(type == Settings::CustomLabel) {
    if(m_settings->customLabelInApplet()) {
      m_icon->setText(m_settings->customLabel());
      m_icon->setOrientation(m_settings->customLabelTextOrientation());
    } else {
      m_icon->setText(QString());
    }
  }
  
  if(type == Settings::All) {
    m_icon->setIcon(m_settings->iconName());
    if(m_settings->customLabelInApplet()) {
      m_icon->setText(m_settings->customLabel());
      m_icon->setOrientation(m_settings->customLabelTextOrientation());
    } else {
      m_icon->setText(QString());
    }
  }
  
  if(!m_saveTimer->isActive()) {
    m_saveTimer->start(600000); //10 minutes
  }
}

void QuickAccess::saveSettings()
{
  KConfigGroup cg = config();
  bool save = false;
  if(m_settings->needsSaving()){
    save = true;
    m_settings->saveSettings(&cg);
  }
  if(m_dialog) {
    if(m_dialog->size() != m_dialogSize) {
      save = true;
      m_dialogSize = m_dialog->size();
      cg.writeEntry("dialogSize", m_dialogSize);
    }
  }
  if(save) {
    emit configNeedsSaving();
  }
}

void QuickAccess::createConfigurationInterface(KConfigDialog *parent)
{
  KTabWidget *widget = new KTabWidget;
  ui.setupUi(widget);
  
  pluginWidget = new PluginWidget(widget);
  
  ui.urlRequester->setMode(KFile::Directory | KFile::ExistingOnly);
  ui.urlRequester->setUrl(m_settings->url());
  
  ui.iconbutton->setIcon(m_settings->iconName());
  ui.iconbutton->setIconType(KIconLoader::NoGroup, KIconLoader::Place);
  
  ui.iconSizeCombo->setCurrentIndex(ui.iconSizeCombo->findText( QString::number(m_settings->iconSize()) ));
  
  if(m_settings->viewMode() == ItemViewBase::ListMode) {
    ui.viewModeCombo->setCurrentIndex(0);
  } else {
    ui.viewModeCombo->setCurrentIndex(1);
  }
  ui.viewModeCombo->setItemIcon(0, KIcon("view-list-details"));
  ui.viewModeCombo->setItemIcon(1, KIcon("view-list-icons"));
    
  
  ui.previewBox->setChecked(m_settings->showPreviews());
  
  //disable the previewplugins options when previews are disabled
  if(!m_settings->showPreviews()) {
    ui.previewLabel1->setEnabled(false);
    ui.previewLabel2->setEnabled(false);
    pluginWidget->setEnabled(false);
  }
  
  ui.hiddenBox->setChecked(m_settings->showHiddenFiles());
  
  ui.onlyDirsBox->setChecked(m_settings->showOnlyDirs());
  
  ui.navigationBox->setChecked(m_settings->allowNavigation());
  
  ui.filterEdit->setText(m_settings->filter());
  
  if(!m_settings->showCustomLabel()) {
    ui.customLabelBox->setChecked(false);
  } else {
    ui.customLabelBox->setChecked(true);
    ui.customLabel->setEnabled(true);
    ui.customLabelEdit->setEnabled(true);
    ui.customLabelEdit->setText(m_settings->customLabel());
    ui.customLabelInApplet->setEnabled(true);
    if(!m_settings->customLabelInApplet()) {
      ui.customLabelInApplet->setChecked(false);
    } else {
      ui.customLabelBox->setEnabled(true);
      ui.customLabelShowIcon->setChecked(m_settings->customLabelShowIcon());
      if(m_settings->customLabelTextOrientation() == Qt::Horizontal) {
	ui.customLabelTextOrientation->setCurrentIndex(0);
      } else {
	ui.customLabelTextOrientation->setCurrentIndex(1);
      }
    }
  }
  
  ui.tooltipBox->setChecked(m_settings->showToolTips());
  
  pluginWidget->setActivePlugins(m_settings->previewPlugins());
  ui.pluginLayout->addWidget(pluginWidget);
  
  
  parent->addPage(widget, parent->windowTitle(), icon());
  parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
  connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
  connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
  
  connect(ui.previewBox, SIGNAL(toggled(bool)), this, SLOT(showPreviewToggled(bool)));
  
}

void QuickAccess::configAccepted()
{
  KUrl url = ui.urlRequester->url();
  url.adjustPath(KUrl::RemoveTrailingSlash);
  
  if(url.protocol() == "applications") {
    KMessageBox::sorry(ui.urlRequester, i18n("Sorry, but the \"applications:\" KIO slave is not supported, because it will crash QuickAccess/Plasma..."));
  } else {
    m_settings->setUrl(url);
  }
  m_settings->setIconName(ui.iconbutton->icon());
  m_settings->setIconSize(ui.iconSizeCombo->currentText().toInt());
  m_settings->setShowPreviews(ui.previewBox->isChecked());
  m_settings->setShowHiddenFiles(ui.hiddenBox->isChecked());
  m_settings->setShowOnlyDirs(ui.onlyDirsBox->isChecked());
  m_settings->setAllowNavigation(ui.navigationBox->isChecked());
  m_settings->setFilter(ui.filterEdit->text());
  m_settings->setShowCustomLabel(ui.customLabelBox->isChecked());
  m_settings->setCustomLabel(ui.customLabelEdit->text());
  m_settings->setCustomLabelInApplet(ui.customLabelInApplet->isChecked());
  m_settings->setCustomLabelShowIcon(ui.customLabelShowIcon->isChecked());
  
  if(ui.customLabelTextOrientation->currentIndex() == 0) {
    m_settings->setCustomLabelTextOrientation(Qt::Vertical);
  } else {
    m_settings->setCustomLabelTextOrientation(Qt::Horizontal);
  }
  
  m_settings->setShowToolTips(ui.tooltipBox->isChecked());
  
  QStringList list = pluginWidget->activePlugins();
  qSort(list); //sort it...
  m_settings->setPreviewPlugins(list);
  
  if(ui.viewModeCombo->currentIndex() == 0) {
    m_settings->setViewMode(ItemViewBase::ListMode);
  } else {
    m_settings->setViewMode(ItemViewBase::IconMode);
  }
  
}

void QuickAccess::showPreviewToggled(bool checked)
{
    ui.previewLabel1->setEnabled(checked);
    ui.previewLabel2->setEnabled(checked);
    pluginWidget->setEnabled(checked);
}

PopupDialog* QuickAccess::dialog()
{
  if(!m_dialog) {
    //create the dialog
    m_dialog = new PopupDialog(m_settings);
    m_dialog->resize(m_dialogSize);
    m_dialog->applySettings(Settings::All);
    connect(m_dialog, SIGNAL(signal_hide()), m_icon, SLOT(setUnpressed()));
  }
  return m_dialog;
}

void QuickAccess::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
  event->setAccepted(event->mimeData()->hasUrls());
  m_dragOver = true;
  QTimer::singleShot(1000, this, SLOT(slotDragEvent()));  
}

void QuickAccess::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
  Q_UNUSED(event);
  m_dragOver = false;
  
}

void QuickAccess::dropEvent(QGraphicsSceneDragDropEvent *event)
{
  m_dragOver = false;
  
  KFileItem item = dialog()->startItem();

  QDropEvent ev(event->screenPos(), event->dropAction(), event->mimeData(),
		event->buttons(), event->modifiers());

  KonqOperations::doDrop(item, item.url(), &ev, event->widget());
}

bool QuickAccess::eventFilter(QObject *object, QEvent *event)
{
  if(event->type() != QEvent::MouseButtonRelease) {
    QMouseEvent *e = static_cast<QMouseEvent*>(event);
    if(e->button() == Qt::MidButton) {
      KRun::runUrl(m_settings->url(), "inode/directory", 0);
      return true;
    }
  }
  
  return Plasma::Applet::eventFilter(object, event);
}

//SLOTS:
void QuickAccess::slotDragEvent()
{
  if(!m_dragOver) {
    return;
  }
  dialog()->move(popupPosition(dialog()->size()));
  dialog()->show();
}

void QuickAccess::slot_iconClicked()
{
  if(!dialog()->isVisible()) {
    m_icon->setPressed(true);
    dialog()->move(popupPosition(dialog()->size()));
    dialog()->show();
  }
}

#include "quickaccess.moc"
