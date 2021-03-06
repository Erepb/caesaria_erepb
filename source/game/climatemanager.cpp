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

#include "climatemanager.hpp"
#include "core/saveadapter.hpp"
#include "vfs/filesystem.hpp"
#include "gfx/loader.hpp"
#include "resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "gfx/picture_bank.hpp"
#include "core/logger.hpp"
#include "vfs/entries.hpp"
#include "resourceloader.hpp"

using namespace vfs;

/*void __appendRange( const std::string& rc, int start, int stop, StringArray& ar )
{
  for( int index=start; index <= stop; index++ )
  {
    ar << StringHelper::format( 0xff, "%s_%05d", rc.c_str(), index );
  }
}*/

void ClimateManager::initialize(ClimateType climate)
{
  VariantMap climateArchives = SaveAdapter::load( ":/climate.model" );

  std::string optName;
  if( climate == city::climate::central ) { optName = "central"; }
  else if( climate == city::climate::northen )  { optName = "north"; }
  else if( climate == city::climate::desert ) { optName = "south"; }

  Path archivePath = climateArchives.get( optName ).toString();
  ArchivePtr archive = FileSystem::instance().mountArchive( archivePath );

  if( archive.isNull() )
  {
    Logger::warning( "ClimateManager: can't load file " + archivePath.toString() );
    return;
  }

  ResourceLoader rc;
  NFile atlasInfo = archive->createAndOpenFile( "info" );
  if( atlasInfo.isOpen() )
  {
    rc.loadAtlases( atlasInfo, false );
  }
  else
  {
    rc.loadFiles( archive );
  }
}
