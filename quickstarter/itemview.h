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

#ifndef ItemView_HEADER
#define ItemView_HEADER

#include "itemview/itemviewbase.h"

class ItemView : public ItemViewBase
{
  Q_OBJECT
  public:
    ItemView(QWidget *parent = 0);
    ~ItemView();
    
    void setShowToolTips(bool show);
    bool showToolTips();
    
  protected:
    //QAbstractItemView
    bool viewportEvent(QEvent *event);
    //void dragEnterEvent(QDragEnterEvent *event);
    //void dropEvent(QDropEvent *event);
    //void dragMoveEvent(QDragMoveEvent *event);
    
    //QAbstractScrollArea
    //void contextMenuEvent( QContextMenuEvent *event);
    
  private:
    bool m_showToolTips;
};

#endif //ItemView_HEADER
