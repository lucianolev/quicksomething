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


#ifndef Animator_HEADER
#define Animator_HEADER

#include <QPainter>
#include <QPaintEvent>
#include <QTimeLine>

#include "itemviewbase.h"

class AnimatorPrivate;

class Animator : public QObject
{
  Q_OBJECT
  public:
    enum Direction{
      NoDirection = 0,
      Forward = 1,
      Backward = 2
    };
    
    Animator(ItemViewBase *view, QObject *parent =  0);
    ~Animator();
    
    virtual void animate();
    virtual void paint(QPainter *painter, QPaintEvent *event);
    virtual QRect backArrowRect() const;
    virtual int backArrowSpacing() const;
    
  protected:
    virtual void drawBackarrow(QPainter *painter, QStyle::State state);
    virtual QStyleOptionViewItem viewOptions(const QModelIndex &index) const;
    //virtual void paintItems(QPainter *painter)
    QTimeLine *animationTimeLine();
    ItemViewBase *view() const;
  
  private slots:
    void timeLineValueChanged(qreal value);
    
  private:
    AnimatorPrivate * const d;
};

#endif //Animator_HEADER
