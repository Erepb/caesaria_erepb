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

#include "empireprices.hpp"
#include "city/trade_options.hpp"
#include "city/city.hpp"
#include "good/goodhelper.hpp"
#include "image.hpp"
#include "core/stringhelper.hpp"
#include "label.hpp"
#include "core/event.hpp"

using namespace constants;
using namespace gfx;

namespace gui
{

namespace advisorwnd
{

EmpirePricesWindow::EmpirePricesWindow(Widget *parent, int id, const Rect &rectangle, PlayerCityPtr city)
  : Window( parent, rectangle, "", id )
{
  setupUI( ":/gui/empireprices.gui" );

  city::TradeOptions& ctrade = city->tradeOptions();
  Font font = Font::create( FONT_1 );
  Point startPos( 140, 50 );
  for( int i=Good::wheat; i < Good::prettyWine; i++ )
    {
      if( i == Good::fish || i == Good::denaries)
        {
          continue;
        }

      Good::Type gtype = (Good::Type)i;
      Picture goodIcon = GoodHelper::picture( gtype );
      new Image( this, startPos, goodIcon );

      std::string priceStr = StringHelper::format( 0xff, "%d", ctrade.buyPrice( gtype ) );
      Label* lb = new Label( this, Rect( startPos + Point( 0, 34 ), Size( 24, 24 ) ), priceStr );
      lb->setFont( font );

      priceStr = StringHelper::format( 0xff, "%d", ctrade.sellPrice( gtype ) );
      lb = new Label( this, Rect( startPos + Point( 0, 58 ), Size( 24, 24 ) ), priceStr );
      lb->setFont( font );

      startPos += Point( 30, 0 );
    }
}

void EmpirePricesWindow::draw(Engine &painter)
{
  if( !visible() )
    return;

  Window::draw( painter );
}

bool EmpirePricesWindow::onEvent(const NEvent &event)
{
  if( event.EventType == sEventMouse && event.mouse.isRightPressed() )
    {
      deleteLater();
      return true;
    }

  return Window::onEvent( event );
}

}

}//end namespace gui
