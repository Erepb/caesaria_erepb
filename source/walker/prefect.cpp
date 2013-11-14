// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.

#include "prefect.hpp"
#include "core/position.hpp"
#include "building/prefecture.hpp"
#include "building/house.hpp"
#include "game/astarpathfinding.hpp"
#include "game/path_finding.hpp"
#include "gfx/tile.hpp"
#include "game/tilemap.hpp"
#include "game/city.hpp"
#include "core/variant.hpp"
#include "game/name_generator.hpp"
#include "core/stringhelper.hpp"
#include "events/event.hpp"
#include "core/logger.hpp"
#include "building/constants.hpp"
#include "corpse.hpp"
#include "game/resourcegroup.hpp"
#include "protestor.hpp"
#include "game/pathway_helper.hpp"

using namespace constants;

class Prefect::Impl
{
public:
  typedef enum { patrol=0,
                 findFire, go2fire, fightFire,
                 go2protestor, fightProtestor,
                 doNothing } PrefectAction;
    
  int water;
  TilePos endPatrolPoint;
  PrefectAction action;
};

Prefect::Prefect(PlayerCityPtr city )
: ServiceWalker( city, Service::prefect ), _d( new Impl )
{
  _setType( walker::prefect );
  _d->water = 0;
  _d->action = Impl::patrol;
  _setAnimation( gfx::prefect );

  setName( NameGenerator::rand( NameGenerator::male ) );
}

void Prefect::_changeTile()
{
  Walker::_changeTile();
}

bool Prefect::_looks4Fire( ServiceWalker::ReachedBuildings& buildings, TilePos& pos )
{
  buildings = getReachedBuildings( getIJ() );

  foreach( BuildingPtr building, buildings )
  {
    if( building->getType() == building::burningRuins )
    {
      pos = building->getTilePos();
      return true;
    }
  }

  return false;
}

bool Prefect::_looks4Protestor( TilePos& pos )
{
  CityHelper helper( _getCity() );
  TilePos offset( 3, 3 );
  ProtestorList protestors = helper.find<Protestor>( walker::protestor, getIJ() - offset, getIJ() + offset );

  int minDistance=99;
  foreach( ProtestorPtr p, protestors )
  {
    int distance = p->getIJ().distanceFrom( getIJ() );
    if( distance < minDistance )
    {
      minDistance =  distance;
      pos = p->getIJ();
    }
  }

  return !protestors.empty();
}

bool Prefect::_checkPath2NearestFire( const ReachedBuildings& buildings )
{
  foreach( BuildingPtr building, buildings )
  {
    if( building->getType() != building::burningRuins )
      continue;

    if( building->getTilePos().distanceFrom( getIJ() ) < 1.5f )
    {
      turn( building->getTilePos() );
      _d->action = Impl::fightFire;
      _setAction( acFight );
      _setAnimation( gfx::prefectFightFire );
      setSpeed( 0.f );
      Walker::_changeDirection();
      return true;
    }
  }

  foreach( BuildingPtr building, buildings )
  {
    if( building->getType() != building::burningRuins )
      continue;

    Pathway tmp = PathwayHelper::create( _getCity(), getIJ(), building->getTilePos(), PathwayHelper::allTerrain );
    if( tmp.isValid() )
    {
      _d->action = Impl::go2fire;
      _updatePathway( tmp );
      _setAnimation( gfx::prefectDragWater );
      setSpeed( 1 );
      go();
      return true;
    }
  }

  return false;
}


void Prefect::_back2Prefecture()
{
  _d->endPatrolPoint = getBase()->getEnterPos();
  _back2Patrol();
}

void Prefect::_serveBuildings( ReachedBuildings& reachedBuildings )
{
  foreach( BuildingPtr building, reachedBuildings )
  {
    building->applyService( ServiceWalkerPtr( this ) );

    HousePtr house = building.as<House>();
    if( house.isValid() && house->getHealthLevel() < 1 )
    {
      house->deleteLater();

      events::GameEventPtr e = events::DisasterEvent::create( house->getTilePos(), events::DisasterEvent::plague );
      e->dispatch();
    }
  }
}

