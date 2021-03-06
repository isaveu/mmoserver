/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "WaypointFactory.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "WaypointObject.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"

//=============================================================================

bool				WaypointFactory::mInsFlag    = false;
WaypointFactory*	WaypointFactory::mSingleton  = NULL;

//======================================================================================================================

WaypointFactory*	WaypointFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new WaypointFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

WaypointFactory::WaypointFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    _setupDatabindings();
}

//=============================================================================

WaypointFactory::~WaypointFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void WaypointFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case WaypointFQuery_MainData:
    {
        std::shared_ptr<WaypointObject> waypoint = _createWaypoint(result);
		LOG(info) << "WaypointFactory::handleDatabaseJobComplete Loaded waypoint id : " << waypoint->getId() ;
		// can't check waypoints on other planets in tutorial
        if (gWorldConfig->isTutorial())	{
			return;
		}

		asyncContainer->mOfCallback->handleObjectReady(waypoint);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void WaypointFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	std::stringstream sql;
	sql <<	"SELECT waypoints.waypoint_id,waypoints.owner_id,waypoints.x,waypoints.y,waypoints.z, waypoints.name,planet.name, waypoints.planet_id, "
		<<	"waypoints.active,waypoints.type FROM "	<<	mDatabase->galaxy()	<<	".waypoints INNER JOIN " <<	mDatabase->galaxy()
		<<	".planet ON (waypoints.planet_id = planet.planet_id) WHERE (waypoints.waypoint_id = " << id << ");";
    
	mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,WaypointFQuery_MainData,client), sql.str());
                               
  
}

//=============================================================================

std::shared_ptr<WaypointObject> WaypointFactory::_createWaypoint(swganh::database::DatabaseResult* result)
{
	std::shared_ptr<WaypointObject> waypoint = std::make_shared<WaypointObject>();
    
    result->getNextRow(mWaypointBinding,(void*)waypoint.get());

	waypoint->setPlanetCRC(swganh::memcrc(waypoint->planet_));
	gWorldManager->addObject(waypoint);

    return waypoint;
}

//=============================================================================

void WaypointFactory::_setupDatabindings()
{
    mWaypointBinding = mDatabase->createDataBinding(10);
    mWaypointBinding->addField(swganh::database::DFT_uint64,offsetof(WaypointObject,mId),8,0);
    mWaypointBinding->addField(swganh::database::DFT_uint64,offsetof(WaypointObject,mParentId),8,1);
    mWaypointBinding->addField(swganh::database::DFT_float,offsetof(WaypointObject,mCoords.x),4,2);
    mWaypointBinding->addField(swganh::database::DFT_float,offsetof(WaypointObject,mCoords.y),4,3);
    mWaypointBinding->addField(swganh::database::DFT_float,offsetof(WaypointObject,mCoords.z),4,4);
	mWaypointBinding->addField(swganh::database::DFT_stdu16string,offsetof(WaypointObject,mName),255,5);
    mWaypointBinding->addField(swganh::database::DFT_stdstring,offsetof(WaypointObject,planet_),255,6);
    mWaypointBinding->addField(swganh::database::DFT_uint16,offsetof(WaypointObject,planet_id_),1,7);
	mWaypointBinding->addField(swganh::database::DFT_uint8,offsetof(WaypointObject,mActive),1,8);
    mWaypointBinding->addField(swganh::database::DFT_uint8,offsetof(WaypointObject,mWPType),1,9);
}

//=============================================================================

void WaypointFactory::_destroyDatabindings()
{
    mDatabase->destroyDataBinding(mWaypointBinding);
}

//=============================================================================

