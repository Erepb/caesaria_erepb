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

#include "advisor_emperor_window.hpp"
#include "gfx/decorator.hpp"
#include "core/gettext.hpp"
#include "pushbutton.hpp"
#include "label.hpp"
#include "game/resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "gfx/engine.hpp"
#include "groupbox.hpp"
#include "listbox.hpp"
#include "listboxitem.hpp"
#include "core/logger.hpp"
#include "city/city.hpp"
#include "city/request.hpp"
#include "core/foreach.hpp"
#include "good/goodhelper.hpp"
#include "game/gamedate.hpp"
#include "gameautopause.hpp"
#include "city/statistic.hpp"
#include "city/requestdispatcher.hpp"
#include "game/player.hpp"
#include "dialogbox.hpp"
#include "game/settings.hpp"
#include "events/fundissue.hpp"
#include "change_salary_window.hpp"
#include "city/funds.hpp"
#include "world/empire.hpp"
#include "world/emperor.hpp"
#include "city_donation_window.hpp"
#include "emperorgiftwindow.hpp"

namespace gui
{

namespace {
  Point requestButtonOffset = Point( 0, 60 );
  Size requestButtonSize = Size( 560, 40 );
}

class RequestButton : public PushButton
{
public:
  RequestButton( Widget* parent, const Point& pos, int index, city::request::RequestPtr request )
    : PushButton( parent, Rect( pos + requestButtonOffset * index, requestButtonSize), "", -1, false, PushButton::blackBorderUp )
  {
    _request = request;
    _resizeEvent();

    CONNECT( this, onClicked(), this, RequestButton::_executeRequest );
  }

  virtual void _updateTexture( ElementState state )
  {
    PushButton::_updateTexture( state );

    PictureRef& pic = _textPictureRef( state );

    Font font = Font::create( FONT_1_WHITE );

    city::request::RqGoodPtr gr = ptr_cast<city::request::RqGood>(_request);
    if( gr.isValid() )
    {
      font.draw( *pic, StringHelper::format( 0xff, "%d", gr->getQty() ), 2, 2 );

      Picture goodPicture = GoodHelper::getPicture( gr->getGoodType() );
      pic->draw( goodPicture, Point( 40, 2 ), false );

      font.draw( *pic, GoodHelper::getTypeName( gr->getGoodType() ), 60, 2 );

      int month2comply = GameDate::current().getMonthToDate( gr->getFinishedDate() );
      font.draw( *pic, StringHelper::format( 0xff, "%d %s", month2comply, _( "##rqst_month_2_comply##") ), 250, 2 );
      font.draw( *pic, gr->getDescription(), 5, pic->height() - 20 );
    }
  }

public oc3_signals:
  Signal1<city::request::RequestPtr>& onExecRequest() { return _onExecRequestSignal; }

private:
  void _acceptRequest()  { _onExecRequestSignal.emit( _request );  }

  void _executeRequest()
  {
    DialogBox* dialog = new DialogBox( parent(), Rect(), "", "##dispatch_emperor_request_question##", DialogBox::btnOk | DialogBox::btnCancel );
    CONNECT( dialog, onOk(), this, RequestButton::_acceptRequest );
    CONNECT( dialog, onOk(), dialog, DialogBox::deleteLater );
    CONNECT( dialog, onCancel(), dialog, DialogBox::deleteLater );
  }

  Signal1<city::request::RequestPtr> _onExecRequestSignal;
  city::request::RequestPtr _request;
};

class AdvisorEmperorWindow::Impl
{
public:
  PlayerCityPtr city;
  PictureRef background;
  gui::Label* lbEmperorFavour;
  gui::Label* lbEmperorFavourDesc;
  gui::Label* lbPost;
  gui::Label* lbPrimaryFunds;
  PushButton* btnSendGift;
  PushButton* btnSend2City;
  PushButton* btnChangeSalary; 
  GameAutoPause autoPause;
  bool isRequestsUpdated;

  void sendMoney( int money );
  void sendGift( int money );
  void resolveRequest( city::request::RequestPtr request );

