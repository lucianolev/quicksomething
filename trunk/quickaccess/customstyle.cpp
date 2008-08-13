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

#include "customstyle.h"

#include <QPainter>
#include <QStyleOptionSlider>

#include <KDebug>


#include <Plasma/Theme>
#include <plasma/paintutils.h>

void CustomStyle::drawComplexControl(ComplexControl cc
				    ,const QStyleOptionComplex *opt
				    ,QPainter *p
				    ,const QWidget *widget)const
{
  
//   if (cc != CC_ScrollBar) {
    /*QCommonStyle*/KStyle::drawComplexControl(cc, opt, p, widget);
    return;
//   }
  
  const QStyleOptionSlider *option = qstyleoption_cast<const QStyleOptionSlider *>(opt);
  p->save();
  p->setRenderHint(QPainter::Antialiasing);
    
  const QRect subLine = subControlRect(cc, option, SC_ScrollBarSubLine, widget);
  const QRect addLine = subControlRect(cc, option, SC_ScrollBarAddLine, widget);
  
  
    
//    painter->setCompositionMode(QPainter::CompositionMode_Clear);

//    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    //painter->fillRect(subLine, Qt::green);
//     painter->fillRect(addLine, Qt::green);
    //painter->fillRect(slider, Qt::blue);
    
//   if(opt->subControls & SC_ScrollBarGroove) {
//     const QRect groove = subControlRect(control, option, SC_ScrollBarGroove, widget);
//     painter->setCompositionMode(QPainter::CompositionMode_Source);
//     painter->fillRect(groove, Qt::transparent);
//     painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
//   }
  
  if(option->subControls & SC_ScrollBarSlider) {
    const QRect slider  = subControlRect(cc, option, SC_ScrollBarSlider, widget).adjusted(1, 0, -1, 0);
    QLinearGradient gradient(slider.topLeft(), slider.bottomLeft());
    gradient.setColorAt(0, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    gradient.setColorAt(0.6, Qt::transparent);
    p->setBrush(gradient);
  
    p->setPen(QPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor), 0));
  
    p->setClipRect(slider);
    p->drawPath(Plasma::PaintUtils::roundedRectangle(slider, 5));
  }
//     QPainterPath path;
//     path.arcMoveTo(subLine, 0);
//     path.arcTo(subLine, 0, 180);
//     path.lineTo(addLine.topLeft() + QPoint(0, addLine.height() / 2));
//     path.arcTo(addLine, 180, 180);
//     path.closeSubpath();
// 
//     QPolygon upArrow, downArrow;
//     upArrow.setPoints(3, 0,5, 6,5, 3,1);
//     downArrow.setPoints(3, 0,1, 6,1, 3,5);
// 
//     upArrow.translate(subLine.x() + (subLine.width() - 6) / 2, subLine.y() + (subLine.height() - 6) / 2);
//     downArrow.translate(addLine.x() + (addLine.width() - 6) / 2, addLine.y() + (addLine.height() - 6) / 2);
// 
//     QColor color(255, 255, 255, 48);
// 
//     //painter->fillPath(path, QColor(0, 0, 0, 24));
// 
//     painter->setBrush(color);
//     painter->setPen(Qt::NoPen);
//     //painter->fillRect(slider, color);
//     qreal radius = slider.width() / 2.0;
//     painter->fillRect(groove, Qt::black);
//     painter->drawRoundedRect(slider, radius, radius);
// 
//     painter->drawPolygon(upArrow);
//     painter->drawPolygon(downArrow);

//     painter->restore();
}
