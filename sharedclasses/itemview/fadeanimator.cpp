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

#include "fadeanimator.h"

#include <KDebug>
FadeAnimator::FadeAnimator(ItemViewBase *view, QObject *parent)
  :Animator(view, parent)
{
  connect(animationTimeLine(), SIGNAL(finished()), this, SLOT(timeLineFinished()));
  animationTimeLine()->setDuration(100);
  animationTimeLine()->setDirection(QTimeLine::Backward);
  animationTimeLine()->setCurrentTime(100);
}

void FadeAnimator::timeLineFinished()
{
  animationTimeLine()->toggleDirection();
  if(animationTimeLine()->direction() ==  QTimeLine::Forward) {
    animationTimeLine()->start();
  }
}

void FadeAnimator::paint(QPainter *painter, QPaintEvent *event)
{
  painter->save();
  painter->setOpacity(painter->opacity() * animationTimeLine()->currentValue());
  QModelIndex root = view()->rootIndex();
  if(animationTimeLine()->state() == QTimeLine::Running && animationTimeLine()->direction() == QTimeLine::Backward) {
    root = view()->previousRootIndex();
  }
  
  paintItems(painter, root, event->rect());
  painter->restore();
  
}
