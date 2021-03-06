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

#include "cityservice_peace.hpp"
#include "city.hpp"
#include "game/gamedate.hpp"
#include "core/priorities.hpp"
#include "objects/house.hpp"
#include "objects/house_level.hpp"
#include <set>

using namespace constants;

namespace city
{

namespace {
  CAESARIA_LITERALCONST(peaceYears)
  CAESARIA_LITERALCONST(protestorOrMugglerSeen)
  CAESARIA_LITERALCONST(rioterSeen)
  CAESARIA_LITERALCONST(value)
  CAESARIA_LITERALCONST(significantBuildingsDestroyed)
}

class Peace::Impl
{
public:
  unsigned int peaceYears;
  bool protestorOrMugglerSeen;
  bool rioterSeen;
  int value;
  bool significantBuildingsDestroyed;

  Priorities<int> unsignificantBuildings;
};

city::SrvcPtr Peace::create()
{
  city::SrvcPtr ret( new Peace() );
  ret->drop();

  return ret;
}

Peace::Peace()
  : city::Srvc( getDefaultName() ), _d( new Impl )
{
  _d->peaceYears = 0;
  _d->protestorOrMugglerSeen = false;
  _d->rioterSeen = false;
  _d->value = 0;
  _d->significantBuildingsDestroyed = false;

  _d->unsignificantBuildings << building::prefecture
                         << building::engineerPost
                         << building::well
                         << building::fortArea
                         << building::fortJavelin
                         << building::fortLegionaire
                         << building::fortMounted
                         << building::gatehouse
                         << building::fortification
                         << construction::road
                         << construction::plaza
                         << building::highBridge
                         << building::lowBridge
                         << building::tower;
}

void Peace::timeStep( PlayerCityPtr city, const unsigned int time )
{
  if( !GameDate::isYearChanged() )
    return;

  int change = _d->protestorOrMugglerSeen ? -1: 0;
  change = std::min( _d->rioterSeen ? -5 : 0, _d->value );
  change = std::min( _d->significantBuildingsDestroyed ? -1 : 0, _d->value );

  if( change == 0 )
  {
    change = _d->peaceYears > 2 ? 5 : 2;
  }

  if( _d->protestorOrMugglerSeen || _d->rioterSeen )
  {
    _d->peaceYears = 0;
  }
  else
  {
    _d->peaceYears++;
  }

  _d->value = math::clamp( _d->value + change, 0, 100  );
  _d->protestorOrMugglerSeen  = false;
  _d->rioterSeen = false;
  _d->significantBuildingsDestroyed = false;
}

void Peace::addProtestor() {  _d->protestorOrMugglerSeen = true; }
void Peace::addRioter() { _d->rioterSeen = true; }

void Peace::buildingDestroyed(gfx::TileOverlayPtr overlay)
{
  HousePtr house = ptr_cast<House>( overlay );
  if( house.isValid() && house->spec().level() > HouseLevel::tent )
  {
    _d->significantBuildingsDestroyed = true;
  }
  else
  {
    _d->significantBuildingsDestroyed |= !_d->unsignificantBuildings.count( overlay->type() );
  }
}

int Peace::value() const { return _d->value; }

std::string Peace::getDefaultName()
{
  return CAESARIA_STR_EXT(Peace);
}

VariantMap Peace::save() const
{
  VariantMap ret;
  ret[ lc_peaceYears ] = (int)_d->peaceYears;
  ret[ lc_protestorOrMugglerSeen ] = _d->protestorOrMugglerSeen;
  ret[ lc_rioterSeen ] = _d->rioterSeen;
  ret[ lc_value ] = _d->value;
  ret[ lc_significantBuildingsDestroyed ] = _d->significantBuildingsDestroyed;

  return ret;
}

void Peace::load(const VariantMap& stream)
{
  _d->peaceYears = stream.get( lc_peaceYears, 0 ).toUInt();
  _d->protestorOrMugglerSeen = stream.get( lc_protestorOrMugglerSeen );
  _d->rioterSeen = stream.get( lc_rioterSeen );
  _d->value = stream.get( lc_value );
  _d->significantBuildingsDestroyed = stream.get( lc_significantBuildingsDestroyed );
}

}
