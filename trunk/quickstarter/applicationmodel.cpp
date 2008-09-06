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

#include "applicationmodel.h"

#include <KServiceGroup>
#include <KDebug>
#include <KIcon>

class Item
{
  public:
    Item() :isCategory(false) ,childrenFetched(false) ,parent(0){};
    ~Item(){ qDeleteAll(children); };
    
    bool isCategory;
    QString name;
    QIcon icon;
    QString relPath;
    QString comment;
    bool childrenFetched;
    Item *parent;
    QString desktopFile;
    
    QList<Item*> children;
};

ApplicationModel::ApplicationModel(QObject *parent, bool categoriesonly)
  :QAbstractItemModel(parent)
  ,m_root(new Item())
{
  m_categoriesonly = categoriesonly;
  setRoot(QString()); //This is to show all categories
}

ApplicationModel::~ApplicationModel()
{
  delete m_root;
}

void ApplicationModel::setRoot(const QString &path)
{
  delete m_root;
  m_root = new Item();
  m_root->relPath = path;
  m_root->isCategory = true;
  getChildren(m_root);
}

QString ApplicationModel::relPath(const QModelIndex &index)
{
  if(!index.isValid())
    return QString();
  else 
    return static_cast<Item*>(index.internalPointer())->relPath;
}

QModelIndex ApplicationModel::index(int row, int column, const QModelIndex &parent) const
{
  if(column != 0 || row < 0) return QModelIndex();
  
  Item *parentItem = m_root;
  if(parent.isValid()) {
    parentItem = static_cast<Item*>(parent.internalPointer());
  }
  
  if(row < parentItem->children.count()) {
    return createIndex(row, 0, parentItem->children.at(row));
  } else {
    return QModelIndex();
  }
  
}

QModelIndex ApplicationModel::parent(const QModelIndex &index) const
{
  if(!index.isValid()) return QModelIndex();
  
  Item *item = static_cast<Item*>(index.internalPointer());
  if(item->parent->parent) {
    int row = item->parent->parent->children.indexOf(item->parent);
    return createIndex(row, 0, item->parent);
  } else {
    return QModelIndex();
  }
}

int ApplicationModel::rowCount(const QModelIndex &parent) const
{
  if(!parent.isValid()) return m_root->children.count();
  
  Item *item = static_cast<Item*>(parent.internalPointer());
  return item->children.count();
}

int ApplicationModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}

QVariant ApplicationModel::data(const QModelIndex &index, int role) const
{
  if(!index.isValid()) return QVariant();
  
  Item *item = static_cast<Item*>(index.internalPointer());
  switch(role){
    case Qt::DisplayRole:
      return item->name;
    case Qt::DecorationRole:
      return item->icon;
    case Qt::ToolTipRole:
      return item->comment;
    default:
      return QVariant();
  }
  
}

bool ApplicationModel::canFetchMore(const QModelIndex &parent) const
{
  Item *item = m_root;
  if(parent.isValid()) {  
    item = static_cast<Item*>(parent.internalPointer());
  }
  
  if(item->isCategory && !item->childrenFetched) {
    return true;
  } else {
    return false;
  }
  
}

void ApplicationModel::fetchMore(const QModelIndex &parent)
{
  if(!parent.isValid()) return;
  
  Item *parentItem = static_cast<Item*>(parent.internalPointer());
  emit layoutAboutToBeChanged();
  getChildren(parentItem);
  emit layoutChanged();
}

bool ApplicationModel::hasChildren(const QModelIndex &parent) const
{
  Item *item = m_root;
  if(parent.isValid()) {
    item = static_cast<Item*>(parent.internalPointer());
  }
  
  if(item->isCategory) {
    return true;
  }
  return false;
}
    
void ApplicationModel::getChildren(Item *parent)
{
  KServiceGroup::Ptr group = KServiceGroup::group(parent->relPath);
  if (!group || !group->isValid()) return;

  KServiceGroup::List list = group->entries();
  // Iterate over all entries in the group
  for( KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++) {
    KSycocaEntry::Ptr p = (*it);
    
    QString name;
    QIcon icon;
    QString relPath;
    bool isCategory = false;
    QString comment;
    QString desktopFile;
    
    if (p->isType(KST_KService) && !m_categoriesonly){
       KService::Ptr s = KService::Ptr::staticCast(p);
       if(s->noDisplay()) continue;
       name = s->name();
       icon = KIcon(s->icon());
       relPath = parent->relPath;
       comment  = s->comment();
       desktopFile = s->entryPath();
    }
    else if (p->isType(KST_KServiceGroup))
    {
       KServiceGroup::Ptr g = KServiceGroup::Ptr::staticCast(p);
       if(g->noDisplay() || g->childCount() == 0) continue;
       name = g->caption();
       icon = KIcon(g->icon());
       relPath = g->relPath();
       comment = g->comment();
       isCategory = true;
    }
    if (!name.isNull())
    {
      Item *newItem = new Item();
      newItem->parent = parent;
      newItem->name = name;
      newItem->icon = icon;
      newItem->relPath = relPath;
      newItem->isCategory = isCategory;
      newItem->comment = comment;
      newItem->desktopFile = desktopFile;
      parent->children.append(newItem);
    }

   }

  parent->childrenFetched = true;
}