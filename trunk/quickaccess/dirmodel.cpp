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

//KDE includes
//#include <KDebug>
#include <KDesktopFile>

//locale includes
#include "dirmodel.h"


DirModel::DirModel(QObject * parent )
  :KDirModel( parent )
{

}

QVariant DirModel::data( const QModelIndex & index, int role  ) const
{

  if (role != Qt::DisplayRole) {
    return KDirModel::data(index, role);
  }
  
  KFileItem item = itemForIndex(index);
  if(item.isDesktopFile()) {
    KDesktopFile f(item.url().path());
    QString name = f.readName();
    if(!name.isEmpty()){
      return name;
    }
  }
  return item.text();
}

#include "dirmodel.moc"
