// This file is part of CaesarIA.
//
// CaesarIA is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CaesarIA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CaesarIA.  If not, see <http://www.gnu.org/licenses/>.

#include "health.hpp"
#include "game/resourcegroup.hpp"
#include "game/gamedate.hpp"
#include "core/position.hpp"
#include "gfx/tilemap.hpp"
#include "city/helper.hpp"
#include "constants.hpp"

using namespace constants;
using namespace gfx;

Doctor::Doctor() : ServiceBuilding(Service::doctor, building::doctor, Size(1))
{
}

unsigned int Doctor::walkerDistance() const{ return 26; }

void Doctor::deliverService()
{
  if( numberWorkers() > 0 && walkers().size() == 0 )
  {
    ServiceBuilding::deliverService();
  }
}

Hospital::Hospital() : ServiceBuilding(Service::hospital, building::hospital, Size(3 ) )
{
}

Baths::Baths() : ServiceBuilding(Service::baths, building::baths, Size(2) )
{
  _haveReservorWater = false;
  _fgPicturesRef().resize(1);
}

unsigned int Baths::walkerDistance() const {  return 35;}

bool Baths::build(PlayerCityPtr city, const TilePos& pos)
{
  return ServiceBuilding::build( city, pos );
}

bool Baths::mayWork() const {  return ServiceBuilding::mayWork() && _haveReservorWater; }

void Baths::timeStep(const unsigned long time)
{
  if( GameDate::isWeekChanged() )
  {
    city::Helper helper( _city() );

    bool haveWater = false;
    TilesArray tiles = helper.getArea( this );
    foreach( tile, tiles ) { haveWater |= (*tile)->param( Tile::pReservoirWater ) > 0; }
    _haveReservorWater = (haveWater && numberWorkers() > 0);
  }

  ServiceBuilding::timeStep( time );
}

void Baths::deliverService()
{
  if( _haveReservorWater && numberWorkers() > 0 && walkers().empty() )
  {
    ServiceBuilding::deliverService();
  }
}

Barber::Barber() : ServiceBuilding(Service::barber, building::barber, Size(1))
{
}

void Barber::deliverService()
{
  if( walkers().empty() && numberWorkers() )
  {
    ServiceBuilding::deliverService();
  }
}

unsigned int Barber::walkerDistance() const {  return 35; }
