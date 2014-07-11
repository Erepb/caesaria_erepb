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
// Copyright 2012-2013 Dalerank, dalerankn8@gmail.com

#include "dialogbox.hpp"
#include "gfx/picture.hpp"
#include "gfx/decorator.hpp"
#include "gui/label.hpp"
#include "texturedbutton.hpp"
#include "core/event.hpp"
#include "gfx/engine.hpp"

using namespace gfx;

namespace gui
{

namespace {
  int okBtnPicId = 239;
  int cancelBtnPicId = 243;
}

class DialogBox::Impl
{
public:
  PictureRef background;

oc3_signals public:
  Signal1<int> onResultSignal;
  Signal0<> onOkSignal;
  Signal0<> onCancelSignal;
  Signal0<> onNeverSignal;
};

DialogBox::DialogBox( Widget* parent, const Rect& rectangle, const std::string& title, 
                      const std::string& text, int buttons )
                      : Widget( parent, -1, rectangle ), _d( new Impl )
{
  if( rectangle.size() == Size( 0, 0 ) )
  {
    setGeometry( Rect( 0, 0, 480, 160 ) );
    setCenter( parent->center() );
  }

  _d->background.reset( Picture::create( size() ) );
  PictureDecorator::draw( *_d->background, Rect( Point( 0, 0 ), size() ), PictureDecorator::whiteFrame );
  
  Label* lbTitle = new Label( this, Rect( 10, 10, width() - 10, 10 + 40), title );
  lbTitle->setFont( Font::create( FONT_3 ) );
  lbTitle->setTextAlignment( align::center, align::center );

  Label* lbText = new Label( this, Rect( 10, 55, width() - 10, 55 + 55 ), text );
  lbText->setTextAlignment( align::center, align::center );

  if( (buttons == btnOk) || (buttons == btnCancel) )
  {
    new TexturedButton( this, Point( width() / 2 - 20, height() - 50),
                        Size( 39, 26 ), buttons,
                        buttons == btnOk ? okBtnPicId : cancelBtnPicId );
  }
  else if( buttons & (btnOk | btnCancel) )
  {
    new TexturedButton( this, Point( width() / 2 - 24 - 16, height() - 50),
                        Size( 39, 26 ), btnOk, okBtnPicId );
    new TexturedButton( this, Point( width() / 2 + 16, height() - 50 ),
                        Size( 39, 26 ), btnCancel, cancelBtnPicId );
  }

  if( buttons & btnNever )
  {
    new TexturedButton( this, Point( width() - 24 - 16, height() - 50),
                        Size( 39, 26 ), btnNever, cancelBtnPicId );

  }
}

Signal1<int>& DialogBox::onResult()
{
  return _d->onResultSignal;
}

bool DialogBox::onEvent( const NEvent& event )
{
  if( event.EventType == sEventGui && event.gui.type == guiButtonClicked )
  {
    int id = event.gui.caller->getID();
    oc3_emit _d->onResultSignal( id );

    switch( id )
    {
    case btnOk: oc3_emit _d->onOkSignal(); break;
    case btnCancel: oc3_emit _d->onCancelSignal(); break;
    case btnNever: oc3_emit _d->onNeverSignal(); break;
    }

    return true;
  }

  return Widget::onEvent( event );
}

Signal0<>& DialogBox::onOk() {  return _d->onOkSignal;}
Signal0<>& DialogBox::onCancel(){  return _d->onCancelSignal;}
Signal0<>& DialogBox::onNever() { return _d->onNeverSignal; }

void DialogBox::draw(gfx::Engine& painter )
{
  if( !isVisible() )
  {
    return;
  }

  if( _d->background )
  {
    painter.draw( *_d->background, screenLeft(), screenTop() );
  }

  Widget::draw( painter );
}

}//end namespace gui
