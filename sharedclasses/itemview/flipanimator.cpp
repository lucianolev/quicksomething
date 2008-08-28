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
  
  paintItems(&painter, view()->previousRootIndex(), view()->viewport()->rect());
  painter.end();
  
  //newroot
  m_newRoot.fill(Qt::transparent);
  painter.begin(&m_newRoot);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

  paintItems(&painter, view()->rootIndex(), view()->viewport()->rect());
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
    paintItems(painter, view()->rootIndex(), event->rect());
  } 
  
  
  painter->restore();  
}
