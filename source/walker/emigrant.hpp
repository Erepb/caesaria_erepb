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

#ifndef __CAESARIA_EMIGRANT_H_INCLUDED__
#define __CAESARIA_EMIGRANT_H_INCLUDED__

#include "walker.hpp"
#include "core/predefinitions.hpp"
#include "game/citizen_group.hpp"

/** This is an emigrant coming with his stuff */
class Emigrant : public Walker
{
public:
  static EmigrantPtr create( PlayerCityPtr city );
  static EmigrantPtr send2city( PlayerCityPtr city, const CitizenGroup& peoples,
                                 const gfx::Tile& startTile, std::string thinks );

  bool send2city( const gfx::Tile& startTile );
  void leaveCity( const gfx::Tile& tile );

  void setPeoples( const CitizenGroup& peoples );
  virtual void timeStep(const unsigned long time);

  virtual ~Emigrant();

  virtual void save(VariantMap& stream) const;
  virtual void load(const VariantMap& stream);
  virtual bool die();

protected:
  virtual void _reachedPathway();
  virtual void _brokePathway(TilePos pos);
  virtual void _noWay();

  void _setCartPicture( const gfx::Picture& pic );
  virtual const gfx::Picture& _cartPicture();
  
  Emigrant( PlayerCityPtr city );

  HousePtr _findBlankHouse();
  Pathway _findSomeWay(TilePos startPoint );

  const CitizenGroup& _getPeoples() const;
  bool _checkNearestHouse();
  void _append2house(HousePtr house);
  void _checkHouses(HouseList &hlist);

private:
  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //__CAESARIA_EMIGRANT_H_INCLUDED__
