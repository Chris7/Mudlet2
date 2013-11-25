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


#include "TMap.h"
#include <QDebug>
#include <QMainWindow>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrent>
#include "dlgMapper.h"
#include <boost/graph/graphviz.hpp>
bool skipZoneCheck = false;

struct pathObject
{
    int from;
    int to;
    TMap::mygraph_t graph;
    std::vector<location> locations;
    QList< int > roomList;
    QList< QString > dirList;
    QMap< int, int > roomToIndexMap;
    QMap< int, int > indexToRoomMap;
    TRoomDB * mpRoomDB;
};

struct pathFunctor : public std::unary_function<void, pathObject*> {
    void operator() (pathObject* path);
};

void pathFunctor::operator()(pathObject* path) {    /* Mapping function */
    int from = path->from;
    int to = path->to;
    TMap::vertex start = path->roomToIndexMap[from];
    TMap::vertex goal = path->roomToIndexMap[to];
    vector<TMap::mygraph_t::vertex_descriptor> p(num_vertices(path->graph));
    vector<cost> d(num_vertices(path->graph));
    QTime t;
    t.start();
    try
    {
        astar_search( path->graph,
                      start,
                      distance_heuristic<TMap::mygraph_t, cost, std::vector<location> >(path->locations, goal),
                      predecessor_map(&p[0]).distance_map(&d[0]).
                      visitor(astar_goal_visitor<TMap::vertex>(goal)) );
    }
    catch( found_goal fg )
    {
        qDebug()<<"in functor time elapsed in astar:"<<t.elapsed();
        TRoomDB * mpRoomDB = path->mpRoomDB;
        t.restart();
        list<TMap::vertex> shortest_path;
        for(TMap::vertex v = goal; ; v = p[v])
        {
            //cout << "assembling path: v="<<v<<endl;
            int nextRoom = path->indexToRoomMap[v];
            if( ! mpRoomDB->getRoom( nextRoom ) )
            {
                qDebug()<<"assembly error";
                cout<<"ERROR path assembly: path room not in map!"<<endl;
                return;
            }
            shortest_path.push_front(nextRoom);
            if(p[v] == v) break;
        }
        TRoom * pRD1 = mpRoomDB->getRoom(from);
        TRoom * pRD2 = mpRoomDB->getRoom(to);
        if( !pRD1 || !pRD2 )
        {
            qDebug()<<"room not found";
            return;
        }
        cout << "Shortest path from " << pRD1->getId() << " to "
             << pRD2->getId() << ": ";
        list<TMap::vertex>::iterator spi = shortest_path.begin();
        cout << pRD1->getId();
        QList< int > mPathList;
        QList< QString > mDirList;
        int curRoom = from;
        for( ++spi; spi != shortest_path.end(); ++spi )
        {
            TRoom * pRcurRoom = mpRoomDB->getRoom( curRoom );
            TRoom * pRPath = mpRoomDB->getRoom( *spi );
            if( !pRcurRoom || !pRPath )
            {
                cout << "ERROR: path not possible. curRoom not in map!" << endl;
                mPathList.clear();
                mDirList.clear();
                qDebug()<<"path not possible";
                return;
            }
            //cout <<" spi:"<<*spi<<" curRoom:"<< curRoom << endl;//" -> ";
            mPathList.push_back( *spi );
            if( pRcurRoom->getNorth() == pRPath->getId() )
            {
                mDirList.push_back("n");
            }
            else if( pRcurRoom->getNortheast() == pRPath->getId() )
            {
                mDirList.push_back("ne");
            }
            else if( pRcurRoom->getNorthwest() == pRPath->getId() )
            {
                mDirList.push_back("nw");
            }
            else if( pRcurRoom->getSoutheast() == pRPath->getId() )
            {
                mDirList.push_back("se");
            }
            else if( pRcurRoom->getSouthwest() == pRPath->getId() )
            {
                mDirList.push_back("sw");
            }
            else if( pRcurRoom->getSouth() == pRPath->getId() )
            {
                mDirList.push_back("s");
            }
            else if( pRcurRoom->getEast() == pRPath->getId() )
            {
                mDirList.push_back("e");
            }
            else if( pRcurRoom->getWest() == pRPath->getId() )
            {
                mDirList.push_back("w");
            }
            else if( pRcurRoom->getUp() == pRPath->getId() )
            {
                mDirList.push_back("up");
            }
            else if( pRcurRoom->getDown() == pRPath->getId() )
            {
                mDirList.push_back("down");
            }
            else if( pRcurRoom->getIn() == pRPath->getId() )
            {
                mDirList.push_back("in");
            }
            else if( pRcurRoom->getOut() == pRPath->getId() )
            {
                mDirList.push_back("out");
            }
            else if( pRcurRoom->getOtherMap().size() > 0 )
            {
                QMapIterator<int, QString> it( pRcurRoom->getOtherMap() );
                while( it.hasNext() )
                {
                    it.next();
                    if( it.key() == pRPath->getId() )
                    {
                        QString _cmd = it.value();
                        if( _cmd.size() > 0 && (_cmd.startsWith('0')))
                        {
                            _cmd = _cmd.mid(1);
                            mDirList.push_back( _cmd );
                            //qDebug()<<" adding special exit: roomID:"<<pRcurRoom->getId()<<" OPEN special exit:"<<_cmd;
                        }
//                         else if( _cmd.startsWith('1'))
//                         {
//                             qDebug()<<"NOT adding roomID:"<<pRcurRoom->getId()<<" LOCKED special exit:"<<_cmd;
//                         }
//                         else
//                             qDebug()<<"ERROR adding roomID:"<<pRcurRoom->getId()<<" special exit:"<<_cmd;
                    }
                }
            }

            //qDebug()<<"added to DirList:"<<mDirList.back();
            curRoom = *spi;
        }
        path->dirList = mDirList;
        path->roomList = mPathList;
        qDebug()<<"dir list"<<mDirList;
        qDebug()<<"room list"<<mPathList;
    }
    return;
}

