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

#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <qmath.h>
#include <QTimeLine>
#include <QToolTip>
#include <QTimer>

#include <KFileItemDelegate>
#include <KGlobalSettings>
#include <KIcon>
#include <KFileItem>
#include <konq_popupmenu.h>
#include <KBookmarkManager>
#include <KToolInvocation>
#include <kdirsortfilterproxymodel.h>
#include <konq_operations.h>

#include <KDebug>
#include <unistd.h>


class ItemView::Private
{
  public:
    Private(ItemView *view);
    ~Private();
    
    void drawBackArrow(QPainter *painter, QStyle::State state) const;
    QRect backArrowRect() const;
    void updateScrollBarRange();
    QPainterPath trianglePath(qreal width = 5, qreal height = 10) const;
    
  public:
    ItemView * const q;
    bool backArrowHover;
    int itemsPerRow; 
    QPersistentModelIndex previousRootIndex;
    QPersistentModelIndex hoveredIndex;
    QPersistentModelIndex watchedIndexForEnter;
    ItemView::ViewMode viewMode;
    QSize gridSize;
    QTimeLine *animationTimeLine;
    QTimer *dragEnterTimer;
    qreal animationTime;
    bool showToolTips;
    bool goBack;
  
  private:

};

ItemView::ItemView(QWidget *parent)
  :QAbstractItemView(parent)
  ,d(new Private(this))
{
  setMouseTracking(true);
  setAutoScroll(true);
  setDragDropMode(QAbstractItemView::DragOnly/*Drop*/);
  
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  
  setFrameShadow(QFrame::Plain);
  
  setViewMode(ItemView::ListMode);
  setIconSize(QSize(16, 16));
  
  //setSelectionMode(QAbstractItemView::ExtendedSelection);
  
  if(KGlobalSettings::singleClick()) {
    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(open(const QModelIndex &)));
  } else {
    connect(this, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(open(const QModelIndex &)));
  }
  
  connect(d->animationTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(updateAnimation(qreal)));
  connect(d->animationTimeLine, SIGNAL(finished()), this, SLOT(timeLineFinished()));
  connect(d->dragEnterTimer , SIGNAL(timeout()), this, SLOT(dragEnter()));
}

ItemView::~ItemView()
{
  delete d;
}

void ItemView::setViewMode(ItemView::ViewMode mode)
{
  if(d->viewMode != mode) {
    d->viewMode = mode;
  }
}

ItemView::ViewMode ItemView::viewMode() const
{
  return d->viewMode;
}

void ItemView::setShowToolTips(bool show)
{
  d->showToolTips = show;
}

bool ItemView::showToolTips()
{
  return d->showToolTips;
}

QRect ItemView::visualRect(const QModelIndex &index) const
{
  if(!index.isValid()) {
    return QRect();
  }

  int topOffset = 0;
  int leftOffset = 0;
  int width = d->gridSize.width();
  
  int backArrowWidth = d->backArrowRect().width() + BACK_ARROW_SPACING;
  
  if(model()->parent(index) != QModelIndex()) {
    if(viewMode() == ItemView::ListMode) {
      width -= backArrowWidth;
    }
    leftOffset += backArrowWidth;
  }

  topOffset -= verticalOffset();
  leftOffset -= horizontalOffset();

  
  int indexRow = index.row() / d->itemsPerRow;

  int indexColumn = index.row() % d->itemsPerRow;
  

  topOffset += indexRow * d->gridSize.height();
  leftOffset += indexColumn * d->gridSize.width();
  
  
  QRect itemRect(leftOffset, topOffset, width , d->gridSize.height());
  return itemRect;
}

void ItemView::setIconSize(const QSize &size)
{
  QAbstractItemView::setIconSize(size);
}

