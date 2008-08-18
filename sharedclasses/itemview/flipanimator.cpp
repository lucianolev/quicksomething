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

#include "flipanimator.h"

#include <KDebug>

FlipAnimator::FlipAnimator(ItemViewBase *view, QObject *parent)
  :Animator(view, parent)
{
  animationTimeLine()->setDuration(200);
}

void FlipAnimator::animate()
{
  if(m_oldRoot.size() != view()->viewport()->size()){
    m_oldRoot = QPixmap(view()->viewport()->size());
    m_newRoot = QPixmap(view()->viewport()->size());
  }
  QPainter painter;
  
  //oldroot
  m_oldRoot.fill(Qt::transparent);
  painter.begin(&m_oldRoot);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
  
  QModelIndex root = view()->previousRootIndex();
  int rows = view()->model()->rowCount(root);
  if(root != QModelIndex()) {
    QStyle::State state = 0;
    if(backArrowRect().contains(view()->viewport()->mapFromGlobal(QCursor::pos()))) {
      state |= QStyle::State_MouseOver;
    }
    drawBackarrow(&painter, state);
  }

  for (int i = 0; i < rows; ++i) {
    QModelIndex index = view()->model()->index(i, 0, root);
    
    if (view()->isItemVisible(index)) {
      QStyleOptionViewItemV4 option = viewOptions(index);
      view()->itemDelegate(index)->paint(&painter,option,index); 
    }
  }
  painter.end();
  
  //newroot
  m_newRoot.fill(Qt::transparent);
  painter.begin(&m_newRoot);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
  
  root = view()->rootIndex();
  rows = view()->model()->rowCount(root);
  if(root != QModelIndex()) {
    QStyle::State state = 0;
    if(backArrowRect().contains(view()->viewport()->mapFromGlobal(QCursor::pos()))) {
      state |= QStyle::State_MouseOver;
    }
    drawBackarrow(&painter, state);
  }

  for (int i = 0; i < rows; ++i) {
    QModelIndex index = view()->model()->index(i, 0, root);
    
    if (view()->isItemVisible(index)) {
      QStyleOptionViewItemV4 option = viewOptions(index);
      view()->itemDelegate(index)->paint(&painter,option,index); 
    }
  }
  painter.end();
  Animator::animate();
  
}

void FlipAnimator::paint(QPainter *painter, QPaintEvent *event)
{
  painter->save();
  
  if(animationTimeLine()->state() == QTimeLine::Running) {
    qreal value = animationTimeLine()->currentValue();
    bool forward = true;
    if(view()->previousRootIndex().parent() == view()->rootIndex())
      forward = false;
    
    if(forward) {
      QRectF target(0.0, 0.0, m_oldRoot.width() * (1.0 - value), m_oldRoot.height());
      QRectF source(m_oldRoot.width() * value, 0.0, m_oldRoot.width() * (1.0 - value), m_oldRoot.height());
      painter->drawPixmap(target, m_oldRoot, source);
      
      QRectF target2(m_newRoot.width() * (1 - value), 0.0, m_newRoot.width() * value, m_newRoot.height());
      QRectF source2(0.0, 0.0, m_newRoot.width() * value, m_newRoot.height());
      painter->drawPixmap(target2, m_newRoot, source2);
    } else {
      QRectF target(m_oldRoot.width() * value, 0.0, m_oldRoot.width() * (1.0 - value), m_oldRoot.height());
      QRectF source(0.0, 0.0, m_oldRoot.width() * (1.0 - value), m_oldRoot.height());
      painter->drawPixmap(target, m_oldRoot, source);
      
      QRectF target2(0.0, 0.0, m_newRoot.width() * value, m_newRoot.height());
      QRectF source2(m_newRoot.width() * (1 - value), 0.0, m_newRoot.width() * value, m_newRoot.height());
      painter->drawPixmap(target2, m_newRoot, source2);
    }
  } else {
    QModelIndex root = view()->rootIndex();
    const int rows = view()->model()->rowCount(root);
    if(root != QModelIndex()) {
      QStyle::State state = 0;
      if(backArrowRect().contains(view()->viewport()->mapFromGlobal(QCursor::pos()))) {
	state |= QStyle::State_MouseOver;
      }
      drawBackarrow(painter, state);
    }

    for (int i = 0; i < rows; ++i) {
      QModelIndex index = view()->model()->index(i, 0, root);
    
      if (!event->rect().intersects(view()->visualRect(index))) {
	continue;
      }
      QStyleOptionViewItemV4 option = viewOptions(index);
      view()->itemDelegate(index)->paint(painter,option,index); 
    }
  } 
  
  
  painter->restore();
  
//   painter->setOpacity(painter->opacity() * animationTimeLine()->currentValue());
//   QModelIndex root = view()->rootIndex();
//   if(animationTimeLine()->state() == QTimeLine::Running && animationTimeLine()->direction() == QTimeLine::Backward) {
//     root = view()->previousRootIndex();
//   }
//   
//   if(root != QModelIndex()){
//     QStyle::State state = 0;
//     if(backArrowRect().contains(view()->viewport()->mapFromGlobal(QCursor::pos()))) {
//       state |= QStyle::State_MouseOver;
//     }
//     drawBackarrow(painter, state);
//   }
//     
//   const int rows = view()->model()->rowCount(root);
// 
//   for (int i = 0; i < rows; ++i) {
//     QModelIndex index = view()->model()->index(i, 0, root);
//     
//     // only draw items intersecting the region of the widget
//     // being updated
//     if (!event->rect().intersects(view()->visualRect(index))) {
//        continue;
//     }
// 
//     QStyleOptionViewItemV4 option = viewOptions(index);
//     view()->itemDelegate(index)->paint(painter,option,index);
//   }
//   painter->restore();
  
}