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

#include "popupdialog.h"

//QT includes
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QMenu>
#include <QActionGroup>
#include <QToolBar>

//KDE includes
// #include <KDebug>
#include <KDirLister>
#include <kdirsortfilterproxymodel.h>
#include <KFileItemDelegate>



//Plasma
#include <Plasma/Theme>


//local includes
#include "itemview.h"
#include "button.h"
#include "itemview/fadeanimator.h"
#include "itemview/flipanimator.h"



PopupDialog::PopupDialog(Settings *settings, QWidget * parent, Qt::WindowFlags f)
  :ResizeDialog(parent, f)
  ,m_settings(settings)
  ,m_label(0)
  ,m_start(KFileItem())
  ,m_current(KFileItem())
  ,m_view(0)
  ,m_model(0)
  ,m_proxyModel(0)
  ,m_iconManager(0)
  ,m_delegate(0)
{
  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Popup);
  

  //layout dialog
  QVBoxLayout *l_layoutVertical = new QVBoxLayout(this);
  l_layoutVertical->setSpacing( 0 );
  l_layoutVertical->setMargin( 1 );
  
  
  //create the Label and buttons
  QHBoxLayout *l_layoutHorizontal = new QHBoxLayout();
  l_layoutHorizontal->setSpacing( 0 );
  l_layoutHorizontal->setMargin( 1 );
  l_layoutVertical->addLayout(l_layoutHorizontal);
  m_backButton = new Button(this);
  m_backButton->setIcon(KIcon("go-up"));
  l_layoutHorizontal->addWidget(m_backButton);
  m_backButton->setHidden(true);
  m_label = new Label(m_settings, this);
  l_layoutHorizontal->addWidget(m_label);
  m_sortButton = new Button(this);
  l_layoutHorizontal->addWidget(m_sortButton);

  
  m_sortGroup = new QActionGroup(this);
  //group->setExclusive(true);
  
  QAction *name = new QAction(i18n("Sort by name"), m_sortGroup);
  name->setCheckable(true);
  name->setObjectName("name");
  QAction *size = new QAction(i18n("Sort by size"), m_sortGroup);
  size->setCheckable(true);
  size->setObjectName("size");
  QAction *type = new QAction(i18n("Sort by type"), m_sortGroup);
  type->setCheckable(true);
  type->setObjectName("type");
  QAction *modified = new QAction(i18n("Sort by last modified"), m_sortGroup);
  modified->setCheckable(true);
  modified->setObjectName("modified");
  
  if(m_settings->sortColumn() == KDirModel::Name){
    name->setChecked(true);
  } else if(m_settings->sortColumn() == KDirModel::Size) {
    size->setChecked(true);
  } else if(m_settings->sortColumn() == KDirModel::Type) {
    type->setChecked(true);
  } else {
    modified->setChecked(true);
  }
  
  m_sortMenu = new QMenu(this);
  m_sortMenu->addAction(name);
  m_sortMenu->addAction(size);
  m_sortMenu->addAction(type);
  m_sortMenu->addAction(modified);
  m_sortButton->setMenu(m_sortMenu);
  m_sortButton->setPopupMode(QToolButton::MenuButtonPopup);
  
  l_layoutVertical->addSpacing(5);
  
  
  //create the ItemView
  m_view = new ItemView(this);
  m_view->setFocus();
  
  m_model = new DirModel(this);
  m_proxyModel = new KDirSortFilterProxyModel(this);
  m_proxyModel->setSourceModel(m_model);
  m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_proxyModel->setFilterKeyColumn(0);
  m_view->setModel(m_proxyModel);
  
  m_delegate = new KFileItemDelegate(this);
  m_delegate->setShadowColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
  m_delegate->setShadowBlur(8);
  m_delegate->setShadowOffset(QPointF(0,0));
  m_view->setItemDelegate(m_delegate);
  m_view->setAnimator(new FlipAnimator(m_view, this));

  l_layoutVertical->addWidget(m_view);
  
  m_view->setIconSize(QSize(16,16));//IconManager needs a valid iconSize
  m_iconManager = new IconManager(m_view, m_proxyModel);
  
  connect(m_view, SIGNAL( signal_open(const QModelIndex &)), this, SLOT(slot_open(const QModelIndex&)));
  connect(m_view, SIGNAL( signal_openInBrowser(const QModelIndex &)), this, SLOT(slot_openInBrowser(const QModelIndex&)));
  connect(m_label, SIGNAL( clicked() ), this, SLOT( open() ) );
  connect(m_model->dirLister(), SIGNAL(completed()), m_view->viewport(), SLOT(update()));
  connect(m_settings, SIGNAL(settingsChanged(Settings::SettingsType)), this, SLOT(applySettings(Settings::SettingsType)));
  connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), SLOT(updateColors()));
  connect(m_backButton, SIGNAL(clicked()), m_view, SLOT(open()));
  connect(m_sortButton, SIGNAL(clicked()), this, SLOT(sortButtonClicked()));
  connect(m_sortGroup, SIGNAL(triggered(QAction*)), this, SLOT(slot_sortingTriggered()));

  
}

PopupDialog::~PopupDialog()
{
  delete m_label;
  m_view->deleteLater();
  delete m_model;
  delete m_proxyModel;
  delete m_iconManager;
  delete m_delegate;
  delete m_backButton;
  delete m_sortButton;
}