TMap::TMap( Host * pH )
: mpRoomDB( new TRoomDB( this ) )
, mpHost( pH )
, mpM( 0 )
, mpMapper( 0 )
, mMapGraphNeedsUpdate( true )
, mNewMove( true )
, version( 0 )
{
    customEnvColors[257] = mpHost->mRed_2;
    customEnvColors[258] = mpHost->mGreen_2;
    customEnvColors[259] = mpHost->mYellow_2;
    customEnvColors[260] = mpHost->mBlue_2;
    customEnvColors[261] = mpHost->mMagenta_2;
    customEnvColors[262] = mpHost->mCyan_2;
    customEnvColors[263] = mpHost->mWhite_2;
    customEnvColors[264] = mpHost->mBlack_2;
    customEnvColors[265] = mpHost->mLightRed_2;
    customEnvColors[266] = mpHost->mLightGreen_2;
    customEnvColors[267] = mpHost->mLightYellow_2;
    customEnvColors[268] = mpHost->mLightBlue_2;
    customEnvColors[269] = mpHost->mLightMagenta_2;
    customEnvColors[270] = mpHost->mLightCyan_2;
    customEnvColors[271] = mpHost->mLightWhite_2;
    customEnvColors[272] = mpHost->mLightBlack_2;
    unitVectors[1] = QVector3D(0,-1,0);
    unitVectors[2] = QVector3D(1,-1,0);
    unitVectors[3] = QVector3D(-1,-1,0);
    unitVectors[4] = QVector3D(1, 0,0);
    unitVectors[5] = QVector3D(-1,0,0);
    unitVectors[6] = QVector3D(0,1,0);
    unitVectors[7] = QVector3D(1,1,0);
    unitVectors[8] = QVector3D(-1,1,0);
    unitVectors[9] = QVector3D(0,0,1);
    unitVectors[10] = QVector3D(0,0,-1);
    reverseDirections[1] = 6; //contains complementary directions
    reverseDirections[2] = 8;
    reverseDirections[3] = 7;
    reverseDirections[4] = 5;
    reverseDirections[5] = 4;
    reverseDirections[6] = 1;
    reverseDirections[7] = 3;
    reverseDirections[8] = 2;
    reverseDirections[9] = 10;
    reverseDirections[10] = 9;
    reverseDirections[11] = 12;
    reverseDirections[12] = 11;
    m2DPanMode = false;
    mLeftDown = false;
    mRightDown = false;
// N/U:     float m2DPanXStart=0;
// N/U:     float m2DPanYStart=0;
// N/U:     int mViewArea = 0;
}

void TMap::mapClear()
{
    mpRoomDB->clearMapDB();
    mRoomId = 0;
    pixNameTable.clear();
    pixTable.clear();
    envColors.clear();
    customEnvColors.clear();
    mapLabels.clear();
}

#include "TConsole.h"
void TMap::logError( QString & msg )
{
    QColor orange = QColor(255,128,0);
    QColor black = QColor(0,0,0);
    QString s1 = QString("[MAP ERROR:]%1\n").arg(msg);
    if( mpHost->mpEditorDialog )
    {
        mpHost->mpEditorDialog->mpErrorConsole->printDebug(orange, black, s1 );
    }
}



#include <QFileDialog>
void TMap::exportMapToDatabase()
{
    QString dbName = QFileDialog::getSaveFileName( 0, "Chose db file name." );
    QString script = QString("exportMapToDatabse([[%1]])").arg(dbName);
    mpHost->mLuaInterpreter.compileAndExecuteScript( script );
}

void TMap::importMapFromDatabase()
{
    QString dbName = QFileDialog::getOpenFileName( 0, "Chose db file name." );
    QString script = QString("importMapFromDatabase([[%1]])").arg(dbName);
    mpHost->mLuaInterpreter.compileAndExecuteScript( script );
}

void TMap::setRoomArea( int id, int area )
{
    TRoom * pR = mpRoomDB->getRoom( id );

    if( !pR )
    {
        QString msg = QString("roomID=%1 does not exist, can't set area=%2 of nonexisting room").arg(id).arg(area);
        logError(msg);
        return;
    }

    pR->setArea( area );


    mMapGraphNeedsUpdate = true;
}

bool TMap::addRoom( int id )
{
    bool ret = mpRoomDB->addRoom( id );
    if( ret ) mMapGraphNeedsUpdate = true;
    return ret;
}

bool TMap::setRoomCoordinates( int id, int x, int y, int z )
{
    TRoom * pR = mpRoomDB->getRoom( id );
    if( !pR ) return false;

    pR->x = x;
    pR->y = y;
    pR->z = z;

    return true;
}

int compSign(int a, int b){
    return (a < 0) == (b < 0);
}

void TMap::connectExitStub(int roomId, int dirType)
{
    TRoom * pR = mpRoomDB->getRoom( roomId );
    if( !pR ) return;
    int area = pR->getArea();
    int minDistance = 999999;
    int minDistanceRoom=0, meanSquareDistance=0;
    if( !unitVectors.contains( dirType ) ) return;
    QVector3D unitVector = unitVectors[dirType];
    int ux = unitVector.x(), uy = unitVector.y(), uz = unitVector.z();
    int rx = pR->x, ry = pR->y, rz = pR->z;
    int dx=0,dy=0,dz=0;
    TArea * pA = mpRoomDB->getArea(area);
    if( !pA ) return;
    for( int i=0; i< pA->rooms.size(); i++ )
    {
        pR = mpRoomDB->getRoom( pA->rooms[i] );
        if( !pR ) continue;
        if( pR->getId() == roomId ) continue;
        if(uz)
        {
            dz = (int)pR->z-rz;
            if(!compSign(dz,uz) || !dz) continue;
        }
        else
        {
            //to avoid lower/upper floors from stealing stubs
            if((int)pR->z != rz) continue;
        }
        if(ux)
        {
            dx = (int)pR->x-rx;
            if (!compSign(dx,ux) || !dx) //we do !dx to make sure we have a component in the desired direction
                continue;
        }
        else
        {
            //to avoid rooms on same plane from stealing stubs
            if((int)pR->x != rx) continue;
        }
        if(uy)
        {
            dy = (int)pR->y-ry;
            //if the sign is the SAME here we keep it b/c we flip our y coordinate.
            if (compSign(dy,uy) || !dy)
                continue;
        }
        else
        {
            //to avoid rooms on same plane from stealing stubs
            if((int)pR->y != ry) continue;
        }
        meanSquareDistance=dx*dx+dy*dy+dz*dz;
        if(meanSquareDistance < minDistance)
        {
            minDistanceRoom=pR->getId();
            minDistance=meanSquareDistance;
        }
    }
    if(minDistanceRoom)
    {
        pR = mpRoomDB->getRoom(minDistanceRoom);
        if( !pR ) return;
        if(pR->exitStubs.contains(reverseDirections[dirType]))
        {
            setExit( roomId, minDistanceRoom, dirType);
            setExit( minDistanceRoom, roomId, reverseDirections[dirType]);
        }
    }
}

int TMap::createNewRoomID()
{
    int _id = 1;
    for( ; ; _id++ )
    {
        if( ! mpRoomDB->getRoom( _id ) )
        {
            return _id;
        }
    }
    return -1;
}

bool TMap::setExit( int from, int to, int dir )
{
    TRoom * pR = mpRoomDB->getRoom( from );
    TRoom * pR_to = mpRoomDB->getRoom( to );

    if( !pR ) return false;
    if( !pR_to && to > 0 ) return false;
    if( to < 1 ) to = -1;

    mPlausaOptOut = 0;
    bool ret = true;

    switch( dir )
    {
        case DIR_NORTH:
            pR->setNorth(to);
            break;
        case DIR_NORTHEAST:
            pR->setNortheast(to);
            break;
        case DIR_NORTHWEST:
            pR->setNorthwest(to);
            break;
        case DIR_EAST:
            pR->setEast(to);
            break;
        case DIR_WEST:
            pR->setWest(to);
            break;
        case DIR_SOUTH:
            pR->setSouth(to);
            break;
        case DIR_SOUTHEAST:
            pR->setSoutheast(to);
            break;
        case DIR_SOUTHWEST:
            pR->setSouthwest(to);
            break;
        case DIR_UP:
            pR->setUp(to);
            break;
        case DIR_DOWN:
            pR->setDown(to);
            break;
        case DIR_IN:
            pR->setIn(to);
            break;
        case DIR_OUT:
            pR->setOut(to);
            break;
        default:
            ret = false;
    }
    pR->setExitStub(dir, 0);
    mMapGraphNeedsUpdate = true;
    TArea * pA = mpRoomDB->getArea( pR->getArea() );
    pA->fast_ausgaengeBestimmen(pR->getId());
    return ret;
}

