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

#include "animator.h"
#include "animator_p.h"

#include <KDebug> //TODO remove

#include <Plasma/Theme>
#include <plasma/paintutils.h>

#include "itemviewbase.h"

Animator::Animator(ItemViewBase *view, QObject *parent)
  :QObject(parent)
  ,d(new AnimatorPrivate(view))
{
  connect(d->animationTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(timeLineValueChanged(qreal)));
}

Animator::~Animator()
{
  delete d;
}

void Animator::animate()
{
  d->animationTimeLine->start();
}

ItemViewBase *Animator::view() const
{
  return d->q;
}

QTimeLine *Animator::animationTimeLine()
{
  return d->animationTimeLine;
}

void Animator::drawBackarrow(QPainter *painter, QStyle::State state)
{
  painter->save();
  painter->setOpacity(painter->opacity() * 0.5);


  QRect rect = backArrowRect();
  

  
  QLinearGradient gradient(rect.right(), rect.center().y(),
			   rect.left(), rect.center().y());
  QColor color = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
  if (state & QStyle::State_MouseOver) {
    gradient.setColorAt(0, view()->palette().color(QPalette::Highlight));
  } else {
    gradient.setColorAt(0, color);
  }
  gradient.setColorAt(1, QColor(255,255,255, 0));
  QPainterPath path = Plasma::PaintUtils::roundedRectangle(rect, 4);
  painter->setBrush(gradient);
  painter->setPen(QPen(color, 0));
  painter->drawPath(path);

  painter->setPen(Qt::NoPen);
  if (state & QStyle::State_MouseOver) {
    painter->setBrush(view()->palette().highlightedText());
  } else {
    painter->setBrush(view()->palette().light());
  }
  painter->translate(rect.center());
  
  QPainterPath trianglePath(QPointF(-5/2,0.0));
  trianglePath.lineTo(5,-10/2);
  trianglePath.lineTo(5,10/2);
  trianglePath.lineTo(-5/2,0.0);
    
  painter->drawPath(trianglePath);
  painter->resetTransform();
  

  painter->restore();
  
}

QRect Animator::backArrowRect() const
{
  return QRect(0, 0, BACK_ARROW_WIDTH, view()->viewport()->height());
}

int Animator::backArrowSpacing() const
{
  return BACK_ARROW_SPACING;
}

void Animator::paintItems(QPainter *painter, const QModelIndex &root, QRect rect)
{
  painter->save();
  const int rows = view()->model()->rowCount(root);
  
  if(root != QModelIndex()){
    QStyle::State state = 0;
    if(backArrowRect().contains(view()->viewport()->mapFromGlobal(QCursor::pos()))) {
      state |= QStyle::State_MouseOver;
    }
    drawBackarrow(painter, state);
  }

  for (int i = 0; i < rows; ++i) {
    QModelIndex index = view()->model()->index(i, 0, root);
    
    // only draw items intersecting the region of the widget
    // being updated
    if (!rect.intersects(view()->visualRect(index))) {
       continue;
    }

    QStyleOptionViewItemV4 option = viewOptions(index);
    view()->itemDelegate(index)->paint(painter,option,index);
  }
  painter->restore();
}

void Animator::paint(QPainter *painter, QPaintEvent *event)
{
  paintItems(painter, view()->rootIndex(), event->rect());
}

QStyleOptionViewItem Animator::viewOptions(const QModelIndex &index) const
{
  QStyleOptionViewItemV4 option;
  option.initFrom(view());
  option.state &= ~QStyle::State_MouseOver;
  option.font = view()->font();
  if (!view()->hasFocus())
    option.state &= ~QStyle::State_Active;

  option.state &= ~QStyle::State_HasFocus;
  if (view()->iconSize().isValid()) {
    option.decorationSize = view()->iconSize();
  } else {
    int pm = view()->style()->pixelMetric(QStyle::PM_SmallIconSize, 0, view());
    option.decorationSize = QSize(pm, pm);
  }
  option.decorationAlignment = Qt::AlignCenter;
  option.textElideMode = view()->textElideMode();
  option.showDecorationSelected = view()->style()->styleHint(QStyle::SH_ItemView_ShowDecorationSelected, 0, view());
  
  if(view()->viewMode() == ItemViewBase::ListMode) {
    option.displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option.decorationPosition  = QStyleOptionViewItem::Left;
  } else {
    option.features = QStyleOptionViewItemV2::WrapText;
    option.displayAlignment = Qt::AlignCenter;
    option.decorationPosition  = QStyleOptionViewItem::Top;
  }
  option.rect = view()->visualRect(index);
  if (view()->selectionModel()->isSelected(index)) {
    option.state |= QStyle::State_Selected;
  }
  if (index == view()->currentIndex()) {
    option.state |= QStyle::State_HasFocus;
  }
  
  if (index == view()->hoveredIndex()) {
    option.state |= QStyle::State_MouseOver;
  }
  
  option.viewItemPosition = QStyleOptionViewItemV4::OnlyOne;
  option.index = index;
  return option;
}

void Animator::timeLineValueChanged(qreal value)
{
  Q_UNUSED(value);
  view()->viewport()->update();
}

AnimatorPrivate::AnimatorPrivate(ItemViewBase *view)
  :q(view)
  ,animationTimeLine(new QTimeLine())
{
  
}

AnimatorPrivate::~AnimatorPrivate()
{
  delete animationTimeLine;
}


