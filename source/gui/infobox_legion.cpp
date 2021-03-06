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

#include <cstdio>

#include "walker/patrolpoint.hpp"
#include "infobox_legion.hpp"
#include "environment.hpp"
#include "core/foreach.hpp"
#include "label.hpp"
#include "core/gettext.hpp"
#include "pushbutton.hpp"
#include "walker/romesoldier.hpp"
#include "city/helper.hpp"
#include "game/resourcegroup.hpp"
#include "core/event.hpp"
#include "objects/fort.hpp"
#include "core/logger.hpp"
#include "core/stringhelper.hpp"
#include "widget_helper.hpp"

using namespace constants;

namespace gui
{

namespace infobox
{

class AboutLegion::Impl
{
public:
  Label* lbFormationTitle;
  Label* lbFormation;
  Label* lbNumberValue;
  Label* lbHealthValue;
  Label* lbMoraleValue;
  Label* lbTrainedValue;
  Label* gbLegionParams;
  PushButton* btnReturn;
  PushButton* btnAttackAnimals;
  FortPtr fort;
};

AboutLegion::AboutLegion(Widget* parent, PlayerCityPtr city, const TilePos& pos  )
  : Simple( parent, Rect( 0, 0, 460, 350 ), Rect() ), _d( new Impl )
{  
  Widget::setupUI( ":/gui/legionopts.gui" );

  GET_DWIDGET_FROM_UI( _d, lbFormationTitle )
  GET_DWIDGET_FROM_UI( _d, lbFormation )
  GET_DWIDGET_FROM_UI( _d, btnReturn )
  GET_DWIDGET_FROM_UI( _d, lbNumberValue )
  GET_DWIDGET_FROM_UI( _d, lbHealthValue )
  GET_DWIDGET_FROM_UI( _d, lbMoraleValue )
  GET_DWIDGET_FROM_UI( _d, lbTrainedValue )
  GET_DWIDGET_FROM_UI( _d, gbLegionParams )
  GET_DWIDGET_FROM_UI( _d, btnAttackAnimals )

  WalkerList walkers = city->walkers( pos );

  if( walkers.empty() )
  {
    _d->gbLegionParams->hide();

    city::Helper helper( city );
    BuildingList barracks = helper.find<Building>( building::barracks );

    std::string text = barracks.empty()
                        ? "##legion_haveho_soldiers_and_barracks##"
                        : "##legion_haveho_soldiers##";

    setText( _( text ) );
  }
  else
  {
    foreach( i, walkers )
    {
      RomeSoldierPtr rs = ptr_cast<RomeSoldier>( *i);
      if( rs.isValid() )
      {
        _d->fort = rs->base();
        break;
      }

      PatrolPointPtr pp = ptr_cast<PatrolPoint>( *i);
      if( pp.isValid() )
      {
        _d->fort = pp->base();
        break;
      }
    }
  }

  setTitle( _( _d->fort.isValid()
                ? _d->fort->legionName()
                : "##unknown_legion##" ) );

  _addAvailalbesFormation();
  _update();

  CONNECT( _d->btnReturn, onClicked(), this, AboutLegion::_returnSoldiers2fort );
  CONNECT( _d->btnAttackAnimals, onClicked(), this, AboutLegion::_toggleAnimalsAttack );
}

AboutLegion::~AboutLegion() {}

void AboutLegion::_update()
{
  if( _d->fort.isNull() )
      return;

  if( _d->lbNumberValue )
  {
    _d->lbNumberValue->setText( StringHelper::i2str( _d->fort->soldiers().size() ) );
  }

  if( _d->lbHealthValue )
  {
    const char* health[] = { "##sldh_health_low##", "##sldh_health_sparse##", "##sldh_health_middle##","##sldh_health_strong##", "##sldh_health_strongest##" };
    int index = math::clamp<unsigned int>( _d->fort->legionHealth() / 20, 0, 4 );
    _d->lbHealthValue->setText( _( health[ index ] ) );
  }

  if( _d->lbMoraleValue )
  {
    const char* morale[] = { "##sldr_totally_distraught##", "##sldr_terrified##", "##sldr_very_frightened##",
                            "##sldr_badly_shaken##", "##sldr_shaken##",
                            "##sldr_extremely_scared##",
                            "##sldr_daring##", "##sldr_encouraged##", "##sdlr_bold##" ,"##sldr_very_bold##" };
    int index = math::clamp<unsigned int>( _d->fort->legionMorale() / 10, 0, 9 );
    _d->lbMoraleValue->setText( _( morale[ index ] ) );
  }

  if( _d->lbTrainedValue )
  {
    _d->lbTrainedValue->setText( StringHelper::i2str( _d->fort->legionTrained() ) );
  }

  if( _d->btnAttackAnimals )
  {
    std::string text = StringHelper::format( 0xff, "##attack_animals_%s##", _d->fort->isAttackAnimals() ? "on" : "off" );
    _d->btnAttackAnimals->setText( text );
  }
}

void AboutLegion::_addAvailalbesFormation()
{
  if( _d->fort.isNull() )
      return;

  Fort::TroopsFormations formations = _d->fort->legionFormations();

  int index = 0;
  int formationPicId[] = { 33, 34, 30, 31, 29, 28 };
  foreach( it, formations )
  {
    _addFormationButton( index, *it, formationPicId[ *it ] );
    index++;
  }
}

void AboutLegion::_toggleAnimalsAttack()
{
  if( _d->fort.isValid() )
  {
    _d->fort->setAttackAnimals( !_d->fort->isAttackAnimals() );
    _update();
  }
}

bool AboutLegion::onEvent(const NEvent& event)
{
  if( event.EventType == sEventGui && event.gui.type == guiButtonClicked )
  {
    int id = event.gui.caller->ID();
    if( id >= 0 )
    {
      std::string text;
      switch( id )
      {
      case Fort::frmNorthLine: text = "##defensive_formation"; break;
      case Fort::frmWestLine: text = "##defensive_formation2_"; break;
      case Fort::frmNorthDblLine: text = "##simple_formation"; break;
      case Fort::frmWestDblLine: text = "##simple_formation2"; break;
      case Fort::frmRandomLocation: text = "##simple_random_location"; break;

      default:
      break;
      }

      if( !text.empty() && _d->lbFormationTitle && _d->lbFormation )
      {
        _d->lbFormationTitle->setText( _(text + "_title##") );
        _d->lbFormation->setText( _(text + "_text##") );
      }

      if( _d->fort.isValid() )
      {
        _d->fort->setFormation( (Fort::TroopsFormation)id );
      }
    }
  }

  if( event.EventType == sEventKeyboard && !event.keyboard.pressed && event.keyboard.control )
  {
    if( event.keyboard.key == KEY_KEY_B ) //build new soldier
    {
      if( _d->fort.isValid() )
      {
        _d->fort->setTraineeValue( walker::soldier, 100 );
      }
      return true;
    }
  }

  return Simple::onEvent( event );
}

void AboutLegion::_addFormationButton(int index, int id, int picId)
{
  Point offset( 83, 0 );
  Rect rect( Point( 16, 140 ) + offset * index, Size( 83 ) );
  PushButton* btn = new PushButton( this, rect, "", id );
  btn->setBackgroundStyle( PushButton::whiteBorderUp );
  btn->setIcon( ResourceGroup::menuMiddleIcons, picId );
  btn->setIconOffset( Point( 1, 1 ) );
  btn->setTooltipText( _("##legion_formation_tooltip##") );
}

void AboutLegion::_returnSoldiers2fort()
{
  if( _d->fort.isValid() )
  {
    _d->fort->returnSoldiers();
  }

  deleteLater();
}

}

}//end namespace gui
