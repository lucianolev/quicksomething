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

#include "itemviewbase.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QScrollBar>
#include <QApplication>
#include <QTimeLine>
#include <qmath.h>

#include <KGlobalSettings>
#include <KIcon>
#include <KIconLoader>
#include <KConfigGroup>
#include <KDebug>

#include <Plasma/Theme>
#include <plasma/paintutils.h>

#include "animator.h"

class ItemViewBase::Private
{
  public:
    Private(ItemViewBase *view);
    ~Private();
    
    void drawBackArrow(QPainter *painter, QStyle::State state) const;
    QRect backArrowRect() const;
    void updateScrollBarRange();
    QPainterPath trianglePath(qreal width = 5, qreal height = 10) const;
    
  public:
    ItemViewBase * const q;
    Animator *animator;
    bool backArrowHover;
    int itemsPerRow; 
    QPersistentModelIndex previousRootIndex;
    QPersistentModelIndex hoveredIndex;
    QPersistentModelIndex watchedIndexForEnter;
    ItemViewBase::ViewMode viewMode;
    QSize gridSize;
    bool goBack;
    QTimeLine *scrollTimeLine;
    int scrollBarValue;
    int wheelScrollLines;
};



ItemViewBase::ItemViewBase(QWidget *parent)
  :QAbstractItemView(parent)
  ,d(new Private(this))
{
  setMouseTracking(true);
  setAutoScroll(true);
   
  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setTextElideMode(Qt::ElideRight);
  setFrameShadow(QFrame::Plain);
  
  if(KGlobalSettings::singleClick()) {
    connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(open(const QModelIndex &)));
  } else {
    connect(this, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(open(const QModelIndex &)));
  }
  
  connect(d->scrollTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(updateScrollAnimation(qreal)));
  connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarValueChanged(int)));
  
  connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), this, SLOT(wheelScrollLinesChanged(int)));
  wheelScrollLinesChanged(KGlobalSettings::SETTINGS_MOUSE);
  
}

ItemViewBase::~ItemViewBase()
{
  delete d;
}

void ItemViewBase::setViewMode(ItemViewBase::ViewMode mode)
{
  if(d->viewMode != mode) {
    d->viewMode = mode;
  }
}

ItemViewBase::ViewMode ItemViewBase::viewMode() const
{
  return d->viewMode;
}

void ItemViewBase::setAnimator(Animator *animator)
{
  if(d->animator != 0) {
    delete d->animator;
  }
  d->animator = animator;
}

Animator *ItemViewBase::animator()
{
  return d->animator;
}

QModelIndex ItemViewBase::previousRootIndex() const
{
  return d->previousRootIndex;
}

QModelIndex ItemViewBase::hoveredIndex() const
{
  return d->hoveredIndex;
}

bool ItemViewBase::isItemVisible(const QModelIndex &index) const
{
  return visualRect(index).intersects(viewport()->rect());//FIXME use the offset
}