  std::string getEmperorFavourStr()
  {
    return StringHelper::format( 0xff, "##emperor_favour_%02d##", (int)(city->getFavour() / 100.f) * 20 );
  }
};

void AdvisorEmperorWindow::_showChangeSalaryWindow()
{
  PlayerPtr pl = _d->city->player();
  ChangeSalaryWindow* dialog = new ChangeSalaryWindow( parent(), pl->salary() );
  dialog->show();

  CONNECT( dialog, onChangeSalary(), pl.object(), Player::setSalary )
}

void AdvisorEmperorWindow::_showSend2CityWindow()
{
  PlayerPtr pl = _d->city->player();
  CityDonationWindow* dialog = new CityDonationWindow( parent(), pl->money() );
  dialog->show();

  CONNECT( dialog, onSendMoney(), _d.data(), Impl::sendMoney );
}

void AdvisorEmperorWindow::_showGiftWindow()
{
  PlayerPtr pl = _d->city->player();
  EmperorGiftWindow* dialog = new EmperorGiftWindow( parent(), pl->money() );
  dialog->show();

  CONNECT( dialog, onSendGift(), _d.data(), Impl::sendGift );
}

void AdvisorEmperorWindow::_updateRequests()
{
  Rect reqsRect( Point( 32, 91 ), Size( 570, 220 ) );
  PictureDecorator::draw( *_d->background, reqsRect, PictureDecorator::blackFrame );

  List<RequestButton*> btns = findChildren<RequestButton*>();
  foreach( btn, btns )
  {
    (*btn)->deleteLater();
  }

  city::request::RequestList reqs;
  city::request::DispatcherPtr dispatcher = ptr_cast<city::request::Dispatcher>( _d->city->findService( city::request::Dispatcher::getDefaultName() ) );

  if( dispatcher.isValid() )
  {
    reqs = dispatcher->getRequests();
  }

  if( reqs.empty() )
  {
    Label* lb = new Label( this, reqsRect, _("##have_no_requests##") );
    lb->setTextAlignment( alignCenter, alignCenter );
  }
  else
  {
    foreach( request, reqs )
    {
      if( !(*request)->isDeleted() )
      {
        bool mayExec = (*request)->isReady( _d->city );
        RequestButton* btn = new RequestButton( this, reqsRect.UpperLeftCorner + Point( 5, 5 ), std::distance( request, reqs.begin() ), *request );
        btn->setTooltipText( _("##request_btn_tooltip##") );
        btn->setEnabled( mayExec );
        CONNECT(btn, onExecRequest(), _d.data(), Impl::resolveRequest );
      }
    }
  }
  _d->isRequestsUpdated = false;
}

AdvisorEmperorWindow::AdvisorEmperorWindow( PlayerCityPtr city, Widget* parent, int id )
: Widget( parent, id, Rect( 0, 0, 1, 1 ) ), _d( new Impl )
{
  _d->autoPause.activate();
  _d->city = city;
  _d->isRequestsUpdated = true;

  setGeometry( Rect( Point( (parent->width() - 640 )/2, parent->height() / 2 - 242 ),
               Size( 640, 432 ) ) );

  gui::Label* title = new gui::Label( this, Rect( 10, 10, width() - 10, 10 + 40) );
  title->setText( city->player()->getName() );
  title->setFont( Font::create( FONT_3 ) );
  title->setTextAlignment( alignCenter, alignCenter );

  _d->background.reset( Picture::create( size() ) );
  //main _d->_d->background
  PictureDecorator::draw( *_d->background, Rect( Point( 0, 0 ), size() ), PictureDecorator::whiteFrame );

  //buttons _d->_d->background  
  PictureDecorator::draw( *_d->background, Rect( 66, 325, 66 + 510, 325 + 94 ), PictureDecorator::blackFrame );
  
  _d->lbEmperorFavour = new gui::Label( this, Rect( Point( 58, 44 ), Size( 550, 20 ) ), StringHelper::format( 0xff, "%s %d", "##advemp_emperor_favour##", _d->city->getFavour() ) );
  _d->lbEmperorFavourDesc = new gui::Label( this, _d->lbEmperorFavour->getRelativeRect() + Point( 0, 20 ), _d->getEmperorFavourStr() );

  _d->lbPost = new gui::Label( this, Rect( Point( 70, 336 ), Size( 240, 26 ) ), "Post");
  _d->lbPrimaryFunds = new gui::Label( this, Rect( Point( 70, 370 ), Size( 240, 20 ) ), "PrimaryFunds 0" );

  _d->btnSendGift = new PushButton( this, Rect( Point( 322, 343), Size( 250, 20 ) ), "Send gift", -1, false, PushButton::blackBorderUp );
  _d->btnSend2City = new PushButton( this, Rect( Point( 322, 370), Size( 250, 20 ) ), "Send to city", -1, false, PushButton::blackBorderUp );
  _d->btnChangeSalary = new PushButton( this, Rect( Point( 70, 395), Size( 500, 20 ) ), "Change salary", -1, false, PushButton::blackBorderUp );  
  CONNECT( _d->btnChangeSalary, onClicked(), this, AdvisorEmperorWindow::_showChangeSalaryWindow );
  CONNECT( _d->btnSend2City, onClicked(), this, AdvisorEmperorWindow::_showSend2CityWindow );
  CONNECT( _d->btnSendGift, onClicked(), this, AdvisorEmperorWindow::_showGiftWindow );
}

void AdvisorEmperorWindow::draw( GfxEngine& painter )
{
  if( !isVisible() )
    return;

  painter.drawPicture( *_d->background, screenLeft(), screenTop() );
  if( _d->isRequestsUpdated )
  {
    _updateRequests();
  }

  Widget::draw( painter );
}

void AdvisorEmperorWindow::Impl::sendMoney( int money )
{
  city->player()->appendMoney( -money );
  events::GameEventPtr e = events::FundIssueEvent::create( city::Funds::donation, money );
  e->dispatch();
}

void AdvisorEmperorWindow::Impl::sendGift(int money)
{
  city->player()->appendMoney( -money );
  city->empire()->emperor().sendGift( city->getName(), money );
}

void AdvisorEmperorWindow::Impl::resolveRequest(city::request::RequestPtr request)
{
  if( request.isValid() )
  {
    request->exec( city );
    isRequestsUpdated = true;
  }
}

}//end namespace gui