void TMap::init( Host * pH )
{
    // init areas
    QTime _time; _time.start();
    if( version < 14 )
    {
        mpRoomDB->initAreasForOldMaps();
    }
    qDebug()<<" TMap::init() initialize area rooms: run time:"<<_time.elapsed();
    mpRoomDB->auditRooms();
    // convert old style labels
    QMapIterator<int, TArea *> it( mpRoomDB->getAreaMap() );
    while( it.hasNext() )
    {
        it.next();
// N/U:         TArea * pA = it.value();
        int areaID = it.key();
        if( mapLabels.contains(areaID) )
        {
            QList<int> labelIDList = mapLabels[areaID].keys();
            for( int i=0; i<labelIDList.size(); i++ )
            {
                TMapLabel l = mapLabels[areaID][labelIDList[i]];
                if( l.pix.isNull() )
                {
                    int newID = createMapLabel(areaID, l.text, l.pos.x(), l.pos.y(), l.pos.z(), l.fgColor, l.bgColor, true, false, 40.0, 50 );
                    if( newID > -1 )
                    {
                        cout << "CONVERTING: old style label areaID:"<<areaID<<" labelID:"<< labelIDList[i]<<endl;
                        mapLabels[areaID][labelIDList[i]] = mapLabels[areaID][newID];
                        deleteMapLabel( areaID, newID );
                    }
                    else
                        cout << "ERROR: cannot convert old style label areaID:"<<areaID<<" labelID:"<< labelIDList[i]<<endl;
                }
                if ( ( l.size.width() > std::numeric_limits<qreal>::max() ) || ( l.size.width() < -std::numeric_limits<qreal>::max() ) )
                {
                    mapLabels[areaID][labelIDList[i]].size.setWidth(l.pix.width());
                }
                if ( ( l.size.height() > std::numeric_limits<qreal>::max() ) || ( l.size.height() < -std::numeric_limits<qreal>::max() ) )
                {
                    mapLabels[areaID][labelIDList[i]].size.setHeight(l.pix.height());
                }
            }
        }
    }
}



void TMap::setView(float x, float y, float z, float zoom )
{
}

void TMap::tidyMap( int areaID )
{
}

void TMap::solveRoomCollision( int id, int creationDirection, bool PCheck )
{
}

QList<int> TMap::detectRoomCollisions( int id )
{
    TRoom * pR = mpRoomDB->getRoom( id );
    if( !pR )
    {
        QList<int> l;
        return l;
    }
    int area = pR->getArea();
    int x = pR->x;
    int y = pR->y;
    int z = pR->z;
    QList<int> collList;
    TArea * pA = mpRoomDB->getArea( area );
    if( !pA )
    {
        QList<int> l;
        return l;
    }
    for( int i=0; i< pA->rooms.size(); i++ )
    {
        pR = mpRoomDB->getRoom( pA->rooms[i] );
        if( !pR ) continue;
        if( pR->x == x && pR->y == y && pR->z == z )
        {
            collList.push_back( pA->rooms[i] );
        }
    }

    return collList;
}

void TMap::astBreitenAnpassung( int id, int id2 )
{
}

void TMap::astHoehenAnpassung( int id, int id2 )
{
}

bool TMap::plausabilitaetsCheck( int area )
{
        return true;
}

bool TMap::fixExits( int id, int dir )
{
        return true;
}

bool TMap::fixExits2( int id )
{
        return true;
}


void TMap::getConnectedNodesGreaterThanX( int id, int min )
{
}

void TMap::getConnectedNodesSmallerThanX( int id, int min )
{
}

void TMap::getConnectedNodesGreaterThanY( int id, int min )
{
}

void TMap::getConnectedNodesSmallerThanY( int id, int min )
{
}

bool TMap::gotoRoom( int r )
{
    mTargetID = r;
    return findPath( mRoomId, r );
}

bool TMap::gotoRoom( int r1, int r2 )
{
    return findPath( r1, r2 );
}