void PopupDialog::updateColors()
{
  m_delegate->setShadowColor(Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
}

void PopupDialog::applySettings(Settings::SettingsType type)
{
  switch(type) {
    case Settings::IconSize:
      m_view->setIconSize(QSize(m_settings->iconSize(), m_settings->iconSize()));
      if(m_settings->showPreviews()) {
	m_model->dirLister()->stop();
	m_model->dirLister()->openUrl(m_settings->url(), KDirLister::Reload);
      }
      break;
    case Settings::Preview:
      m_iconManager->setShowPreview(m_settings->showPreviews());
      m_iconManager->setPreviewPlugins(m_settings->previewPlugins());
      m_model->dirLister()->stop();
      m_model->dirLister()->openUrl(m_settings->url(), KDirLister::Reload);
      break;
    case Settings::ShowHiddenFiles:
      m_model->dirLister()->setShowingDotFiles(m_settings->showHiddenFiles());
      m_model->dirLister()->stop();
      m_model->dirLister()->openUrl(m_settings->url(), KDirLister::Reload);
      break;
    case Settings::ShowOnlyDirs:
      m_model->dirLister()->setDirOnlyMode(m_settings->showOnlyDirs());
      m_model->dirLister()->stop();
      m_model->dirLister()->openUrl(m_settings->url(), KDirLister::Reload);
      break;
    case Settings::Filter:
      m_proxyModel->setFilterWildcard(m_settings->filter());
      break;
    case Settings::Url:
      setStartUrl(m_settings->url());
      break;
    case Settings::ToolTips:
      m_view->setShowToolTips(m_settings->showToolTips());
      break;
    case Settings::ViewMode:
      m_view->setViewMode(m_settings->viewMode());
      break;
    case Settings::Sorting:
      updateSorting();
      break;
    case Settings::All:
      m_view->setIconSize(QSize(m_settings->iconSize(), m_settings->iconSize()));
      m_iconManager->setShowPreview(m_settings->showPreviews());
      m_iconManager->setPreviewPlugins(m_settings->previewPlugins());
      m_model->dirLister()->setShowingDotFiles(m_settings->showHiddenFiles());
      m_model->dirLister()->setDirOnlyMode(m_settings->showOnlyDirs());
      m_proxyModel->setFilterWildcard(m_settings->filter());
      setStartUrl(m_settings->url());
      m_view->setShowToolTips(m_settings->showToolTips());
      m_view->setViewMode(m_settings->viewMode());
      updateSorting();
      break;
    default:
      break;
  }
}

void PopupDialog::hideEvent ( QHideEvent * event )
{
  m_current = m_start;
  m_view->setRootIndex(QModelIndex());
  m_view->selectionModel()->clear();
  m_view->verticalScrollBar()->setValue(0);
  m_label->setFileItem(m_current);
  m_backButton->hide();
  QWidget::hideEvent( event );
  emit signal_hide();
}

void PopupDialog::setStartUrl(const KUrl &url)
{
  if( !m_model->dirLister()->openUrl( url ) )
    kDebug() << "can not open url: " << url;
  connect(m_model->dirLister(), SIGNAL(completed()), this, SLOT(dirListerCompleted()));
}

void PopupDialog::dirListerCompleted()
{
  disconnect(m_model->dirLister(), SIGNAL(completed()), this, SLOT(dirListerCompleted()));
  m_current = m_model->dirLister()->rootItem();
  m_start = m_current;
  m_label->setFileItem(m_current);
}

void PopupDialog::open()
{
  m_current.run();
  hide();
}

void PopupDialog::updateSorting()
{
  if(m_settings->sortOrder() == Qt::AscendingOrder) {
    m_sortButton->setIcon(KIcon("view-sort-ascending"));
  } else {
    m_sortButton->setIcon(KIcon("view-sort-descending"));
  }
  m_proxyModel->sort(m_settings->sortColumn(), m_settings->sortOrder());
}

void PopupDialog::slot_sortingTriggered()
{
  QAction *triggered = m_sortGroup->checkedAction();
  if(triggered->objectName() == "name") {
    m_settings->setSortColumn(KDirModel::Name);
  } else if(triggered->objectName() == "size") {
    m_settings->setSortColumn(KDirModel::Size);
  } else if(triggered->objectName() == "type") {
    m_settings->setSortColumn(KDirModel::Type);
  } else {
    m_settings->setSortColumn(KDirModel::ModifiedTime);
  }
}

void PopupDialog::sortButtonClicked()
{
  if(m_settings->sortOrder() == Qt::AscendingOrder) {
    m_settings->setSortOrder(Qt::DescendingOrder);
  } else {
    m_settings->setSortOrder(Qt::AscendingOrder);
  }
}

void PopupDialog::slot_open(const QModelIndex &index)
{
  if(index.isValid()) {
    m_backButton->show();
    m_current = m_model->itemForIndex(m_proxyModel->mapToSource(index));
  } else {
    m_backButton->hide();
    m_current = m_start;
  }
  if(m_current.isFile() || !m_settings->allowNavigation()) {
    m_current.run();
    hide();
  }
  m_label->setFileItem(m_current);
}

void PopupDialog::slot_openInBrowser(const QModelIndex &index)
{
  m_model->itemForIndex(m_proxyModel->mapToSource(index)).run();
}

#include "popupdialog.moc"