void Prefect::_back2Patrol()
{
  Pathway pathway = PathwayHelper::create( _getCity(), getIJ(), _d->endPatrolPoint, PathwayHelper::allTerrain, Size(0) );

  if( pathway.isValid() )
  {
    _d->action = Impl::patrol;
    _setAnimation( gfx::prefect );
    setSpeed( 1 );
    _updatePathway( pathway );
    go();

    Walker::_changeDirection();
  }
  else
  {
    die();
  }
}

bool Prefect::_findFire()
{
  TilePos firePos;
  ReachedBuildings reachedBuildings;
  bool haveBurningRuinsNear = _looks4Fire( reachedBuildings, firePos );
  if( haveBurningRuinsNear && _d->water > 0 )
  {
    return _checkPath2NearestFire( reachedBuildings );
  }

  return false;
}

void Prefect::_brokePathway(TilePos pos)
{
  TileOverlayPtr overlay = _getCity()->getOverlay( pos );
  if( overlay.isValid() && overlay->getType() == building::burningRuins )
  {
    setSpeed( 0 );
    _setAction( acFight );
    _d->action = Impl::fightFire;
    _setAnimation( gfx::prefectFightFire );
    Walker::_changeDirection();
    return;
  }
  else if( _d->water > 0 )
  {
    TilePos destination = _pathwayRef().getDestination().getIJ();

    Pathway pathway = PathwayHelper::create( _getCity(), getIJ(), destination, PathwayHelper::allTerrain );
    if( pathway.isValid() )
    {
      setSpeed( 1.f );
      _d->action = Impl::findFire;
      _setAnimation( gfx::prefectDragWater );

      setPathway( pathway );
      go();
      return;
    }
  }

  _back2Patrol();
}

void Prefect::_reachedPathway()
{
  switch( _d->action )
  {
  case Impl::patrol:
    if( getIJ() == getBase()->getEnterPos() )
    {
      deleteLater();
      _d->action = Impl::doNothing;
    }
    else
    {
      _back2Prefecture();
    }
  break;

  case Impl::go2fire:
    if( !_findFire() )
    {
      _back2Patrol();
    }
  break;

  default: break;
  }
}

void Prefect::_centerTile()
{
  switch( _d->action )
  {
  case Impl::doNothing:
  break; 

  case Impl::patrol:
  {
    TilePos protestorPos, firePos;
    ReachedBuildings reachedBuildings;
    bool haveProtestorNear = _looks4Protestor( protestorPos );
    bool haveBurningRuinsNear = _looks4Fire( reachedBuildings, firePos );

    if( haveProtestorNear )
    {      
      Pathway pathway = PathwayHelper::create( _getCity(), getIJ(), protestorPos, PathwayHelper::allTerrain );

      if( pathway.isValid() )
      {
        setSpeed( 1.5f );
        _updatePathway( pathway );
        go();

        _d->action = Impl::go2protestor;
      }
    }
    else if( haveBurningRuinsNear )
    {
      //tell our prefecture that need send prefect with water to fight with fire
      //on next deliverService

      //found fire, no water, go prefecture
      getBase().as<Prefecture>()->fireDetect( firePos );
      _back2Prefecture();
    }
    else
    {
      _serveBuildings( reachedBuildings );
    }
  }
  break;

  case Impl::findFire:
  {
    _findFire();
  }
  break;

  case Impl::go2protestor:
  {
    TilePos protestorPos;
    bool haveProtestorNear = _looks4Protestor( protestorPos );
    if( haveProtestorNear )
    {
      if(  protestorPos.distanceFrom( getIJ() ) < 1.5f  )
      {
        _d->action = Impl::fightProtestor;
        setSpeed( 0.f );
        _setAction( acFight );
        _setAnimation( gfx::prefectFight );
        Walker::_changeDirection();
      }
    }
    else
    {
      _back2Patrol();
    }
  }
  break;

  case Impl::go2fire:
  {
    BuildingPtr building = _getNextTile().getOverlay().as<Building>();
    if( building.isValid() && building->getType() == building::burningRuins )
    {
      _d->action = Impl::fightFire;
      _setAnimation( gfx::prefectFightFire );
      _setAction( acFight );
      setSpeed( 0.f );
      Walker::_changeDirection();
    }
  }
  break;

  case Impl::fightFire:
  case Impl::fightProtestor:
  break;
  }
  Walker::_centerTile();
}

