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
//
// Copyright 2012-2014 Dalerank, dalerankn8@gmail.com

#include "statistic.hpp"
#include "objects/construction.hpp"
#include "helper.hpp"
#include "trade_options.hpp"
#include "objects/house.hpp"
#include "objects/constants.hpp"
#include "objects/granary.hpp"
#include "objects/house_level.hpp"
#include "good/goodstore.hpp"
#include "city/funds.hpp"
#include "objects/farm.hpp"
#include "world/empire.hpp"
#include "objects/warehouse.hpp"
#include "cityservice_disorder.hpp"
#include "cityservice_military.hpp"
#include "core/time.hpp"
#include "cityservice_health.hpp"
#include "core/logger.hpp"
#include <map>

using namespace constants;

namespace city
{

void Statistic::getWorkersNumber(PlayerCityPtr city, int& workersNumber, int& maxWorkers )
{
  WorkingBuildingList buildings;
  buildings << city->overlays();

  workersNumber = 0;
  maxWorkers = 0;
  foreach( bld, buildings )
  {
    workersNumber += (*bld)->numberWorkers();
    maxWorkers += (*bld)->maximumWorkers();
  }
}

float Statistic::getBalanceKoeff(PlayerCityPtr city)
{
  if( city.isNull() )
  {
    Logger::warning( "Statistic::getBalanceKoeff cityptr is null");
    return 1.f;
  }

  return atan( city->population() / 1000.f );
}

CitizenGroup Statistic::getPopulation(PlayerCityPtr city)
{
  Helper helper( city );
  HouseList houses = helper.find<House>( building::house );

  CitizenGroup ret;
  foreach( it, houses ) { ret += (*it)->habitants(); }

  return ret;
}

unsigned int Statistic::getWorkersNeed(PlayerCityPtr city)
{
  int have, need;
  getWorkersNumber( city, have, need );
  return need < have ? 0 : need - have;
}

unsigned int Statistic::getAvailableWorkersNumber(PlayerCityPtr city)
{
  Helper helper( city );
  HouseList houses = helper.find<House>( building::house );

  int workersNumber = 0;
  foreach( h, houses )
  {
    workersNumber += (*h)->habitants().count( CitizenGroup::mature );
  }

  return workersNumber;
}

unsigned int Statistic::getMonthlyWorkersWages(PlayerCityPtr city)
{
  int workersNumber, maxWorkers;
  getWorkersNumber( city, workersNumber, maxWorkers );

  if( workersNumber == 0 )
    return 0;

  //wages all worker in year
  //workers take salary in sestertius 1/100 part of dinarius
  int wages = workersNumber * getMonthlyOneWorkerWages( city );

  wages = std::max<int>( wages, 1 );

  return wages;
}

float Statistic::getMonthlyOneWorkerWages(PlayerCityPtr city)
{
  return city->funds().workerSalary() / (10.f * DateTime::monthsInYear);
}

unsigned int Statistic::getWorklessNumber(PlayerCityPtr city)
{
  Helper helper( city );

  HouseList houses = helper.find<House>( building::house );

  int worklessNumber = 0;
  foreach( h, houses ) { worklessNumber += (*h)->getServiceValue( Service::recruter ); }

  return worklessNumber;
}

unsigned int Statistic::getWorklessPercent(PlayerCityPtr city)
{
  return math::percentage( getWorklessNumber( city ), getAvailableWorkersNumber( city ) );
}

unsigned int Statistic::getCrimeLevel( PlayerCityPtr city )
{
  DisorderPtr ds;
  ds << city->findService( Disorder::defaultName() );

  return ds.isValid() ? ds->value() : 0;
}

unsigned int Statistic::getFoodStock(PlayerCityPtr city)
{
  Helper helper( city );

  int foodSum = 0;

  GranaryList granaries = helper.find<Granary>( building::granary );
  foreach( gr, granaries ) { foodSum += (*gr)->store().qty(); }

  return foodSum;
}

unsigned int Statistic::getFoodMonthlyConsumption(PlayerCityPtr city)
{
  Helper helper( city );

  int foodComsumption = 0;
  HouseList houses = helper.find<House>( building::house );

  foreach( h, houses ) { foodComsumption += (*h)->spec().computeMonthlyFoodConsumption( *h ); }

  return foodComsumption;
}

unsigned int Statistic::getFoodProducing(PlayerCityPtr city)
{
  Helper helper( city );

  int foodProducing = 0;
  FarmList farms = helper.find<Farm>( building::foodGroup );

  foreach( f, farms ) { foodProducing += (*f)->getProduceQty(); }

  return foodProducing;
}

unsigned int Statistic::getTaxValue(PlayerCityPtr city)
{
  Helper helper( city );
  HouseList houses = helper.find<House>( building::house );

  float taxValue = 0.f;
  float taxRate = city->funds().taxRate();
  foreach( house, houses )
  {
    int maxhb = (*house)->maxHabitants();
    if( maxhb == 0 )
      continue;

    int maturehb = (*house)->habitants().count( CitizenGroup::mature );
    int housetax = (*house)->spec().taxRate();
    taxValue += housetax * maturehb * taxRate / maxhb;
  }

  return taxValue;
}

unsigned int Statistic::getTaxPayersPercent(PlayerCityPtr city)
{
  Helper helper( city );
  HouseList houses = helper.find<House>( building::house );

  unsigned int registered = 0;
  unsigned int population = 0;
  foreach( house, houses )
  {
    unsigned int hbCount = (*house)->habitants().count();
    population += hbCount;
    if( (*house)->getServiceValue( Service::forum ) > 25 )
    {
      registered += hbCount;
    }
  }

  return math::percentage( registered, population );
}

unsigned int Statistic::getHealth(PlayerCityPtr city)
{
  HealthCarePtr hc;
  hc << city->findService( HealthCare::defaultName() );
  return hc.isValid() ? hc->value() : 0;
}

int Statistic::months2lastAttack(PlayerCityPtr city)
{
  MilitaryPtr ml;
  ml << city->findService( Military::defaultName() );
  return ml.isValid() ? ml->monthFromLastAttack() : 0;
}

int Statistic::getWagesDiff(PlayerCityPtr city)
{
  return city->funds().workerSalary() - city->empire()->workerSalary();
}

unsigned int Statistic::getFestivalCost(PlayerCityPtr city, FestivalType type)
{
  switch( type )
  {
  case smallFest: return int( city->population() / 20 ) + 10;
  case middleFest: return int( city->population() / 10 ) + 20;
  case greatFest: return int( city->population() / 5 ) + 40;
  }

  return 0;
}

HouseList Statistic::getEvolveHouseReadyBy(PlayerCityPtr city, const std::set<int>& checkTypes )
{
  HouseList ret;

  Helper helper( city );
  HouseList houses = helper.find<House>( building::house );

  foreach( it, houses )
  {
    gfx::TileOverlay::Type btype;
    (*it)->spec().next().checkHouse( *it, NULL, &btype );
    if( checkTypes.count( btype ) )
    {    
      ret.push_back( *it );      
    }
  }

  return ret;
}

Statistic::GoodsMap Statistic::getGoodsMap(PlayerCityPtr city)
{
  Helper helper( city );
  GoodsMap cityGoodsAvailable;

  WarehouseList warehouses = helper.find<Warehouse>( building::warehouse );
  foreach( wh, warehouses )
  {
    for( int i=Good::wheat; i < Good::goodCount; i++ )
    {
      Good::Type goodType = (Good::Type)i;
      cityGoodsAvailable[ goodType ] += (*wh)->store().qty( goodType );
    }
  }

  return cityGoodsAvailable;
}

}//end namespace city
