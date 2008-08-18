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

#ifndef AnimatorPrivate_HEADER
#define AnimatorPrivate_HEADER

#include <QTimeLine>

static const int BACK_ARROW_WIDTH = 16;
static const int BACK_ARROW_SPACING = 5;

class AnimatorPrivate
{
  public:
    AnimatorPrivate(ItemViewBase *view);
    ~AnimatorPrivate();
    
    ItemViewBase *q;
    QTimeLine *animationTimeLine;
    
  
};

#endif //AnimatorPrivate_HEADER