void Prefect::timeStep(const unsigned long time)
{
  ServiceWalker::timeStep( time );

  switch( _d->action )
  {
  case Impl::fightFire:
  {    
    BuildingPtr building = _getNextTile().getOverlay().as<Building>();
    bool inFire = (building.isValid() && building->getType() == building::burningRuins );

    if( inFire )
    {
      ServiceWalkerPtr ptr( this );
      const float beforeFight = building->evaluateService( ptr );
      building->applyService( ptr );
      const float afterFight = building->evaluateService( ptr );
      _d->water -= math::clamp( (int)(beforeFight - afterFight), 0, 100 );

      if( afterFight == 0)
      {
        inFire = false;
      }
    }

    if( !inFire && _d->water > 0 )
    {
      if( !_findFire() )
      {
        _back2Patrol();
      }
    }
    else if( _d->water <= 0 )
    {
      _back2Prefecture();
    }
  }
  break;

  case Impl::fightProtestor:
  {
    CityHelper helper( _getCity() );
    ProtestorList protestors = helper.find<Protestor>( walker::protestor,
                                                       getIJ() - TilePos( 1, 1), getIJ() + TilePos( 1, 1) );

    if( !protestors.empty() )
    {
      ProtestorPtr p = protestors.front();

      turn( p->getIJ() );

      p->updateHealth( -3 );
      p->acceptAction( Walker::acFight, getIJ() );
    }
    else
    {
      _back2Patrol();
    }
  }
  break;

  default: break;
  } // end switch( _d->action )
}

Prefect::~Prefect()
{
}

float Prefect::getServiceValue() const
{
  return 5;
}

PrefectPtr Prefect::create(PlayerCityPtr city )
{
  PrefectPtr ret( new Prefect( city ) );
  ret->drop();

  return ret;
}

void Prefect::send2City(PrefecturePtr prefecture, int water/*=0 */ )
{
  _d->action = water > 0 ? Impl::findFire : Impl::patrol;
  _d->water = water;
  _setAnimation( water > 0 ? gfx::prefectDragWater : gfx::prefect );

  if( water > 0 )
  {
    setBase( prefecture.as<Building>() );

    _getCity()->addWalker( WalkerPtr( this ));
  }
  else
  {
    ServiceWalker::send2City( prefecture.as<Building>() );    
  }

  if( _pathwayRef().isValid() )
  {
    _d->endPatrolPoint = _pathwayRef().getDestination().getIJ();
  }
}

void Prefect::die()
{
  ServiceWalker::die();

  Corpse::create( _getCity(), getIJ(), ResourceGroup::citizen2, 711, 718 );
}

void Prefect::load( const VariantMap& stream )
{
  ServiceWalker::load( stream );
 
  _d->action = (Impl::PrefectAction)stream.get( "prefectAction" ).toInt();
  _d->water = (int)stream.get( "water" );
  _d->endPatrolPoint = stream.get( "endPoint" );

  _setAnimation( _d->water > 0 ? gfx::prefectDragWater : gfx::prefect );

  PrefecturePtr prefecture = getBase().as<Prefecture>();
  if( prefecture.isValid() )
  {
    prefecture->addWalker( WalkerPtr( this ) );
    _getCity()->addWalker( WalkerPtr( this ) );
  }
  
  if( prefecture.isNull() )
  {
    Logger::warning( "Not found prefecture on loading" );
    deleteLater();
  }
}

void Prefect::save( VariantMap& stream ) const
{
  ServiceWalker::save( stream );

  stream[ "type" ] = (int)walker::prefect;
  stream[ "water" ] = _d->water;
  stream[ "prefectAction" ] = (int)_d->action;
  stream[ "endPoint" ] = _d->endPatrolPoint;
  stream[ "__debug_typeName" ] = Variant( std::string( OC3_STR_EXT(walker::prefect) ) );
}