void TMap::initGraph()
{
    /*
     *The graph is comprised as follows:
     *We have 2 graphs, one which is our standard 'everything
     *is interconnected' which has a bad distance metric since
     *each area has its own coordinate system, so between-area
     *distances become difficult to establish.
     *
     *To address this, we have a graph which is comprised entirely
     *of nodes which exit areas. The edge weights for each one of
     *these nodes are the distance between the nodes (as calculated
     *by the movement cost of the standard graph, which are also
     *stored so these costs are only hit once).
     *
     *For movements, we do the following:
     *1) If both start and end are in the same area, proceed on the normal
     *graph
     *2) Otherwise, take the distance from the end goal to the zone connections.
     *   Then take the distance from the start position to all zone connections.
     *   Then establish the fastest route between those zones on the zone-map.
     *   Then stitch together the 3 paths.
     *
     *This approach allows us to use euclidean distance for all metrics, and deals
     *with SOME of the cases where there is some magic wormhole in the corner of
     *an area that would normally not be used (but within-area wormholes may still
     *not be used, it's a crap shoot with those).
     */
    QTime _time; _time.start();
    locations.clear();
    areaLocations.clear();
    roomidToAreaIndex.clear();
    areaIndexToRoomid.clear();
    indexToRoomid.clear();
    roomidToIndex.clear();
    g.clear();
    g = mygraph_t();
    area_graph.clear();
    area_graph = mygraph_t();
    weightmap = get(edge_weight, g);
    area_weightmap = get(edge_weight, area_graph);
    QList<TRoom*> roomList = mpRoomDB->getRoomPtrList();
    int roomCount=0;
    int edgeCount=0;
    for( int _k=0; _k<roomList.size(); _k++ )
    {
        TRoom * pR = roomList[_k];
        int i = pR->getId();
        if( pR->isLocked || i < 1 )
        {
            continue;
        }
        roomCount++;
        int roomExits = edgeCount;
        location l;
        l.x = pR->x;
        l.y = pR->y;
        l.z = pR->z;
        l.id = pR->getId();
        l.area = pR->getArea();
        l.zone_node = 0;
        l.room_cost = pR->getWeight();
        locations.push_back( l );
    }
    for(int i=0;i<locations.size();i++){
        roomidToIndex[locations[i].id] = i;
        indexToRoomid[i] = locations[i].id;
    }
    QMultiMap< int, int > roomListConnections;
    for( int _k=0; _k<roomList.size(); _k++ ){
        TRoom * pR = roomList[_k];
        if( pR->isLocked || !roomidToIndex.contains(pR->getId()) )
        {
            continue;
        }
        int roomIndex = roomidToIndex[pR->getId()];
        TRoom * pN = mpRoomDB->getRoom( pR->getNorth() );
        TRoom * pNW = mpRoomDB->getRoom( pR->getNorthwest() );
        TRoom * pNE = mpRoomDB->getRoom( pR->getNortheast() );
        TRoom * pS = mpRoomDB->getRoom( pR->getSouth() );
        TRoom * pSW = mpRoomDB->getRoom( pR->getSouthwest() );
        TRoom * pSE = mpRoomDB->getRoom( pR->getSoutheast() );
        TRoom * pW = mpRoomDB->getRoom( pR->getWest() );
        TRoom * pE = mpRoomDB->getRoom( pR->getEast() );
        TRoom * pUP = mpRoomDB->getRoom( pR->getUp() );
        TRoom * pDOWN = mpRoomDB->getRoom( pR->getDown() );
        TRoom * pIN = mpRoomDB->getRoom( pR->getIn() );
        TRoom * pOUT = mpRoomDB->getRoom( pR->getOut() );


        QMap<QString, int> exitWeights = pR->getExitWeights();
        int fromArea = pR->getArea();
        QSet< int > roomList;
        if ( areaRoomList.contains( fromArea ) )
            roomList = areaRoomList[fromArea];
        /*
         *First in this routine we build our general graph, but we also
         *build the graph of nodes which exit areas. Following this, we'll
         *connect the nodes within areas since these have not been calculated.
         */
        if( pN && !pN->isLocked )
        {
            if( !pR->hasExitLock( DIR_NORTH ) )
            {
                //edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                             roomidToIndex[pR->getNorth()],
                                            g );
                if( exitWeights.contains("n"))
                    weightmap[e] = pR->getExitWeight("n");
                else
                    weightmap[e] = pN->getWeight();
                if ( pN->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getNorth()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getNorth() );
//                    if( exitWeights.contains("n"))
//                        area_weightmap[e] = pR->getExitWeight("n");
//                    else
//                        area_weightmap[e] = pN->getWeight();
                }
            }
        }
        if( pS && !pS->isLocked )
        {
            if( !pR->hasExitLock( DIR_SOUTH ) )
            {
                //edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getSouth()],
                                            g );
                if( exitWeights.contains("s"))
                    weightmap[e] = pR->getExitWeight("s");
                else
                    weightmap[e] = pS->getWeight();
                if ( pS->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getSouth()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getSouth() );
//                    if( exitWeights.contains("s"))
//                        area_weightmap[e] = pR->getExitWeight("s");
//                    else
//                        area_weightmap[e] = pS->getWeight();
                }
            }
        }
        if( pNE && !pNE->isLocked )
        {
            if( !pR->hasExitLock( DIR_NORTHEAST ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getNortheast()],
                                            g );
                if( exitWeights.contains("ne"))
                    weightmap[e] = pR->getExitWeight("ne");
                else
                    weightmap[e] = pNE->getWeight();
                if ( pNE->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getNortheast()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getNortheast() );
//                    if( exitWeights.contains("ne"))
//                        area_weightmap[e] = pR->getExitWeight("ne");
//                    else
//                        area_weightmap[e] = pNE->getWeight();
                }
            }
        }
        if( pE && !pE->isLocked )
        {
            if( !pR->hasExitLock( DIR_EAST ) )
            {
               //edgeCount++;
               edge_descriptor e;
               bool inserted;
               tie(e, inserted) = add_edge( roomIndex,
                                           roomidToIndex[pR->getEast()],
                                           g );
               if( exitWeights.contains("e"))
                   weightmap[e] = pR->getExitWeight("e");
               else
                   weightmap[e] = pE->getWeight();
               if ( pE->getArea() != fromArea )
               {
//                   tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getEast()], area_graph );
                   roomList.insert( pR->getId() );
                   roomListConnections.insertMulti( pR->getId(), pR->getEast() );
//                   if( exitWeights.contains("e"))
//                       area_weightmap[e] = pR->getExitWeight("e");
//                   else
//                       area_weightmap[e] = pE->getWeight();
               }
            }
        }
        if( pW && !pW->isLocked )
        {
            if( !pR->hasExitLock( DIR_WEST ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                bool exit = false;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getWest()],
                                            g );
                if( exitWeights.contains("w"))
                    weightmap[e] = pR->getExitWeight("w");
                else
                    weightmap[e] = pW->getWeight();
                if ( pW->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getWest()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getWest() );
//                    if( exitWeights.contains("w"))
//                        area_weightmap[e] = pR->getExitWeight("w");
//                    else
//                        area_weightmap[e] = pW->getWeight();
                }
            }
        }
        if( pSW && !pSW->isLocked )
        {
            if( !pR->hasExitLock( DIR_SOUTHWEST ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getSouthwest()],
                                            g );
                if( exitWeights.contains("sw"))
                    weightmap[e] = pR->getExitWeight("sw");
                else
                    weightmap[e] = pSW->getWeight();
                if ( pSW->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getSouthwest()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getSouthwest() );
//                    if( exitWeights.contains("sw"))
//                        area_weightmap[e] = pR->getExitWeight("sw");
//                    else
//                        area_weightmap[e] = pSW->getWeight();
                }
            }
        }
        if( pSE && !pSE->isLocked )
        {
            if( !pR->hasExitLock( DIR_SOUTHEAST ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getSoutheast()],
                                            g );
                if( exitWeights.contains("se"))
                    weightmap[e] = pR->getExitWeight("se");
                else
                    weightmap[e] = pSE->getWeight();
                if ( pSE->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getSoutheast()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getSoutheast() );
//                    if( exitWeights.contains("se"))
//                        area_weightmap[e] = pR->getExitWeight("se");
//                    else
//                        area_weightmap[e] = pSE->getWeight();
                }
            }
        }
        if( pNW && !pNW->isLocked )
        {
            if( !pR->hasExitLock( DIR_NORTHWEST ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getNorthwest()],
                                            g );
                if( exitWeights.contains("nw"))
                    weightmap[e] = pR->getExitWeight("nw");
                else
                    weightmap[e] = pNW->getWeight();
                if ( pNW->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getNorthwest()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getNorthwest() );
//                    if( exitWeights.contains("nw"))
//                        area_weightmap[e] = pR->getExitWeight("nw");
//                    else
//                        area_weightmap[e] = pNW->getWeight();
                }
            }
        }
        if( pUP && !pUP->isLocked )
        {
            if( !pR->hasExitLock( DIR_UP ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getUp()],
                                            g );
                if( exitWeights.contains("up"))
                    weightmap[e] = pR->getExitWeight("up");
                else
                    weightmap[e] = pUP->getWeight();
                if ( pUP->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getUp()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getUp() );
//                    if( exitWeights.contains("up"))
//                        area_weightmap[e] = pR->getExitWeight("up");
//                    else
//                        area_weightmap[e] = pUP->getWeight();
                }
            }
        }
        if( pDOWN && !pDOWN->isLocked )
        {
            if( !pR->hasExitLock( DIR_DOWN ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getDown()],
                                            g );
                if( exitWeights.contains("down"))
                    weightmap[e] = pR->getExitWeight("down");
                else
                    weightmap[e] = pDOWN->getWeight();
                if ( pDOWN->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getDown()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getDown() );
//                    if( exitWeights.contains("down"))
//                        area_weightmap[e] = pR->getExitWeight("down");
//                    else
//                        area_weightmap[e] = pDOWN->getWeight();
                }
            }
        }
        if( pIN && !pIN->isLocked )
        {
            if( !pR->hasExitLock( DIR_IN ) )
            {
//                edgeCount++;
                edge_descriptor e;
                bool inserted;
                tie(e, inserted) = add_edge( roomIndex,
                                            roomidToIndex[pR->getIn()],
                                            g );
                if( exitWeights.contains("in"))
                    weightmap[e] = pR->getExitWeight("in");
                else
                    weightmap[e] = pIN->getWeight();
                if ( pIN->getArea() != fromArea )
                {
//                    tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getIn()], area_graph );
                    roomList.insert( pR->getId() );
                    roomListConnections.insertMulti( pR->getId(), pR->getIn() );
//                    if( exitWeights.contains("in"))
//                        area_weightmap[e] = pR->getExitWeight("in");
//                    else
//                        area_weightmap[e] = pIN->getWeight();
                }
            }
        }
        if( pOUT && !pOUT->isLocked )
        {
            if( !pR->hasExitLock( DIR_OUT ) )
            {
//                 edgeCount++;
                 edge_descriptor e;
                 bool inserted;
                 tie(e, inserted) = add_edge( roomIndex,
                                              roomidToIndex[pR->getOut()],
                                             g );
                 if( exitWeights.contains("out"))
                     weightmap[e] = pR->getExitWeight("out");
                 else
                     weightmap[e] = pOUT->getWeight();
                 if ( pOUT->getArea() != fromArea )
                 {
//                     tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pR->getOut()], area_graph );
                     roomList.insert( pR->getId() );
                     roomListConnections.insertMulti( pR->getId(), pR->getOut() );
//                     if( exitWeights.contains("out"))
//                         area_weightmap[e] = pR->getExitWeight("out");
//                     else
//                         area_weightmap[e] = pOUT->getWeight();
                 }
            }
        }
        if( pR->getOtherMap().size() > 0 )
        {
            QMapIterator<int, QString> it( pR->getOtherMap() );
            while( it.hasNext() )
            {
                it.next();
                int _id = it.key();
                QString _cmd = it.value();
                if( _cmd.size()>0 ) _cmd.remove(0,1);//strip special exit lock information
                TRoom * pSpecial = mpRoomDB->getRoom( _id );
                if( !pSpecial || pR->hasSpecialExitLock( _id, _cmd ) || pSpecial->isLocked)
                    continue;
                else
                {
//                    edgeCount++;
                    edge_descriptor e;
                    bool inserted;
                    tie(e, inserted) = add_edge( roomIndex,
                                                roomidToIndex[pSpecial->getId()],
                                                g );
                    if( exitWeights.contains(_cmd))
                    {
                        weightmap[e] = pR->getExitWeight(_cmd);
                    }
                    else
                    {
                        weightmap[e] = pSpecial->getWeight();
                    }
                    if ( pSpecial->getArea() != fromArea )
                    {
//                        tie( e, inserted ) = add_edge( roomIndex, roomidToIndex[pSpecial->getId()], area_graph );
                        roomList.insert( pR->getId() );
                        roomListConnections.insertMulti( pR->getId(), pSpecial->getId() );
//                        if( exitWeights.contains("_cmd"))
//                            area_weightmap[e] = pR->getExitWeight("_cmd");
//                        else
//                            area_weightmap[e] = pSpecial->getWeight();
                    }

                }
            }
        }
        if ( roomList.size() )
            areaRoomList[fromArea] = roomList;
