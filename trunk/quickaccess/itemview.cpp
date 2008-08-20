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

#include "itemview.h"

#include <QDropEvent>
#include <QScrollBar>
#include <QToolTip>

#include <kdirsortfilterproxymodel.h>
#include <konq_popupmenu.h>
#include <KBookmarkManager>

#include "dirmodel.h"

#include "itemview/animator.h"

ItemView::ItemView(QWidget *parent)
  :ItemViewBase(parent)
  ,m_goBack(false)
  ,m_watchedIndexForEnter(QModelIndex())
  ,m_showToolTips(false)
{
  m_dragEnterTimer.setSingleShot(true);
  setDragDropMode(QAbstractItemView::DragDrop);
  connect(&m_dragEnterTimer , SIGNAL(timeout()), this, SLOT(dragEnter()));
}

ItemView::~ItemView()
{
  
}

void ItemView::setShowToolTips(bool show)
{
  m_showToolTips = show;
}

bool ItemView::showToolTips()
{
  return m_showToolTips;
}

void ItemView::dragEnterEvent(QDragEnterEvent *event)
{
  event->setAccepted(event->mimeData()->hasUrls());
  setState(QAbstractItemView::DraggingState);
}

void ItemView::dropEvent(QDropEvent *event)
{
  m_dragEnterTimer.stop();
  KDirSortFilterProxyModel *proxyModel = static_cast<KDirSortFilterProxyModel*>( model());
  DirModel *model = static_cast<DirModel*>(proxyModel->sourceModel());
  KFileItem item = model->itemForIndex(proxyModel->mapToSource(rootIndex()));


  QDropEvent ev(mapToGlobal(event->pos()), event->dropAction(), event->mimeData(),
		event->mouseButtons(), event->keyboardModifiers());

  KonqOperations::doDrop(item, item.url(), &ev, this);
}

void ItemView::dragMoveEvent(QDragMoveEvent *event)
{

  if( event->pos().y() < autoScrollMargin() ) {
    QAbstractItemView::dragMoveEvent(event);
    if(verticalScrollBar()->value() != verticalScrollBar()->minimum()) {
      return;//avoid entering a folder while the itemview scrolls
    }
  } else if( viewport()->height() - event->pos().y() < autoScrollMargin() ){
    QAbstractItemView::dragMoveEvent(event);
    if(verticalScrollBar()->value() != verticalScrollBar()->maximum()) {
      return;//avoid entering a folder while the itemview scrolls
    }

  }


  QModelIndex indexUnderMouse = indexAt(event->pos());
  

  if(indexUnderMouse.isValid()) {
    m_goBack = false;
    
    if(model()->hasChildren(indexUnderMouse)) {

      QModelIndex index = hoveredIndex();
      setHoveredIndex(indexUnderMouse);
      update(hoveredIndex());
      update(index);

      if(m_watchedIndexForEnter != indexUnderMouse) {
	m_watchedIndexForEnter = indexUnderMouse;
	m_dragEnterTimer.start(DRAG_ENTER_TIME);
      }

    } else { 

      m_watchedIndexForEnter = QModelIndex();

    }

  } else if (animator()->backArrowRect().contains(event->pos()) && rootIndex() != QModelIndex()) {
    
    m_watchedIndexForEnter = QModelIndex();
    
    if(!m_goBack) {
      m_goBack = true;
      m_dragEnterTimer.start(DRAG_ENTER_TIME);
    }

  } else {
    m_watchedIndexForEnter = QModelIndex();
    m_dragEnterTimer.stop();
    m_goBack = false;
  }

  event->accept();
}

bool ItemView::viewportEvent(QEvent *event)
{
  if(event->type() == QEvent::ToolTip && m_showToolTips) {
    QHelpEvent *helpEvent = dynamic_cast<QHelpEvent*>(event);
    QModelIndex index = indexAt(helpEvent->pos());
    if(!index.isValid() || index == rootIndex()) {
      return false;
    }
    KDirSortFilterProxyModel *proxyModel = dynamic_cast<KDirSortFilterProxyModel*>(model());
    DirModel *model = dynamic_cast<DirModel*>(proxyModel->sourceModel());
    KFileItem item = model->itemForIndex(proxyModel->mapToSource(index));
    QToolTip::showText(mapToGlobal(helpEvent->pos()), item.getToolTipText(), this, visualRect(index));
    return true;
  }
  return QAbstractItemView::viewportEvent(event);

}

void ItemView::contextMenuEvent( QContextMenuEvent *event)
{ 
  QModelIndex index = indexAt(mapFromParent(event->pos()));
  if(!index.isValid() || index == rootIndex()){
    return;
  }

  QModelIndexList list = selectionModel()->selectedIndexes();
  
  if(list.isEmpty()) {
    return;
  }

  KFileItemList items;
  foreach(const QModelIndex &i, list) {
    KFileItem selectedItem = i.data(KDirModel::FileItemRole).value<KFileItem>();
    if(!selectedItem.isNull()) {
      items.append(selectedItem);
    }
  }
  

  KParts::BrowserExtension::PopupFlags flags = 
         KParts::BrowserExtension::ShowUrlOperations | KParts::BrowserExtension::ShowProperties;

  KActionCollection coll(this);
  
  KDirSortFilterProxyModel *proxyModel = dynamic_cast<KDirSortFilterProxyModel*>(model());
  DirModel *model = dynamic_cast<DirModel*>(proxyModel->sourceModel());
  KFileItem item = model->itemForIndex(proxyModel->mapToSource(index));

  KonqPopupMenu *contextMenu = new KonqPopupMenu(items, item.url() , coll,  0 ,
                                                   KonqPopupMenu::ShowNewWindow, flags, this,
                                                   KBookmarkManager::userBookmarksManager());
  
  if(contextMenu->exec(event->globalPos()) != 0){
    qobject_cast<QWidget*>(parent())->hide();
  }
  delete contextMenu;
}

void ItemView::mouseReleaseEvent(QMouseEvent *event)
{
  if(event->button() == Qt::MidButton) {
    if(animator()->backArrowRect().contains(event->pos()) && rootIndex() != QModelIndex()) {
      openInBrowser(rootIndex().parent());
    } else if (indexAt(event->pos()).isValid()) {
      openInBrowser(indexAt(event->pos()));
    }
  } else {
    ItemViewBase::mouseReleaseEvent(event);
  }
}

void ItemView::dragEnter()
{
  if(state() != QAbstractItemView::DraggingState) {
    m_goBack = false;
    m_watchedIndexForEnter = QModelIndex();
    return;
  }
  
  QPoint pos = mapFromGlobal(QCursor::pos());
  if(m_goBack) {
    
    if(animator()->backArrowRect().contains(pos)) {
      open(rootIndex().parent());
    }

  } else if(m_watchedIndexForEnter == indexAt(pos)) {
    open(m_watchedIndexForEnter);
  }
  m_watchedIndexForEnter = QModelIndex();
  m_goBack = false;
}

void ItemView::openInBrowser(const QModelIndex &index)
{
  selectionModel()->clearSelection();
  emit signal_openInBrowser(index);
}

