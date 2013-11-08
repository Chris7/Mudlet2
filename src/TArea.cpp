/***************************************************************************
 *   Copyright (C) 2008 by Heiko Koehn (KoehnHeiko@googlemail.com)         *
 *                                                                         *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/




#include "TArea.h"
#include "TMap.h"
#include "TRoom.h"
#include <stdlib.h>
#include <QDebug>
#include <QPair>
#include <QList>

//#define NORTH       12
//#define NORTHEAST    1
//#define EAST         3
//#define SOUTHEAST    4
//#define SOUTH        6
//#define SOUTHWEST    7
//#define WEST         9
//#define NORTHWEST   10
//#define UP          13
//#define DOWN        14
//#define IN          15
//#define OUT         16
//#define OTHER       17
// The above DO NOT match the DIR_ defines in TRoom.h, but as the data gets
// stored in the map file they cannot be redefined without a map file change
// So for consistancy as of map version 17 we will use the (supplimented by the
// addition of a code for OTHER) DIR_**** values set in the top of TRoom.h.
// Fortunately this data does not seem to be used at present!
// FIXME: Modify mapper "painter" code to use "exits" rather than deriving the
// same information each time it is run ???

TArea::TArea(TMap * map , TRoomDB * pRDB )
: min_x(0)
, min_y(0)
, min_z(0)
, max_x(0)
, max_y(0)
, max_z(0)
, gridMode( false )
, isZone( false )
, zoneAreaRef( 0 )
, mpRoomDB( pRDB )
{
}

TArea::~TArea()
{
    if( mpRoomDB )
        mpRoomDB->removeArea( (TArea*)this );
    else
        qDebug()<<"ERROR: In TArea::~TArea(), instance has no mpRoomDB";
}

int TArea::getAreaID()
{
    if( mpRoomDB )
        return mpRoomDB->getAreaID( this );
    else
    {
        qDebug()<<"ERROR: TArea::getAreaID() instance has no mpRoomDB, returning -1 as ID";
        return -1;
    }
}

QMap<int,QMap<int,QMultiMap<int,int> > > TArea::koordinatenSystem()
{
    QMap<int,QMap<int,QMultiMap<int,int> > > kS;
    QList<TRoom*> roomList = mpRoomDB->getRoomPtrList();
    for( int i=0; i<roomList.size(); i++ )
    {
        TRoom * pR = roomList[i];
        int id = pR->getId();
        int x = pR->x;
        int y = pR->y;
        int z = pR->z;
        QMap<int,QMultiMap<int,int> > _y;
        QMultiMap<int,int> _z;
        if( ! kS.contains( x ) )
        {
            kS[x] = _y;
        }
        if( ! kS[x].contains( y ) )
        {
            kS[x][y] = _z;
        }
        kS[x][y].insertMulti( z, id );
    }
    //qDebug()<< "kS="<<kS;
    return kS;
}

QList<int> TArea::getRoomsByPosition( int x, int y, int z )
{
    QList<int> dL;
    for( int i=0; i<rooms.size(); i++ )
    {
        TRoom * pR = mpRoomDB->getRoom(rooms[i]);
        if( pR )
        {
            int id = pR->getId();
            int _x = pR->x;
            int _y = pR->y;
            int _z = pR->z;
            if( _x == x && _y == y && _z == z )
            {
                dL.push_back( id );
            }
        }
    }
    return dL;
}

QList<int> TArea::getCollisionNodes()
{
    QList<int> problems;
    QMap<int,QMap<int,QMultiMap<int,int> > > kS = koordinatenSystem();
    QMapIterator<int,QMap<int,QMultiMap<int,int> > > it(kS);
    while (it.hasNext())
    {
        it.next();
        QMap<int,QMultiMap<int,int> > x_val = it.value();
        QMapIterator<int,QMultiMap<int,int> > it2(x_val);
        while (it2.hasNext())
        {
            it2.next();
            QMultiMap<int,int> y_val = it2.value();
            QMapIterator<int,int> it3(y_val);
            QList<int> z_coordinates;
            while (it3.hasNext())
            {
                it3.next();
                int z = it3.key();
                int node = it3.value();

                if( ! z_coordinates.contains( node ) )
                    z_coordinates.append( node );
                else
                {
                    if( ! problems.contains( node ) )
                    {
                        QMultiMap<int, int>::iterator it4 = y_val.find(z);
                        problems.append( it4.value() );
                        //qDebug()<<"problem node="<<node;
                    }
                }
            }
        }
    }
    return problems;
}

void TArea::fast_ausgaengeBestimmen( int id )
{
    if( ! mpRoomDB )
        return;

    TRoom * pR = mpRoomDB->getRoom(id);
    //makeMap lua test code caused seg fault here, without the following
    //check for a valid mpRoomDB?

    if( ! pR )
        return;

    exits.remove(id);

    int exitId = pR->getNorth();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>(exitId, DIR_NORTH);
        exits.insertMulti( id, p );
    }
    exitId = pR->getNortheast();
    if( exitId > 0 && rooms.indexOf( exitId )  < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_NORTHEAST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getEast();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_EAST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getSoutheast();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_SOUTHEAST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getSouth();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_SOUTH);
        exits.insertMulti( id, p );
    }
    exitId = pR->getSouthwest();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_SOUTHWEST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getWest();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_WEST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getNorthwest();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_NORTHWEST);
        exits.insertMulti( id, p );
    }
    exitId = pR->getUp();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_UP);
        exits.insertMulti( id, p );
    }
    exitId = pR->getDown();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_DOWN);
        exits.insertMulti( id, p );
    }
    exitId = pR->getIn();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_IN);
        exits.insertMulti( id, p );
    }
    exitId = pR->getOut();
    if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
    {
        QPair<int, int> p = QPair<int,int>( exitId, DIR_OUT);
        exits.insertMulti( id, p );
    }
    const QMap<int, QString> otherMap = pR->getOtherMap();
    QMapIterator<int,QString> it( otherMap );
    while( it.hasNext() )
    {
        it.next();
        int _exit = it.key();
        TRoom * pO = mpRoomDB->getRoom(_exit);
        if( pO )
        {
            if( pO->getArea() != getAreaID() )
            {
                QPair<int, int> p = QPair<int,int>(pO->getId(), DIR_OTHER);
                exits.insertMulti( id, p );
            }
        }
    }
}

void TArea::ausgaengeBestimmen()
{
    exits.clear();
    for( int i=0; i<rooms.size(); i++ )
    {
        TRoom * pR = mpRoomDB->getRoom(rooms[i]);
        if( ! pR )
            continue;

        int id = pR->getId();
        int exitId = pR->getNorth();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            // Uses existance of valid exit that DOES NOT appear in the list of
            // rooms for THIS area to determine an out of area exit.
            QPair<int, int> p = QPair<int,int>(exitId, DIR_NORTH);
            // Was using id (in area room) as key ^^^ when it should be the out of
            // area room according to notation in the header file?
            exits.insertMulti( id, p );
        }
        exitId = pR->getNortheast();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_NORTHEAST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getEast();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_EAST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getSoutheast();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_SOUTHEAST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getSouth();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_SOUTH);
            exits.insertMulti( id, p );
        }
        exitId = pR->getSouthwest();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_SOUTHWEST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getWest();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_WEST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getNorthwest();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_NORTHWEST);
            exits.insertMulti( id, p );
        }
        exitId = pR->getUp();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_UP);
            exits.insertMulti( id, p );
        }
        exitId = pR->getDown();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_DOWN);
            exits.insertMulti( id, p );
        }
        exitId = pR->getIn();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_IN);
            exits.insertMulti( id, p );
        }
        exitId = pR->getOut();
        if( exitId > 0 && rooms.indexOf( exitId ) < 0 )
        {
            QPair<int, int> p = QPair<int,int>(exitId, DIR_OUT);
            exits.insertMulti( id, p );
        }
        const QMap<int, QString> otherMap = pR->getOtherMap();
        QMapIterator<int,QString> it( otherMap );
        while( it.hasNext() )
        {
            it.next();
            int _exit = it.key();
            TRoom * pO = mpRoomDB->getRoom(_exit);
            if( pO )
            {
                if( pO->getArea() != getAreaID() )
                {
                    QPair<int, int> p = QPair<int,int>(pO->getId(), DIR_OTHER);
                    exits.insertMulti( id, p ); // was using pO->getId() = id of other room NOT this room as key.
                }
            }
        }
    }
    //qDebug()<<"exits:"<<exits.size();
}

void TArea::fast_calcSpan( int id )
{

    TRoom * pR = mpRoomDB->getRoom(id);
    if( !pR ) return;

    int x = pR->x;
    int y = pR->y;
    int z = pR->z;
    if( x > max_x ) max_x = x;
    if( x < min_x ) min_x = x;
    if( y > max_y ) max_y = y;
    if( y < min_y ) min_y = y;
    if( z > max_z ) max_z = z;
    if( z < min_z ) min_z = z;
}

void TArea::addRoom( int id )
{
    TRoom * pR = mpRoomDB->getRoom( id );
    if( pR )
    {
        if( !rooms.contains( id ) )
        {
            rooms.append( id );
        }
        else
        {
            qDebug()<<"TArea::addRoom("<<id<<") No creation! room already exists";
        }
    }
    else
    {
        QString error = QString("roomID=%1 does not exist, can't set properties of non-existent rooms").arg(id);
    }
}

void TArea::calcSpan()
{
    if( rooms.size() > 0 )
    {
        int id = rooms[0];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) return;
        min_x = pR->x;
        max_x = min_x;
        min_y = pR->y*-1;
        max_y = min_y;
        min_z = pR->z;
        max_z = min_z;
    }

    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->x;
        if( _m < min_x )
        {
            min_x = _m;
        }
    }
    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->y*-1;
        if( _m < min_y )
        {
            min_y = _m;
        }
    }
    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->z;
        if( _m < min_z )
        {
            min_z = _m;
            if( ! ebenen.contains( _m ) )
            {
                ebenen.push_back( _m );
            }
        }
    }
    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->x;
        if( _m > max_x )
        {
            max_x = _m;
        }
    }
    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->y*-1;
        if( _m > max_y )
        {
            max_y = _m;
        }
    }
    for( int i=0; i<rooms.size(); i++ )
    {
        int id = rooms[i];
        TRoom * pR = mpRoomDB->getRoom( id );
        if( !pR ) continue;
        int _m = pR->z;
        if( _m > max_z )
        {
            max_z = _m;
        }
        // ebenenliste anlegen
        if( ! ebenen.contains( _m ) )
        {
            ebenen.push_back( _m );
        }
    }

    for( int k=0; k<ebenen.size(); k++ )
    {
        // For each of the (used) z-axis values that has been put into the list "ebenen"
        int _min_x;
        int _min_y;
        int _min_z;
        int _max_x;
        int _max_y;
        int _max_z;
        bool minAndMaxsInitialized = false;

        if( rooms.size() > 0 )
        {
            int id = rooms[0];
            TRoom * pR = mpRoomDB->getRoom( id );
            if( !pR )
                continue;
            _min_x = pR->x;
            _max_x = _min_x;
            _min_y = pR->y*-1;
            _max_y = _min_y;
            _min_z = pR->z;
            _max_z = _min_z;
            minAndMaxsInitialized = true;
        }

        for( int i=0; i<rooms.size(); i++ )
        {
            int id = rooms[i];
            TRoom * pR = mpRoomDB->getRoom( id );
            if( !pR )
                continue;   // Not a valid room so ignore
            if( pR->z != ebenen[k])
                continue;   // Room is not on the z-axis value level that we currently are working with
            if( ! minAndMaxsInitialized )
            {  // Will get here if FIRST room (in rooms[]) was not a valid one
                _min_x = pR->x;
                _max_x = _min_x;
                _min_y = pR->y*-1;
                _max_y = _min_y;
                _min_z = pR->z;
                _max_z = _min_z;
                minAndMaxsInitialized = true;
            }
            else
            {
                int _m = pR->x;
                if( _m < _min_x )
                    _min_x = _m;
                if( _m > _max_x )
                    _max_x = _m;

                _m = pR->y*-1;
                if( _m < _min_y )
                   _min_y = _m;
                if( _m > _max_y )
                   _max_y = _m;

                // This bit is pointless, we already known that pR->z will be the
                // fixed z-axis value that ebenen[k] holds so _min_z and _max_z will
                // also have that value.
                _m = pR->z;
                if( _m < _min_z )
                   _min_z = _m;
                if( _m > _max_z )
                   _max_z = _m;
            }
        }
        if( minAndMaxsInitialized )
        {
            xminEbene[ebenen[k]] = _min_x;
            yminEbene[ebenen[k]] = _min_y;
            zminEbene[ebenen[k]] = _min_z;
            xmaxEbene[ebenen[k]] = _max_x;
            ymaxEbene[ebenen[k]] = _max_y;
            zmaxEbene[ebenen[k]] = _max_z;
        }
    }
}