//        if( roomExits == edgeCount ) locations.pop_back();
    }
    /*
     *At this point our general graph is setup, and we have a graph which is
     *all areas.
     */
    mMapGraphNeedsUpdate = false;
    //now we want to see what's the fastest way between each node. Since this is computationally
    //useless for many paths, we just do euclidean distance and if a path exists, we use it.
    QMapIterator< int, QSet< int > > it(areaRoomList);
    int pathsMade = 0;
    while( it.hasNext() )
    {
        it.next();
        QSetIterator< int > it2( it.value() );
        while( it2.hasNext() )
        {
            int from = it2.next();
            location l;
            l.x = 1;
            l.y = 1;
            l.z = 1;
            l.id = from;
            l.area = 1;
            l.zone_node = 1;
            TRoom * aR = mpRoomDB->getRoom( from );
            l.room_cost = aR->getWeight();
            areaLocations.push_back( l );
        }
    }
    for(int i=0;i<areaLocations.size();i++){
        roomidToAreaIndex[areaLocations[i].id] = i;
        areaIndexToRoomid[i] = areaLocations[i].id;
    }
    QMapIterator< int, int > ri(roomListConnections);
    while( ri.hasNext() )
    {
        ri.next();
        int fIndex = roomidToAreaIndex[ri.key()];
        int tIndex = roomidToAreaIndex[ri.value()];
        edge_descriptor e;
        bool inserted;
        tie( e, inserted ) = add_edge( fIndex, tIndex, area_graph );
        area_weightmap[e] = 1;
    }
//    qDebug()<<"graph before";
//    write_graphviz(std::cout, area_graph);
    it.toFront();
    while( it.hasNext() )
    {
        it.next();
        QSetIterator< int > it2( it.value() );
        while( it2.hasNext() )
        {
            int from = it2.next();
            TRoom * pFrom = mpRoomDB->getRoom( from );
            int fx = pFrom->x;
            int fy = pFrom->y;
            int fz = pFrom->z;
            QSetIterator< int > it3( it.value() );
            QMap< int, int > paths;
            while( it3.hasNext() )
            {
                int to = it3.next();
                if ( to == from )
                    continue;
                TRoom * pTo = mpRoomDB->getRoom( to );
                int dx = fx-pTo->x;
                int dy = fy-pTo->y;
                int dz = fz-pTo->z;
                int distance = (int)::sqrt(dx * dx + dy * dy + dz * dz);
                paths.insert(distance, to);
            }
            QMapIterator< int, int > it4(paths);
            while( it4.hasNext() )
            {
                it4.next();
                //assume there's a path, remove if invalid
                //if( findPath( from, it4.value() ) )
                //{
                    edge_descriptor e;
                    bool inserted;
                    tie( e, inserted ) = add_edge( roomidToAreaIndex[from], roomidToAreaIndex[it4.value()], area_graph );
                    area_weightmap[e] = it4.key();
                    //break;
                //}
            }
            pathsMade++;
            qDebug()<<pathsMade<<"paths made";
        }
    }
//    write_graphviz(std::cout, area_graph);
    qDebug()<<"initGraph: nodes: "<<locations.size()<<"/"<<roomCount<<" edges:"<<edgeCount<<" run time:"<<_time.elapsed();
}

