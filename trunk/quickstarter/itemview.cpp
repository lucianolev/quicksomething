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

#include <KDebug>

ItemView::ItemView(QWidget *parent)
  :ItemViewBase(parent)
{
  setDragDropMode(QAbstractItemView::DragOnly);
  
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


// void ItemView::dragMoveEvent(QDragMoveEvent *event)
// {
// 
//   if( event->pos().y() < autoScrollMargin() ) {
//     QAbstractItemView::dragMoveEvent(event);
//     if(verticalScrollBar()->value() != verticalScrollBar()->minimum()) {
//       return;//avoid entering a folder while the itemview scrolls
//     }
//   } else if( viewport()->height() - event->pos().y() < autoScrollMargin() ){
//     QAbstractItemView::dragMoveEvent(event);
//     if(verticalScrollBar()->value() != verticalScrollBar()->maximum()) {
//       return;//avoid entering a folder while the itemview scrolls
//     }
// 
//   }
// 
// 
//   QModelIndex indexUnderMouse = indexAt(event->pos());
//   
// 
//   if(indexUnderMouse.isValid()) {
//     d->goBack = false;
// 
//     if(model()->hasChildren(indexUnderMouse)) {
// 
//       QModelIndex index = d->hoveredIndex;
//       d->hoveredIndex = indexUnderMouse;
//       update(d->hoveredIndex);
//       update(index);
// 
//       if(d->watchedIndexForEnter != indexUnderMouse) {
// 	d->watchedIndexForEnter = indexUnderMouse;
// 	d->dragEnterTimer->start(DRAG_ENTER_TIME);
//       }
// 
//     } else { 
// 
//       d->watchedIndexForEnter = QModelIndex();
// 
//     }
// 
//   } else if (d->backArrowRect().contains(event->pos()) && rootIndex() != QModelIndex()) {
//     
//     d->watchedIndexForEnter = QModelIndex();
//     
//     if(!d->goBack) {
//       d->goBack = true;
//       d->dragEnterTimer->start(DRAG_ENTER_TIME);
//     }
// 
//   } else {
//     d->watchedIndexForEnter = QModelIndex();
//     d->dragEnterTimer->stop();
//     d->goBack = false;
//   }
// 
//   event->accept();
// }
// 
// void ItemView::dragEnterEvent(QDragEnterEvent *event)
// {
//   event->setAccepted(event->mimeData()->hasUrls());
//   setState(QAbstractItemView::DraggingState);
// }
// 
// void ItemView::dropEvent(QDropEvent *event)
// {
//   KDirSortFilterProxyModel *proxyModel = static_cast<KDirSortFilterProxyModel*>( model());
//   DirModel *model = static_cast<DirModel*>(proxyModel->sourceModel());
//   KFileItem item = model->itemForIndex(proxyModel->mapToSource(rootIndex()));
// 
// 
//   QDropEvent ev(mapToGlobal(event->pos()), event->dropAction(), event->mimeData(),
// 		event->mouseButtons(), event->keyboardModifiers());
// 
//   KonqOperations::doDrop(item, item.url(), &ev, this);
// }
// 
// bool ItemView::viewportEvent(QEvent *event)
// {
//   if(event->type() == QEvent::ToolTip && d->showToolTips) {
//     QHelpEvent *helpEvent = dynamic_cast<QHelpEvent*>(event);
//     QModelIndex index = indexAt(helpEvent->pos());
//     if(!index.isValid() || index == rootIndex()) {
//       return false;
//     }
//     KDirSortFilterProxyModel *proxyModel = dynamic_cast<KDirSortFilterProxyModel*>(model());
//     DirModel *model = dynamic_cast<DirModel*>(proxyModel);
//     KFileItem item = model->itemForIndex(proxyModel->mapToSource(index));
//     QToolTip::showText(mapToGlobal(helpEvent->pos()), item.getToolTipText(), this, visualRect(index));
//     return true;
//   }
//   return QAbstractItemView::viewportEvent(event);
// 
// }
// 
// void ItemView::contextMenuEvent( QContextMenuEvent *event)
// { 
//   QModelIndex index = indexAt(mapFromParent(event->pos()));
//   if(!index.isValid() || index == rootIndex()){
//     return;
//   }
// 
//   KFileItem selectedItem = index.data(KDirModel::FileItemRole).value<KFileItem>();
//   KFileItemList items;
//   items.append( selectedItem );
//   
// 
//   KParts::BrowserExtension::PopupFlags flags = 
//          KParts::BrowserExtension::ShowUrlOperations | KParts::BrowserExtension::ShowProperties;
// 
//   KActionCollection coll(this);
// 
//   KonqPopupMenu *contextMenu = new KonqPopupMenu(items, selectedItem.url() , coll,  0 ,
//                                                    KonqPopupMenu::ShowNewWindow, flags, this,
//                                                    KBookmarkManager::userBookmarksManager());
//   
//   contextMenu->exec(event->globalPos());
//   delete contextMenu;
// }

// void ItemView::open(const QModelIndex &index)
// {
//   
//   if (model()->canFetchMore(index)) {
//     model()->fetchMore(index);
//   }
//   
//   if(model()->hasChildren(index)) {
//     d->previousRootIndex = rootIndex();
//     setRootIndex(index);
//     setCurrentIndex(model()->index(0,0, index));
//     if(d->animationTimeLine->state() == QTimeLine::Running) {
//       d->animationTimeLine->stop();
//       d->animationTimeLine->setDirection(QTimeLine::Backward);
//     }
//     d->animationTimeLine->start();
//   } else {
//     //FIXME: Not generic, appmodel especific
//     QString appname = index.data(Qt::DisplayRole).toString(); 
//     KToolInvocation::startServiceByDesktopName(appname);
//   }
//   emit signal_open(index);
// }




#include "itemview.moc"