QModelIndex ItemView::indexAt(const QPoint& point) const
{
  if(rootIndex() != QModelIndex() && d->backArrowRect().contains(point)){
    return QModelIndex();
  }
  
  int topOffset = 0 - verticalOffset();
  int leftOffset = 0 - horizontalOffset();
  if(rootIndex() != QModelIndex()) {
    leftOffset += BACK_ARROW_WIDTH;
  }
    
  int rowIndex = (point.y() - topOffset) / d->gridSize.height();
  int columnIndex = (point.x() - leftOffset) / d->gridSize.width();
    
  if(columnIndex >= d->itemsPerRow) return QModelIndex();
    
  int row = rowIndex * d->itemsPerRow + columnIndex;

  
  if(row < model()->rowCount(rootIndex())) {
    return model()->index(row, 0, rootIndex());
  } else {
    return QModelIndex();
  }
}

void ItemView::scrollTo(const QModelIndex& index , ScrollHint hint)
{
  if (!index.isValid()) {
    return;
  }

  QRect itemRect = visualRect(index);
  
  if (itemRect.isValid() && hint == EnsureVisible) {
    if (itemRect.top() < 0) {
      verticalScrollBar()->setValue(verticalScrollBar()->value() +
                                    itemRect.top());
    } else if (itemRect.bottom() > viewport()->height()) {
      verticalScrollBar()->setValue(verticalScrollBar()->value() +
				    (itemRect.bottom()- viewport()->height()));
    }
  }
}

int ItemView::horizontalOffset() const
{
  return horizontalScrollBar()->value();
}

int ItemView::verticalOffset() const
{
  return verticalScrollBar()->value();
}

QModelIndex ItemView::moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers)
{
  Q_UNUSED(cursorAction);
  Q_UNUSED(modifiers);
  
  return QModelIndex();
}

void ItemView::setSelection(const QRect& rect , QItemSelectionModel::SelectionFlags flags)
{
  QItemSelection selection;
  selection.select(indexAt(rect.topLeft()),indexAt(rect.bottomRight()));
  selectionModel()->select(selection,flags);
}

bool ItemView::isIndexHidden(const QModelIndex&) const
{
  return false;
}

QRegion ItemView::visualRegionForSelection(const QItemSelection& selection) const
{
  QRegion region;
  foreach(const QModelIndex& index , selection.indexes()) {
      region |= visualRect(index);
  }
  return region;
}