bool TMap::findZonePath( int from, int to )
{
    vertex start = roomidToAreaIndex[from];
    vertex goal = roomidToAreaIndex[to];
    qDebug()<<"What we're looking for";
    qDebug()<<from<<to;
    qDebug()<<start<<goal;
    vector<mygraph_t::vertex_descriptor> p(num_vertices(area_graph));
    vector<cost> d(num_vertices(area_graph));

    QTime t;
    t.start();
    try
    {
        astar_search( area_graph,
                      start,
                      distance_heuristic<mygraph_t, cost, std::vector<location> >(areaLocations, goal),
                      predecessor_map(&p[0]).distance_map(&d[0]).
                      visitor(astar_goal_visitor<vertex>(goal)) );
    }
    catch( found_goal fg )
    {
        qDebug()<<"time elapsed in zone astar:"<<t.elapsed();
        t.restart();
        zone_shortest_path.clear();
        for(vertex v = goal; ; v = p[v])
        {
            cout << "assembling zone path: v="<<v<<endl;
            int nextRoom = areaIndexToRoomid[v];
            if( ! mpRoomDB->getRoom( nextRoom ) )
            {
                cout<<"ERROR path assembly: path room not in map!"<<endl;
                return false;
            }
            zone_shortest_path.push_front(nextRoom);
            qDebug()<<"adding"<<nextRoom<<"to shortest path";
            if(p[v] == v) break;
        }
        return true;
    }
    qDebug()<<"nothing found!";
    skipZoneCheck = true;
    return false;
}

bool TMap::findPath( int from, int to )
{
     if( mMapGraphNeedsUpdate )
     {
        initGraph();
     }

     //vertex start = from;//mRoomId;
     //vertex goal = to;//mTargetID;
     TRoom * pFrom = mpRoomDB->getRoom( from );
     TRoom * pTo = mpRoomDB->getRoom( to );

     if( !pFrom || !pTo )
     {
         return false;
     }
     /*
      *This problem can be broken down into several components.
      *1) We're traveling in the same zone - in this case do as we
      *   already do
      *2) We're moving zones - in this case we want to figure out
      *   the fastest way out of the zone, the fastest way to the
      *   new zone, and the fastest way from that new zone to
      *   our target room
      */
     pathFunctor functor;
     if ( !skipZoneCheck && pFrom->getArea() != pTo->getArea() )
     {
         //find exits from current room
         QSetIterator< int > it(areaRoomList[pFrom->getArea()]);
         QMap< int, int > nodePaths;
         QMap< int, QList< int > > nodePathList;
         QMap< int, QList< QString > > nodeDirList;
//         QList< pathObject* > startPaths;
         while( it.hasNext() )
         {
             int exitNode = it.next();
             if ( from == exitNode ) //special case: we're at the exit
                 nodePaths.insert( 0, exitNode );
             else if ( findPath( from, exitNode ) )
             {

                 nodePaths.insert( mPathList.size(), exitNode );
                 nodePathList.insert( exitNode, mPathList );
                 nodeDirList.insert( exitNode, mDirList );
             }
//             pathObject* startNode = new startNode();
//             startNode->from = from;
//             startNode->to = exitNode;
//             startNode->graph = g;
//             startNode->indexToRoomMap = indexToRoomid;
//             startNode->roomToIndexMap = roomidToIndex;
//             startNode->locations = locations;
//             startNode->mpRoomDB = mpRoomDB;
//             startPaths << startNode;
         }
         QSetIterator< int > it2(areaRoomList[pTo->getArea()]);
         QMap< int, int > goalNodePaths;
         QMap< int, QList< int > > goalNodePathList;
         QMap< int, QList< QString > > goalNodeDirList;
         while( it2.hasNext() )
         {
             int exitNode = it2.next();
             if ( exitNode == to )
                 goalNodePaths.insert( 0, exitNode );
             else if ( findPath( exitNode, to ) )
             {
                 goalNodePaths.insert( mPathList.size(), exitNode );
                 goalNodePathList.insert( exitNode, mPathList );
                 goalNodeDirList.insert( exitNode, mDirList );
             }
         }
         qDebug()<<"nodePaths"<<nodePaths;
         qDebug()<<"goalnodePaths"<<goalNodePaths;
         //we have the cheapest route (sort of)
         if ( !nodePaths.size() || !goalNodePaths.size() )
             return false;
         QMapIterator< int, int > it3(nodePaths);
         it3.next();
         int exitNode = it3.value();
         QMapIterator< int, int > it4(goalNodePaths);
         it4.next();
         int goalExitNode = it4.value();
         QList< int > finalPath = nodePathList[exitNode];
         QList< QString > finalDirs = nodeDirList[exitNode];
         if ( findZonePath( exitNode, goalExitNode ) ){
             qDebug()<<"path to stitch starting at"<<from;
             qDebug()<<"exiting current zone:"<<nodePathList[exitNode];
             qDebug()<<"zones we need to transverse:";
             skipZoneCheck = true;
             list<vertex>::iterator spi = zone_shortest_path.begin();
             for( ++spi; spi != zone_shortest_path.end(); ++spi )
             {
                 int nextRoom = *spi;
                 if( findPath( exitNode, nextRoom ) )
                 {
                     qDebug()<<*spi<<mPathList;
                     for(int i=0;i<mPathList.size();i++)
                         finalPath << mPathList[i];
                     for(int i=0;i<mDirList.size();i++)
                         finalDirs << mDirList[i];
                 }
                 else
                 {
                     qDebug()<<"no path possible!";
                     skipZoneCheck = false;
                     return false;
                 }
                 exitNode = *spi;
             }
             for(int i=0;i<goalNodePathList.size();i++)
                 finalPath << goalNodePathList[i];
             for(int i=0;i<goalNodeDirList.size();i++)
                 finalDirs << goalNodeDirList[i];
             qDebug()<<"starting of end zone to end room"<<goalExitNode<<goalNodePathList[goalExitNode];
         }
         else
             return false;
         qDebug()<<"final path"<<finalPath;
         qDebug()<<"final dirs"<<finalDirs;
         mPathList = finalPath;
         mDirList = finalDirs;
         return true;
     }
     else
     {
         pathObject *path = new pathObject();
         path->from = from;
         path->to = to;
         path->graph = g;
         path->indexToRoomMap = indexToRoomid;
         path->roomToIndexMap = roomidToIndex;
         path->locations = locations;
         path->mpRoomDB = mpRoomDB;
         QList< pathObject* > paths;
         paths << path;
         QtConcurrent::blockingMap( paths, functor);
         qDebug()<<"path list"<<path->dirList;
         qDebug()<<"dir list"<<path->roomList;
         if (path->dirList.size())
         {
            mPathList = path->roomList;
            mDirList = path->dirList;
            return true;
         }
         else
         {
             mPathList.clear();
             mDirList.clear();
             return false;
         }
     }
     return false;
}

const int CURRENT_MAP_VERSION = 16;

