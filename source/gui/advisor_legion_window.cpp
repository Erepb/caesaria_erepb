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

#include "advisor_legion_window.hpp"
#include "gfx/decorator.hpp"
#include "core/gettext.hpp"
#include "gui/pushbutton.hpp"
#include "gui/label.hpp"
#include "game/resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "gfx/engine.hpp"
#include "city/city.hpp"
#include "texturedbutton.hpp"
#include "objects/military.hpp"
#include "walker/soldier.hpp"
#include "core/logger.hpp"
#include "events/movecamera.hpp"
#include "widget_helper.hpp"
#include "legion_target_window.hpp"
#include "environment.hpp"

using namespace gfx;

namespace gui
{

namespace advisorwnd
{

namespace {
  Point legionButtonOffset = Point( 4, 4 );
  Size legionButtonSize = Size( 565, 42 );
}

class LegionButton : public PushButton
{
public:
  LegionButton( Widget* parent, const Point& pos, int index, FortPtr fort )
    : PushButton( parent, Rect( pos + legionButtonOffset * index, legionButtonSize), "", -1, false, PushButton::blackBorderUp )
  {
    _fort = fort;
    _resizeEvent();

    PushButton* gotoLegion    = new PushButton( this, Rect( 360, 5, 360 + 32, 6 + 32 ), "", -1, false, PushButton::blackBorderUp );
    gotoLegion->setIcon( ResourceGroup::panelBackground, 563 );
    gotoLegion->setIconOffset( Point( 4, 4 ) );
    PushButton* return2fort   = new PushButton( this, Rect( 450, 5, 450 + 32, 6 +32 ), "", -1, false, PushButton::blackBorderUp );
    return2fort->setIcon(  ResourceGroup::panelBackground, 564 );
    return2fort->setIconOffset( Point( 4, 4 ) );
    return2fort->setTooltipText( _("##return_2_fort##") );
    PushButton* empireService = new PushButton( this, Rect( 530, 5, 530 + 32, 6 + 32), "", -1, false, PushButton::blackBorderUp );
    empireService->setIcon( ResourceGroup::panelBackground, 566 );
    empireService->setIconOffset( Point( 4, 4 ) );
    empireService->setTooltipText( "##empire_service_tip##");

    CONNECT( gotoLegion, onClicked(), this, LegionButton::_resolveMove2Legion );
    CONNECT( return2fort, onClicked(), this, LegionButton::_resolveReturnLegion2Fort );
    CONNECT( empireService, onClicked(), this, LegionButton::_resolveEmpireService );
  }

  virtual void _updateTextPic()
  {
    PushButton::_updateTextPic();

    PictureRef& pic = _textPictureRef();

    Font fontW = Font::create( FONT_1_WHITE );
    Font fontB = Font::create( FONT_1 );

    if( _fort.isValid() )
    {
      fontW.draw( *pic, _fort->legionName(), 70, 4 );

      std::string qtyStr = StringHelper::format( 0xff, "%d %s", _fort->soldiers().size(), _("##soldiers##") );
      fontB.draw( *pic, qtyStr, 70, 22 );

      int moraleValue = _fort->legionMorale() / 10;
      std::string moraleStr = StringHelper::format( 0xff, "##legion_morale_%d##", moraleValue );
      fontB.draw( *pic, _( moraleStr ), 180, 15 );
    }    
  }

  virtual void draw(Engine &painter)
  {
    PushButton::draw( painter );

    painter.draw( _fort->legionEmblem(), absoluteRect().lefttop() + Point( 6, 4 ), &absoluteClippingRectRef() );
  }

public signals:
  Signal1<FortPtr> onShowLegionSignal;
  Signal1<FortPtr> onLegionRetreatSignal;
  Signal1<FortPtr> onEmpireServiceSignal;

private slots:
  void _resolveMove2Legion() { emit onShowLegionSignal( _fort ); }
  void _resolveReturnLegion2Fort() { emit onLegionRetreatSignal( _fort ); }
  void _resolveEmpireService() { emit onEmpireServiceSignal( _fort ); }

private:
  FortPtr _fort;
};

class Legion::Impl
{
public:
  gui::Label* alarm;
  gui::Label* helpRequest;
  gui::Label* lbBlackframe;
  FortPtr currentFort;
  PlayerCityPtr city;
};

Legion::Legion( Widget* parent, int id, PlayerCityPtr city, FortList forts )
: Window( parent, Rect( 0, 0, 1, 1 ), "", id ), _d( new Impl )
{
  Widget::setupUI( ":/gui/legionadv.gui" );
  setPosition( Point( (parent->width() - 640 )/2, parent->height() / 2 - 242 ) );

  //buttons background
  Point startLegionArea( 32, 70 );
  _d->city = city;

  GET_DWIDGET_FROM_UI( _d, alarm )
  GET_DWIDGET_FROM_UI( _d, helpRequest )
  GET_DWIDGET_FROM_UI( _d, lbBlackframe )

  int index=0;
  foreach( it, forts )
  {
    LegionButton* btn = new LegionButton( this, startLegionArea + legionButtonOffset, index++, *it );

    CONNECT( btn, onShowLegionSignal, this, Legion::_handleMove2Legion );
    CONNECT( btn, onLegionRetreatSignal, this, Legion::_handleRetreaLegion );
    CONNECT( btn, onEmpireServiceSignal, this, Legion::_handleServiceEmpire );
  }

  if( _d->lbBlackframe && forts.empty() )
  {
    _d->lbBlackframe->setText( _("##legionadv_no_legions##") );
  }
}

void Legion::draw( Engine& painter )
{
  if( !visible() )
    return;

  Window::draw( painter );
}

void Legion::_handleMove2Legion(FortPtr fort)
{
  parent()->deleteLater();
  events::GameEventPtr e = events::MoveCamera::create( fort->patrolLocation() );
  e->dispatch();
}

void Legion::_handleRetreaLegion(FortPtr fort)
{
  if( fort.isValid() )
    fort->returnSoldiers();
}

void Legion::_handleServiceEmpire(FortPtr fort)
{
  LegionTargetWindow* dlg = LegionTargetWindow::create( _d->city, ui()->rootWidget(), -1 );
  dlg->show();

  CONNECT( dlg, onSelectLocation(), fort.object(), Fort::sendExpedition );
}

}

}//end namespace gui