void ItemView::startDrag(Qt::DropActions supportedActions) 
{
  QDrag *drag = new QDrag(this);
  QMimeData *mimeData = model()->mimeData(selectionModel()->selectedIndexes());
  
  if (!mimeData || mimeData->text().isNull()) {
    return;
  }

  drag->setMimeData(mimeData);


  QModelIndexList list = selectionModel()->selectedIndexes();
  
  if(list.isEmpty()) {
    return;
  }

  QModelIndex idx = list.first();


  QIcon icon = idx.data(Qt::DecorationRole).value<QIcon>();
  drag->setPixmap(icon.pixmap(IconSize(KIconLoader::Desktop)));

  drag->exec(supportedActions);
  QAbstractItemView::startDrag(supportedActions);
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

void ItemView::open(const QModelIndex &index)
{
  
  if (model()->canFetchMore(index)) {
    model()->fetchMore(index);
  }
  
  if(model()->hasChildren(index)) {
    d->previousRootIndex = rootIndex();
    setRootIndex(index);
    setCurrentIndex(model()->index(0,0, index));
    if(d->animationTimeLine->state() == QTimeLine::Running) {
      d->animationTimeLine->stop();
      d->animationTimeLine->setDirection(QTimeLine::Backward);
    }
    d->animationTimeLine->start();
  } else {
    //FIXME: Not generic, appmodel especific
    QString appname = index.data(Qt::DisplayRole).toString(); 
    KToolInvocation::startServiceByDesktopName(appname);
  }
  emit signal_open(index);
}

void ItemView::updateAnimation(qreal value)
{
  d->animationTime = value;
  viewport()->update();
}

void ItemView::timeLineFinished()
{
  d->animationTimeLine->toggleDirection();
  if(d->animationTimeLine->direction() ==  QTimeLine::Forward) {
    d->animationTimeLine->start();
  }
}

void ItemView::paintEvent(QPaintEvent *event)
{  
  relayout();//TODO put that in place where it is not called that often but often enough

  QPainter painter(viewport());
  QStyle::State state = 0;
  if(d->backArrowHover) {
    state |= QStyle::State_MouseOver;
  }

  if(d->animationTimeLine->state() == QTimeLine::Running) {
    if(d->animationTimeLine->direction() == QTimeLine::Backward) { // Fade out
      painter.save();
      painter.setOpacity(d->animationTime);
      paintItems(painter, event, d->previousRootIndex);
      painter.restore();
      
      //draw backarrow
      painter.save();    
      if(rootIndex() == QModelIndex()){
	painter.setOpacity(d->animationTime);
	d->drawBackArrow(&painter, state);
      } else if(d->previousRootIndex == QModelIndex()) {
	//do nothing
      } else { //don't fade
	d->drawBackArrow(&painter, state);
      }
      painter.restore();
    } else { //QTimeLine::Forward Fade in
      painter.save();
      painter.setOpacity(d->animationTime);
      paintItems(painter, event, rootIndex());
      painter.restore();
      
      //draw backarrow
      painter.save();
           
      if(rootIndex() == QModelIndex()){
	//do nothing
      } else if(d->previousRootIndex == QModelIndex()) {
	painter.setOpacity(d->animationTime);
	d->drawBackArrow(&painter, state);
      } else { //don't fade
	d->drawBackArrow(&painter, state);
      }
      painter.restore();
    }
  } else { //NotRunning
    paintItems(painter, event, rootIndex());
    //draw backarrow    
    if(rootIndex() != QModelIndex()){
      d->drawBackArrow(&painter, state);
    }
  }
}
  

void ItemView::paintItems(QPainter &painter, QPaintEvent *event, const QModelIndex &root)
{
  painter.save();
  const int rows = model()->rowCount(root);

  for (int i = 0; i < rows; ++i) {
    QModelIndex index = model()->index(i, 0, root);
    
    QStyleOptionViewItemV4 option = viewOptions();
    
    option.decorationSize = iconSize();
    option.displayAlignment = Qt::AlignHCenter;
    option.textElideMode = Qt::ElideRight;

    if(viewMode() == ItemView::ListMode) {
      option.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
      option.decorationPosition  = QStyleOptionViewItem::Left;
    } else {
      option.features = QStyleOptionViewItemV2::WrapText;
      option.displayAlignment = Qt::AlignCenter;
      option.decorationPosition  = QStyleOptionViewItem::Top;
    }
    option.rect = visualRect(index);

    // only draw items intersecting the region of the widget
    // being updated
    if (!event->rect().intersects(option.rect)) {
       continue;
     }

    if (selectionModel()->isSelected(index)) {
      option.state |= QStyle::State_Selected;
    }

    if (index == d->hoveredIndex) {
      option.state |= QStyle::State_MouseOver;
    }

    if (index == currentIndex()) {
      option.state |= QStyle::State_HasFocus;
    }

    itemDelegate(index)->paint(&painter,option,index);
  }
  painter.restore();
}

void ItemView::relayout()
{
  if(viewMode() == ItemView::IconMode) {
    d->gridSize.setWidth(qMax(iconSize().width() + 10, fontMetrics().width("wwwwwwwwwww")));
    d->gridSize.setHeight(iconSize().height() + 10 + fontMetrics().height()*2);
  } else { // ItemView::ListMode
    d->gridSize.setWidth(viewport()->width());
    d->gridSize.setHeight(qMax(iconSize().height(), fontMetrics().height()));
  }
  d->itemsPerRow = viewport()->width() / d->gridSize.width();
  if(d->itemsPerRow == 0) d->itemsPerRow = 1; //force one column

  d->updateScrollBarRange();
}

void ItemView::dragEnter()
{
  if(state() != QAbstractItemView::DraggingState) {
    d->goBack = false;
    d->watchedIndexForEnter = QModelIndex();
    return;
  }
  
  QPoint pos = mapFromGlobal(QCursor::pos());
  if(d->goBack) {
    
    if(d->backArrowRect().contains(pos)) {
      open(rootIndex().parent());
    }

  } else if(d->watchedIndexForEnter == indexAt(pos)) {
    open(d->watchedIndexForEnter);
  }
  d->watchedIndexForEnter = QModelIndex();
  d->goBack = false;
}

void ItemView::resizeEvent(QResizeEvent *event)
{
  viewport()->update();
  QAbstractItemView::resizeEvent(event);
}

void ItemView::mouseMoveEvent(QMouseEvent *event)
{

  bool mouseOverBackArrow = d->backArrowRect().contains(event->pos());

  if (mouseOverBackArrow != d->backArrowHover) {
    d->backArrowHover = mouseOverBackArrow;
    setDirtyRegion(d->backArrowRect());
  }
  QModelIndex index = indexAt(event->pos());
  //kDebug() << "hoveredIndex:" << index.data(Qt::DisplayRole).toString();
  d->hoveredIndex = index;
  update(index);
  QAbstractItemView::mouseMoveEvent(event);
}

void ItemView::mouseReleaseEvent(QMouseEvent *event)
{
  if(d->backArrowRect().contains(event->pos()) && rootIndex() != QModelIndex()) {
    open(rootIndex().parent());
  }
  QAbstractItemView::mouseReleaseEvent(event);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////PRIVATE///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ItemView::Private::Private(ItemView *view)
  :q(view)
  ,backArrowHover(false)
  ,itemsPerRow(1)
  ,animationTimeLine(new QTimeLine())
  ,dragEnterTimer(new QTimer())
  ,showToolTips(false)
  ,goBack(false)

{
  dragEnterTimer->setSingleShot(true);
  animationTimeLine->setDuration(ANIMATION_TIME);
  animationTimeLine->setDirection(QTimeLine::Backward);
}

ItemView::Private::~Private()
{
  delete animationTimeLine;
  delete dragEnterTimer;  
}

void ItemView::Private::drawBackArrow(QPainter *painter, QStyle::State state) const
{
  painter->save();
  painter->setOpacity(painter->opacity() * 0.5);
  if (state & QStyle::State_MouseOver) {
    painter->setBrush(q->palette().highlight());
  } else {
    painter->setBrush(q->palette().mid());
  }

  QRect rect = backArrowRect();

  // background
  painter->setPen(Qt::NoPen);
  painter->drawRect(rect);

  painter->setPen(QPen(q->palette().dark(), 0));
  painter->drawLine (rect.topRight() + QPointF(0.5, 0),
		     rect.bottomRight() + QPointF(0.5, 0));
		     
  painter->setPen(Qt::NoPen);

  if (state & QStyle::State_MouseOver) {
    painter->setBrush(q->palette().highlightedText());
  } else {
    painter->setBrush(q->palette().light());
  }
  painter->translate(rect.center());
  
  painter->drawPath(trianglePath());
  painter->resetTransform();

  painter->restore();
  
}

QRect ItemView::Private::backArrowRect() const
{
  return QRect(0, 0, BACK_ARROW_WIDTH, q->viewport()->height());
}

void ItemView::Private::updateScrollBarRange()
{
  int childCount = q->model()->rowCount(q->rootIndex());
  int pageSize = q->viewport()->height();
  
  int rowHeight = gridSize.height();
  int rows = qCeil((qreal)childCount / (qreal)itemsPerRow);
  
  q->verticalScrollBar()->setRange(0, (rowHeight * rows) - pageSize);
  q->verticalScrollBar()->setPageStep(pageSize);
  q->verticalScrollBar()->setSingleStep(rowHeight);
}

QPainterPath ItemView::Private::trianglePath(qreal width, qreal height) const
{
  QPainterPath path(QPointF(-width/2,0.0));
  path.lineTo(width,-height/2);
  path.lineTo(width,height/2);
  path.lineTo(-width/2,0.0);

  return path;
}


#include "itemview.moc"