bool TMap::serialize( QDataStream & ofs )
{
    version = CURRENT_MAP_VERSION;
    ofs << version;
    ofs << envColors;
    ofs << mpRoomDB->getAreaNamesMap();
    ofs << customEnvColors;
    ofs << mpRoomDB->hashTable;

    ofs << mpRoomDB->getAreaMap().size();
    // serialize area table
    QMapIterator<int, TArea *> itAreaList(mpRoomDB->getAreaMap());
    while( itAreaList.hasNext() )
    {
        itAreaList.next();
        int areaID = itAreaList.key();
        TArea * pA = itAreaList.value();
        ofs << areaID;
        ofs << pA->rooms;
        ofs << pA->ebenen;
        ofs << pA->exits;
        ofs << pA->gridMode;
        ofs << pA->max_x;
        ofs << pA->max_y;
        ofs << pA->max_z;
        ofs << pA->min_x;
        ofs << pA->min_y;
        ofs << pA->min_z;
        ofs << pA->span;
        ofs << pA->xmaxEbene;
        ofs << pA->ymaxEbene;
        ofs << pA->zmaxEbene;
        ofs << pA->xminEbene;
        ofs << pA->yminEbene;
        ofs << pA->zminEbene;
        ofs << pA->pos;
        ofs << pA->isZone;
        ofs << pA->zoneAreaRef;
    }

    if (mRoomId)
        ofs << mRoomId;
    else{
        mRoomId = 0;
        ofs << mRoomId;
    }
    ofs << mapLabels.size(); //anzahl der areas
    QMapIterator<int, QMap<int, TMapLabel> > itL1(mapLabels);
    while( itL1.hasNext() )
    {
        itL1.next();
        int i = itL1.key();
        ofs << itL1.value().size();//anzahl der labels pro area
        ofs << itL1.key(); //area id
        QMapIterator<int, TMapLabel> itL2(mapLabels[i]);
        while( itL2.hasNext() )
        {
            itL2.next();
// N/U:             int ii = itL2.key();
            ofs << itL2.key();//label ID
            TMapLabel label = itL2.value();
            ofs << label.pos;
            ofs << label.pointer;
            ofs << label.size;
            ofs << label.text;
            ofs << label.fgColor;
            ofs << label.bgColor;
            ofs << label.pix;
            ofs << label.noScaling;
            ofs << label.showOnTop;
        }
    }
    QMapIterator<int, TRoom *> it( mpRoomDB->getRoomMap() );
    while( it.hasNext() )
    {

        it.next();
// N/U:         int i = it.key();
        TRoom * pR = it.value();
        ofs <<  pR->getId();
        ofs << pR->getArea();
        ofs << pR->x;
        ofs << pR->y;
        ofs << pR->z;
        ofs << pR->getNorth();
        ofs << pR->getNortheast();
        ofs << pR->getEast();
        ofs << pR->getSoutheast();
        ofs << pR->getSouth();
        ofs << pR->getSouthwest();
        ofs << pR->getWest();
        ofs << pR->getNorthwest();
        ofs << pR->getUp();
        ofs << pR->getDown();
        ofs << pR->getIn();
        ofs << pR->getOut();
        ofs << pR->environment;
        ofs << pR->getWeight();
//        ofs << rooms[i]->xRot;
//        ofs << rooms[i]->yRot;
//        ofs << rooms[i]->zRot;
//        ofs << rooms[i]->zoom;
        ofs << pR->name;
        ofs << pR->isLocked;
        ofs << pR->getOtherMap();
        ofs << pR->c;
        ofs << pR->userData;
        ofs << pR->customLines;
        ofs << pR->customLinesArrow;
        ofs << pR->customLinesColor;
        ofs << pR->customLinesStyle;
        ofs << pR->exitLocks;
        ofs << pR->exitStubs;
        ofs << pR->getExitWeights();
        ofs << pR->doors;
    }

    return true;
}

#include <QDir>

bool TMap::restore(QString location)
{
    qDebug()<<"restoring map of profile:"<<mpHost->getName()<<" url:"<<mpHost->getUrl();
    QTime _time; _time.start();
    QString folder;
    QStringList entries;
    qDebug()<<"RESTORING MAP";

    if(location == "")
    {
        folder = QDir::homePath()+"/.config/mudlet/profiles/"+mpHost->getName()+"/map/";
        QDir dir( folder );
        dir.setSorting(QDir::Time);
        entries = dir.entryList( QDir::Files, QDir::Time );
        for( int i=0;i<entries.size(); i++ )
            qDebug()<<i<<"#"<<entries[i];
    }

    bool canRestore = true;
    if( entries.size() > 0 || location != "")
    {
        QFile file((location == "") ? folder+entries[0] : location);
        if (!file.open( QFile::ReadOnly ))
            return false;
        qDebug()<<"[LOADING MAP]:"<<file.fileName();
        QDataStream ifs( & file );

        ifs >> version;
        qDebug()<<"map version:"<<version;
        if( version > CURRENT_MAP_VERSION )
        {
            qDebug()<<"ERROR: map version > CURRENT_MAP_VERSION. Please upgrade Mudlet";
            file.close();
            return false;
        }
        if( version >= 3 )
        {
            ifs >> envColors;
        }
        else
        {
            canRestore = false;
        }
        if( version >= 4 )
        {
            mpRoomDB->restoreAreaMap(ifs);
        }
        else
        {
            canRestore = false;
        }
        if( version >= 5 )
        {
            ifs >> customEnvColors;
        }
        if( version > 6 )
        {
            ifs >> mpRoomDB->hashTable;
        }
        if( version >= 14 )
        {
            int areaSize;
            ifs >> areaSize;
            // restore area table
            for( int i=0; i<areaSize; i++ )
            {
                TArea * pA = new TArea( this, mpRoomDB );
                int areaID;
                ifs >> areaID;
                ifs >> pA->rooms;

                ifs >> pA->ebenen;
                ifs >> pA->exits;
                ifs >> pA->gridMode;
                ifs >> pA->max_x;
                ifs >> pA->max_y;
                ifs >> pA->max_z;
                ifs >> pA->min_x;
                ifs >> pA->min_y;
                ifs >> pA->min_z;
                ifs >> pA->span;
                ifs >> pA->xmaxEbene;
                ifs >> pA->ymaxEbene;
                ifs >> pA->zmaxEbene;
                ifs >> pA->xminEbene;
                ifs >> pA->yminEbene;
                ifs >> pA->zminEbene;
                qDebug()<<"areaID:"<<areaID<<" rooms:"<<pA->rooms<<" exits:"<<pA->exits<<" xmaxEbene:"<<pA->xmaxEbene<<" ymaxEbene:"<<pA->ymaxEbene;
                ifs >> pA->pos;
                ifs >> pA->isZone;
                ifs >> pA->zoneAreaRef;
                mpRoomDB->restoreSingleArea( ifs, areaID, pA );
            }
        }

        if( version >= 12 )
        {
            ifs >> mRoomId;
        }
        if( version >= 11 )
        {
            int size;
            ifs >> size; //size of mapLabels
            int areaLabelCount = 0;
            while( ! ifs.atEnd() && areaLabelCount < size )
            {
                int areaID;
                int size_labels;
                ifs >> size_labels;
                ifs >> areaID;
                int labelCount = 0;
                QMap<int, TMapLabel> _map;
                while( ! ifs.atEnd() &&  labelCount < size_labels )
                {
                    int labelID;
                    ifs >> labelID;
                    TMapLabel label;
                    if( version >= 12 )
                    {
                        ifs >> label.pos;
                    }
                    else
                    {
                        QPointF __label_pos;
                        ifs >> __label_pos;
                        label.pos = QVector3D(__label_pos.x(), __label_pos.y(), 0);
                    }
                    ifs >> label.pointer;
                    ifs >> label.size;
                    ifs >> label.text;
                    ifs >> label.fgColor;
                    ifs >> label.bgColor;
                    ifs >> label.pix;
                    if( version >= 15 )
                    {
                        ifs >> label.noScaling;
                        ifs >> label.showOnTop;
                    }
                    _map.insert( labelID, label );
                    labelCount++;
                }
                mapLabels[areaID] = _map;
                areaLabelCount++;
            }
        }
        while( ! ifs.atEnd() )
        {
            int i;
            ifs >> i;
            TRoom * pT = new TRoom(mpRoomDB);
            mpRoomDB->restoreSingleRoom( ifs, i, pT );
            pT->restore( ifs, i, version );


        }
        customEnvColors[257] = mpHost->mRed_2;
        customEnvColors[258] = mpHost->mGreen_2;
        customEnvColors[259] = mpHost->mYellow_2;
        customEnvColors[260] = mpHost->mBlue_2;
        customEnvColors[261] = mpHost->mMagenta_2;
        customEnvColors[262] = mpHost->mCyan_2;
        customEnvColors[263] = mpHost->mWhite_2;
        customEnvColors[264] = mpHost->mBlack_2;
        customEnvColors[265] = mpHost->mLightRed_2;
        customEnvColors[266] = mpHost->mLightGreen_2;
        customEnvColors[267] = mpHost->mLightYellow_2;
        customEnvColors[268] = mpHost->mLightBlue_2;
        customEnvColors[269] = mpHost->mLightMagenta_2;
        customEnvColors[270] = mpHost->mLightCyan_2;
        customEnvColors[271] = mpHost->mLightWhite_2;
        customEnvColors[272] = mpHost->mLightBlack_2;
        qDebug()<<"LOADED rooms:"<<mpRoomDB->size()<<" loading time:"<<_time.elapsed();
        if( canRestore )
        {
            return true;
        }
    }
    if( ! canRestore || entries.size() == 0 )
    {
        QMessageBox msgBox;

        if( mpHost->mUrl.toLower().contains( "achaea.com" )
            || mpHost->mUrl.toLower().contains( "aetolia.com" )
            || mpHost->mUrl.toLower().contains( "imperian.com" )
            || mpHost->mUrl.toLower().contains( "midkemiaonline.com" )
            || mpHost->mUrl.toLower().contains( "lusternia.com" ) )
        {
            msgBox.setText("No map found. Would you like to download the map or start your own?");
            QPushButton *yesButton = msgBox.addButton("Download the map", QMessageBox::ActionRole);
            QPushButton *noButton = msgBox.addButton("Start my own", QMessageBox::ActionRole);
            msgBox.exec();
            init( mpHost );
            if (msgBox.clickedButton() == yesButton) {
                mpMapper->downloadMap();
            }
            else if(msgBox.clickedButton() == noButton) {
                ; //No-op to avoid unused "noButton"
            }
        }
        else
        {
            mpHost->mpMap->init( mpHost );
        }
    }
    return canRestore;//FIXME
}


