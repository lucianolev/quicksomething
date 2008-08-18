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

#ifndef FlipAnimator_HEADER
#define FlipAnimator_HEADER

#include "animator.h"

class FlipAnimator : public Animator
{
  Q_OBJECT
  public:
    FlipAnimator(ItemViewBase *view, QObject *parent = 0);
    void paint(QPainter *painter, QPaintEvent *event);
    void animate();
    
  private:
    QPixmap m_oldRoot;
    QPixmap m_newRoot;

};

#endif //FlipAnimator_HEADER