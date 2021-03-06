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
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com

#ifndef __CAESARIA_MENU_H_INCLUDE_
#define __CAESARIA_MENU_H_INCLUDE_

#include "widget.hpp"

#include "gfx/picture.hpp"
#include "game/enums.hpp"

namespace gui
{

class PushButton;

// this is the menu with newGame/loadGame/quit choice
class StartMenu : public Widget
{
public:
  StartMenu( Widget* parent );
  virtual ~StartMenu();

  virtual void draw(gfx::Engine &painter);
  PushButton* addButton( const std::string& caption, int id );
  void clear();
private:
  class Impl;
  ScopedPtr<Impl> _d;
};

}//end namespace gui
#endif //__CAESARIA_MENU_H_INCLUDE_