int TMap::createMapLabel(int area, QString text, float x, float y, float z, QColor fg, QColor bg, bool showOnTop, bool noScaling, qreal zoom, int fontSize )
{
    if( ! mpRoomDB->getArea( area ) ) return -1;

    TMapLabel label;
    label.text = text;
    label.bgColor = bg;
    label.bgColor.setAlpha(50);
    label.fgColor = fg;
    label.size = QSizeF(100,100);
    label.pos = QVector3D( x, y, z);
    label.showOnTop = showOnTop;
    label.noScaling = noScaling;

    if( label.text.length() < 1 )
    {
        return -1;
    }
    QRectF lr = QRectF( 0, 0, 1000, 1000 );
    QPixmap pix( lr.size().toSize() );
    pix.fill(QColor(0,0,0,0));
    QPainter lp( &pix );
    lp.fillRect( lr, label.bgColor );
    QPen lpen;
    lpen.setColor( label.fgColor );
    QFont font;
    font.setPointSize(fontSize); //good: font size = 50, zoom = 30.0
    lp.setRenderHint(QPainter::TextAntialiasing, true);
    lp.setPen( lpen );
    lp.setFont(font);
    QRectF br;
    lp.drawText( lr, Qt::AlignLeft|Qt::AlignTop, label.text, &br );

    label.size = br.normalized().size();
    label.pix = pix.copy(br.normalized().topLeft().x(), br.normalized().topLeft().y(), br.normalized().width(), br.normalized().height());
    QSizeF s = QSizeF(label.size.width()/zoom, label.size.height()/zoom);
    label.size = s;
    label.clickSize = s;
    if( ! mpRoomDB->getArea(area) ) return -1;
    int labelID;
    if( !mapLabels.contains( area ) )
    {
        QMap<int, TMapLabel> m;
        m[0] = label;
        mapLabels[area] = m;
    }
    else
    {
        labelID = createMapLabelID( area );
        if( labelID > -1 )
        {
            mapLabels[area].insert(labelID, label);
        }
    }

    if( mpMapper ) mpMapper->mp2dMap->update();
    return labelID;
}

int TMap::createMapImageLabel(int area, QString imagePath, float x, float y, float z, float width, float height, float zoom, bool showOnTop, bool noScaling )
{
    if( ! mpRoomDB->getArea( area ) ) return -1;

    TMapLabel label;
    label.size = QSizeF(width, height);
    label.pos = QVector3D( x, y, z);
    label.showOnTop = showOnTop;
    label.noScaling = noScaling;

    QRectF drawRect = QRectF( 0, 0, width*zoom, height*zoom );
    QPixmap imagePixmap = QPixmap(imagePath);
    QPixmap pix = QPixmap( drawRect.size().toSize() );
    pix.fill(QColor(0,0,0,0));
    QPainter lp( &pix );
    lp.drawPixmap(QPoint(0,0), imagePixmap.scaled(drawRect.size().toSize()));
    label.size = QSizeF(width, height);
    label.pix = pix;
    if( ! mpRoomDB->getArea(area) ) return -1;
    int labelID;
    if( !mapLabels.contains( area ) )
    {
        QMap<int, TMapLabel> m;
        m[0] = label;
        mapLabels[area] = m;
    }
    else
    {
        labelID = createMapLabelID( area );
        if( labelID > -1 )
        {
            mapLabels[area].insert(labelID, label);
        }
    }

    if( mpMapper ) mpMapper->mp2dMap->update();
    return labelID;
}


int TMap::createMapLabelID(int area )
{
    if( mapLabels.contains( area ) )
    {
        QList<int> idList = mapLabels[area].keys();
        int id = 0;
        while( id >= 0 )
        {
            if( !idList.contains( id ) )
            {
                return id;
            }
            id++;
        }
    }
    return -1;
}

void TMap::deleteMapLabel(int area, int labelID )
{
    if( ! mpRoomDB->getArea( area ) ) return;
    if( ! mapLabels.contains( area ) ) return;
    if( ! mapLabels[area].contains( labelID ) ) return;
    mapLabels[area].remove( labelID );
    if( mpMapper ) mpMapper->mp2dMap->update();
}