QRect ItemViewBase::visualRect(const QModelIndex &index) const
{
  if(!index.isValid()) {
    return QRect();
  }

  int topOffset = 0;
  int leftOffset = 0;
  int width = d->gridSize.width();
  
  int backArrowWidth = d->animator->backArrowRect().width() + d->animator->backArrowSpacing();
  
  if(model()->parent(index) != QModelIndex()) {
    if(viewMode() == ItemViewBase::ListMode) {
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

void ItemViewBase::setIconSize(const QSize &size)
{
  QAbstractItemView::setIconSize(size);
}

QModelIndex ItemViewBase::indexAt(const QPoint& point) const
{
  if(rootIndex() != QModelIndex() && d->animator->backArrowRect().contains(point)){
    return QModelIndex();
  }
  
  int topOffset = 0 - verticalOffset();
  int leftOffset = 0 - horizontalOffset();
  if(rootIndex() != QModelIndex()) {
    leftOffset += d->animator->backArrowRect().width();
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

void ItemViewBase::scrollTo(const QModelIndex& index , ScrollHint hint)
{
  if (!index.isValid()) {
    return;
  }

  QRect itemRect = visualRect(index);
  
  if (itemRect.isValid() && hint == EnsureVisible) {
    d->scrollBarValue = verticalScrollBar()->value();
    if (itemRect.top() < 0) {
      d->scrollBarValue += itemRect.top();

    } else if (itemRect.bottom() > viewport()->height()) {
      d->scrollBarValue +=  itemRect.bottom() - viewport()->height();

    }
    if(d->scrollTimeLine->state() == QTimeLine::Running) {
      d->scrollTimeLine->stop();
    }
    d->scrollTimeLine->setFrameRange(verticalScrollBar()->value(), d->scrollBarValue);
    d->scrollTimeLine->start();
  }
}

int ItemViewBase::horizontalOffset() const
{
  return horizontalScrollBar()->value();
}

int ItemViewBase::verticalOffset() const
{
  return verticalScrollBar()->value();
}

QModelIndex ItemViewBase::moveCursor(CursorAction cursorAction,Qt::KeyboardModifiers modifiers)
{
  Q_UNUSED(modifiers);
  QModelIndex index = currentIndex();
  switch (cursorAction) {
    case MoveUp:
      if (viewMode() == ItemViewBase::IconMode) {
	index = model()->index(index.row() - d->itemsPerRow, 0, rootIndex());
      } else {
	index = model()->index(index.row() - 1, 0, rootIndex());
      }
      setCurrentIndex(index);
      break;
    case MoveDown:
      if (viewMode() == ItemViewBase::IconMode) {
	index = model()->index(index.row() + d->itemsPerRow, 0, rootIndex());
      } else {
	index = model()->index(index.row() + 1, 0, rootIndex());
      }
      setCurrentIndex(index);
      break;
    case MoveLeft:
      if (viewMode() == ItemViewBase::IconMode) {
	index = model()->index(index.row() - 1, 0, rootIndex());
	setCurrentIndex(index);
      }
      break;
    case MoveRight:
      if (viewMode() == ItemViewBase::IconMode) {
	index = model()->index(index.row() + 1, 0, rootIndex());
	setCurrentIndex(index);
      }
      break;
    default:
      // Do nothing
      break;
  }

  // clear the hovered index
  update(d->hoveredIndex);
  d->hoveredIndex = index;

  return index;
}

void ItemViewBase::setSelection(const QRect& rect , QItemSelectionModel::SelectionFlags flags)
{
  QItemSelection selection;
  for(int i = 0; i < model()->rowCount(rootIndex()); i++) {
    if(!visualRect(model()->index(i, 0, rootIndex())).intersects(rect)) continue;
    
    int start = i;
    int end = i;

    while (i < model()->rowCount(rootIndex()) && visualRect(model()->index(i, 0, rootIndex())).intersects(rect)) {
      end = i++;
    }
    selection.select(model()->index(start, 0, rootIndex()), model()->index(end, 0, rootIndex()));
  }
   
  selectionModel()->select(selection,flags);
}

bool ItemViewBase::isIndexHidden(const QModelIndex&) const
{
  return false;
}

QRegion ItemViewBase::visualRegionForSelection(const QItemSelection& selection) const
{
  QRegion region;
  foreach(const QModelIndex& index , selection.indexes()) {
      region |= visualRect(index);
  }
  return region;
}

void ItemViewBase::startDrag(Qt::DropActions supportedActions) 
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

void ItemViewBase::scrollBarValueChanged(int value)
{
  if(d->scrollTimeLine->state() != QTimeLine::Running) {
    d->scrollBarValue = value;
  }
}

void ItemViewBase::wheelScrollLinesChanged(int category)
{
  if(category != KGlobalSettings::SETTINGS_MOUSE) return;
  
  KConfigGroup group = KGlobal::config()->group("KDE");
  d->wheelScrollLines = group.readEntry("WheelScrollLines", 3);  
}

void ItemViewBase::open(const QModelIndex &index)
{
  const Qt::KeyboardModifiers modifier = QApplication::keyboardModifiers();
  if(modifier == Qt::ShiftModifier || modifier == Qt::ControlModifier) {
    return;
  }
  
  if (model()->canFetchMore(index)) {
    model()->fetchMore(index);
  }
  
  if(model()->hasChildren(index)) {
    d->previousRootIndex = rootIndex();
    setRootIndex(index);
    d->animator->animate();
    setCurrentIndex(model()->index(0,0, index));
  }
  selectionModel()->clearSelection();
  emit signal_open(index);
}

void ItemViewBase::openInBrowser(const QModelIndex &index)
{
  selectionModel()->clearSelection();
  emit signal_openInBrowser(index);
}

void ItemViewBase::updateScrollAnimation(qreal value)
{
  Q_UNUSED(value);
  verticalScrollBar()->setValue(d->scrollTimeLine->currentFrame());
}

void ItemViewBase::paintEvent(QPaintEvent *event)
{
  relayout();//TODO put that in place where it is not called that often but often enough

  QPainter painter(viewport());
  painter.setRenderHints(QPainter::Antialiasing);
  d->animator->paint(&painter, event);

}


void ItemViewBase::relayout()
{
  if(viewMode() == ItemViewBase::IconMode) {
    d->gridSize.setWidth(qMax(iconSize().width() + 10, fontMetrics().width("wwwwwwwwwww")));
    d->gridSize.setHeight(iconSize().height() + 10 + fontMetrics().height()*2);
  } else { // ItemViewBase::ListMode
    d->gridSize.setWidth(viewport()->width());
    d->gridSize.setHeight(qMax(iconSize().height(), fontMetrics().height()));
  }
  d->itemsPerRow = viewport()->width() / d->gridSize.width();
  if(d->itemsPerRow == 0) d->itemsPerRow = 1; //force one column

  d->updateScrollBarRange();
}

void ItemViewBase::resizeEvent(QResizeEvent *event)
{
  viewport()->update();
  QAbstractItemView::resizeEvent(event);
}

void ItemViewBase::mouseMoveEvent(QMouseEvent *event)
{
  
  bool mouseOverBackArrow = d->animator->backArrowRect().contains(event->pos());

  if (mouseOverBackArrow != d->backArrowHover) {
    d->backArrowHover = mouseOverBackArrow;
    setDirtyRegion(d->animator->backArrowRect());
  }
  QModelIndex index = indexAt(event->pos());
  d->hoveredIndex = index;
  update(index);
  QAbstractItemView::mouseMoveEvent(event);
}

void ItemViewBase::mousePressEvent(QMouseEvent *event)
{
  QAbstractItemView::mousePressEvent(event);
}

void ItemViewBase::mouseReleaseEvent(QMouseEvent *event)
{  
  if(d->animator->backArrowRect().contains(event->pos()) && rootIndex() != QModelIndex()) {
    if (event->button() == Qt::LeftButton) {
      open(rootIndex().parent());
    }
    else if (event->button() == Qt::MidButton) {
      openInBrowser(rootIndex().parent());
    }
  }
  else if (indexAt(event->pos()).isValid() && event->button() == Qt::MidButton) {
    openInBrowser(indexAt(event->pos()));
  }
  else {
    QAbstractItemView::mouseReleaseEvent(event);
  }
}

void ItemViewBase::wheelEvent(QWheelEvent *event)
{
  if(event->orientation() != Qt::Vertical) {
    QAbstractItemView::wheelEvent(event);
    return;
  }
    
  int degrees = event->delta() / 8;
  int steps = degrees / 15;
  steps *= d->wheelScrollLines;
  
  d->scrollBarValue -= steps * verticalScrollBar()->singleStep();
  if(d->scrollBarValue < verticalScrollBar()->minimum()) d->scrollBarValue = verticalScrollBar()->minimum();
  if(d->scrollBarValue > verticalScrollBar()->maximum()) d->scrollBarValue = verticalScrollBar()->maximum();
  
  if(d->scrollTimeLine->state() == QTimeLine::Running) {
    d->scrollTimeLine->stop();
  }
  d->scrollTimeLine->setFrameRange(verticalScrollBar()->value(), d->scrollBarValue);
  d->scrollTimeLine->start();
}

void ItemViewBase::keyPressEvent(QKeyEvent *event)
{
  if(event->key() == Qt::Key_Backspace) {
    if(rootIndex().isValid()) {
      open(model()->parent(rootIndex()));
    }
  } else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    open(currentIndex());
  } else { 
    QAbstractItemView::keyPressEvent(event);
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////PRIVATE///////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ItemViewBase::Private::Private(ItemViewBase *view)
  :q(view)
  ,animator(0)
  ,backArrowHover(false)
  ,itemsPerRow(1)
  ,goBack(false)
  ,scrollTimeLine(new QTimeLine())
  ,scrollBarValue(0)


{
  scrollTimeLine->setDuration(100);
}

ItemViewBase::Private::~Private()
{
  delete scrollTimeLine;
}

void ItemViewBase::Private::updateScrollBarRange()
{
  int childCount = q->model()->rowCount(q->rootIndex());
  int pageSize = q->viewport()->height();
  
  int rowHeight = gridSize.height();
  int rows = qCeil((qreal)childCount / (qreal)itemsPerRow);
  
  q->verticalScrollBar()->setRange(0, (rowHeight * rows) - pageSize);
  q->verticalScrollBar()->setPageStep(pageSize);
  q->verticalScrollBar()->setSingleStep(rowHeight);
}

QPainterPath ItemViewBase::Private::trianglePath(qreal width, qreal height) const
{
  QPainterPath path(QPointF(-width/2,0.0));
  path.lineTo(width,-height/2);
  path.lineTo(width,height/2);
  path.lineTo(-width/2,0.0);

  return path;
}

#include "itemviewbase.moc"

