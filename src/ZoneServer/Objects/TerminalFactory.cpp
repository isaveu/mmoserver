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

#include "TerminalFactory.h"
#include "ZoneServer/Objects/BankTerminal.h"
#include "Zoneserver/Objects/PlayerStructureTerminal.h"
#include "BazaarTerminal.h"
#include "ZoneServer/Objects/CharacterBuilderTerminal.h"
#include "CloningTerminal.h"
#include "ElevatorTerminal.h"
#include "InsuranceTerminal.h"
#include "MissionTerminal.h"
#include "ZoneServer/Objects/Object/ObjectFactoryCallback.h"
#include "ZoneServer/GameSystemManagers/Travel Manager/TravelTerminal.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool				TerminalFactory::mInsFlag    = false;
TerminalFactory*	TerminalFactory::mSingleton  = NULL;

//======================================================================================================================

TerminalFactory*	TerminalFactory::Init(swganh::app::SwganhKernel*	kernel)
{
    if(!mInsFlag)
    {
        mSingleton = new TerminalFactory(kernel);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//=============================================================================

TerminalFactory::TerminalFactory(swganh::app::SwganhKernel*	kernel) : FactoryBase(kernel)
{
    _setupDatabindings();
}

//=============================================================================

TerminalFactory::~TerminalFactory()
{
    _destroyDatabindings();

    mInsFlag = false;
    delete(mSingleton);
}

//=============================================================================

void TerminalFactory::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

    switch(asyncContainer->mQueryType)
    {
    case TFQuery_MainData:
    {
        Terminal* terminal = _createTerminal(result);

        if(terminal->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
            asyncContainer->mOfCallback->handleObjectReady(terminal,asyncContainer->mClient);
        else
        {
            switch(terminal->getTangibleType())
            {
            case TanType_ElevatorTerminal:
            case TanType_ElevatorUpTerminal:
            case TanType_ElevatorDownTerminal:
            {
                ElevatorTerminal* elTerminal = dynamic_cast<ElevatorTerminal*>(terminal);

                switch(elTerminal->mLoadState)
                {
                case LoadState_Tangible_Data:
                {
                    QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,TFQuery_ElevatorData,asyncContainer->mClient);
                    asContainer->mObject = elTerminal;

					std::stringstream sql;
					sql << "SELECT * FROM " << mDatabase->galaxy() << ".terminal_elevator_data WHERE id=" << elTerminal->getId() <<" ORDER BY direction";
                    mDatabase->executeSqlAsync(this,asContainer,sql.str());
                    
                }
                break;

                default:
                    break;
                }
            }
            break;

            default:
                break;
            }
        }
    }
    break;

    case TFQuery_ElevatorData:
    {
        ElevatorTerminal* terminal = dynamic_cast<ElevatorTerminal*>(asyncContainer->mObject);

        //uint64 count = result->getRowCount();
        //assert(count < 3 && count > 0);

        // we order by direction in select, though up is first
        if(terminal->mTanType == TanType_ElevatorUpTerminal || terminal->mTanType == TanType_ElevatorTerminal)
            result->getNextRow(mElevetorDataUpBinding, (void*)terminal);

        if(terminal->mTanType == TanType_ElevatorDownTerminal || terminal->mTanType == TanType_ElevatorTerminal)
            result->getNextRow(mElevetorDataDownBinding, (void*)terminal);

        terminal->setLoadState(LoadState_Loaded);

        asyncContainer->mOfCallback->handleObjectReady(terminal, asyncContainer->mClient);
    }
    break;

    default:
        break;
    }

    mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void TerminalFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
    mDatabase->executeSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,TFQuery_MainData,client),
                               "SELECT terminals.id, terminals.parent_id, terminals.oX, terminals.oY, terminals.oZ,terminals.oW,terminals.x,"
                               "terminals.y,terminals.z,terminals.terminal_type,terminal_types.object_string,terminal_types.name,terminal_types.file,"
                               "terminals.dataStr,terminals.dataInt1,terminals.customName"
                               " FROM %s.terminals INNER JOIN %s.terminal_types ON (terminals.terminal_type = terminal_types.id)"
                               " WHERE (terminals.id = %"PRIu64")",mDatabase->galaxy(),mDatabase->galaxy(), id);
  
}

//=============================================================================

Terminal* TerminalFactory::_createTerminal(swganh::database::DatabaseResult* result)
{
    if (!result->getRowCount()) {
    	return nullptr;
    }

    Terminal*		terminal(0);
    TangibleType	tanType;

    swganh::database::DataBinding* typeBinding = mDatabase->createDataBinding(1);
    typeBinding->addField(swganh::database::DFT_uint32, 0, 4, 9);

    result->getNextRow(typeBinding, &tanType);
    result->resetRowIndex();

    mDatabase->destroyDataBinding(typeBinding);

    switch(tanType)
    {
    case TanType_HQTerminal:
    case TanType_SpaceTerminal:
    case TanType_NewsNetTerminal:
    case TanType_BestineQuest1Terminal:
    case TanType_BestineQuest2Terminal:
    case TanType_BallotBoxTerminal:
    case TanType_BountyDroidTerminal:
    case TanType_GuildTerminal:
    case TanType_MissionStatueTerminal:
    case TanType_NewbieClothingTerminal:
    case TanType_NewbieFoodTerminal:
    case TanType_NewbieInstrumentTerminal:
    case TanType_NewbieMedicineTerminal:
    case TanType_NewbieToolTerminal:
    case TanType_HQRebelTerminal:
    case TanType_HQImperialTerminal:
    case TanType_PMRegisterTerminal:
    case TanType_SKillTerminal:
    case TanType_CityTerminal:
    case TanType_PlayerStructureNoSnapTerm:
    case TanType_CityVoteTerminal:
    case TanType_PlayerStructureNoSnapMini:
    case TanType_NymCaveTerminal:
    case TanType_CommandConsoleTerminal:
    case TanType_GeoBunkerTerminal:
    case TanType_BestineQuests3:
    case TanType_HQTurrentControlTermainl:
    case TanType_ImageDesignTerminal:
    case TanType_WaterPressureTerminal:
    case TanType_Light_Enc_VotingTerminal:
    case TanType_Dark_Enc_ChallengeTerminal:
    case TanType_Dark_Enc_VotingTerminal:
    case TanType_ShipInteriorSecurity1:
    case TanType_POBShipTerminal:
    case TanType_Light_Enc_ChallengeTerminal:
    case TanType_CampTerminal:
    {
        terminal = new Terminal();
        terminal->setTangibleType(tanType);

        result->getNextRow(mTerminalBinding,(void*)terminal);


        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_PlayerStructureTerminal:
    {
        terminal = new PlayerStructureTerminal();
        terminal->setTangibleType(tanType);

      

        result->getNextRow(mTerminalBinding,(void*)terminal);

        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_BankTerminal:
    {
        terminal = new BankTerminal();
        terminal->setTangibleType(tanType);
        result->getNextRow(mTerminalBinding,(void*)terminal);
        terminal->setLoadState(LoadState_Loaded);

    }
    break;

    case TanType_CharacterBuilderTerminal:
    {
        terminal = new CharacterBuilderTerminal();
        terminal->setTangibleType(tanType);
        result->getNextRow(mTerminalBinding,(void*)terminal);
        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_BazaarTerminal:
    {
        terminal = new BazaarTerminal();
        terminal->setTangibleType(tanType);
        result->getNextRow(mTerminalBinding,(void*)terminal);
        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_CloningTerminal:
    {
        terminal = new CloningTerminal();
        terminal->setTangibleType(tanType);
        result->getNextRow(mTerminalBinding,(void*)terminal);
        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_InsuranceTerminal:
    {
        terminal = new InsuranceTerminal();
        terminal->setTangibleType(tanType);
        result->getNextRow(mTerminalBinding,(void*)terminal);
        terminal->setLoadState(LoadState_Loaded);
    }
    break;


    case TanType_MissionTerminal:
    case TanType_EntertainerMissionTerminal:
    case TanType_BountyMissionTerminal:
    case TanType_RebelMissionTerminal:
    case TanType_ImperialMissionTerminal:
    case TanType_ScoutMissionTerminal:
    case TanType_ArtisanMissionTerminal:
    case TanType_MissionNewbieTerminal:
    {
        terminal = new MissionTerminal();
        terminal->setTangibleType(tanType);

        result->getNextRow(mTerminalBinding,(void*)terminal);

        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_TravelTerminal:
    {
        terminal = new TravelTerminal();
        terminal->setTangibleType(tanType);

        result->getNextRow(mTravelMainDataBinding,(void*)terminal);

        terminal->setLoadState(LoadState_Loaded);
    }
    break;

    case TanType_ElevatorUpTerminal:
    case TanType_ElevatorDownTerminal:
    case TanType_ElevatorTerminal:
    {
        terminal = new ElevatorTerminal();
        terminal->setTangibleType(tanType);

        result->getNextRow(mElevatorMainDataBinding,(void*)terminal);

        //((ElevatorTerminal*)(terminal))->prepareRadialMenu();
        terminal->setLoadState(LoadState_Tangible_Data);
    }
    break;

    default:
    {
        DLOG(info) << "TerminalFactory::_createTerminal: unknown eType: " << tanType;

        terminal = new Terminal();
        terminal->setTangibleType(tanType);

        
        result->getNextRow(mTerminalBinding,(void*)terminal);

        terminal->setLoadState(LoadState_Loaded);
    }
    break;
    }

    terminal->mTypeOptions = 0x108;

    return terminal;
}

//=============================================================================

void TerminalFactory::_setupDatabindings()
{
    
    // travel terminal
    mTravelMainDataBinding = mDatabase->createDataBinding(15);
    mTravelMainDataBinding->addField(swganh::database::DFT_uint64,offsetof(TravelTerminal,mId),8,0);
    mTravelMainDataBinding->addField(swganh::database::DFT_uint64,offsetof(TravelTerminal,mParentId),8,1);
	mTravelMainDataBinding->addField(swganh::database::DFT_stdstring,offsetof(TravelTerminal,template_string_),256,10);
    mTravelMainDataBinding->addField(swganh::database::DFT_bstring,offsetof(TravelTerminal,mName),64,11);
    mTravelMainDataBinding->addField(swganh::database::DFT_bstring,offsetof(TravelTerminal,mNameFile),64,12);
    mTravelMainDataBinding->addField(swganh::database::DFT_bstring,offsetof(TravelTerminal,mPositionDescriptor),128,13);
    mTravelMainDataBinding->addField(swganh::database::DFT_uint32,offsetof(TravelTerminal,mPortType),4,14);
    mTravelMainDataBinding->addField(swganh::database::DFT_stdu16string,offsetof(TravelTerminal,custom_name_),256,15);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mDirection.x),4,2);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mDirection.y),4,3);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mDirection.z),4,4);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mDirection.w),4,5);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mPosition.x),4,6);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mPosition.y),4,7);
    mTravelMainDataBinding->addField(swganh::database::DFT_float,offsetof(TravelTerminal,mPosition.z),4,8);


    
    mTerminalBinding= mDatabase->createDataBinding(13);
    mTerminalBinding->addField(swganh::database::DFT_uint64,offsetof(Terminal,mId),8,0);
    mTerminalBinding->addField(swganh::database::DFT_uint64,offsetof(Terminal,mParentId),8,1);
	mTerminalBinding->addField(swganh::database::DFT_stdstring,offsetof(Terminal,template_string_),256,10);
    mTerminalBinding->addField(swganh::database::DFT_bstring,offsetof(Terminal,mName),64,11);
    mTerminalBinding->addField(swganh::database::DFT_bstring,offsetof(Terminal,mNameFile),64,12);
    mTerminalBinding->addField(swganh::database::DFT_stdu16string,offsetof(Terminal,custom_name_),256,15);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mDirection.x),4,2);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mDirection.y),4,3);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mDirection.z),4,4);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mDirection.w),4,5);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mPosition.x),4,6);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mPosition.y),4,7);
    mTerminalBinding->addField(swganh::database::DFT_float,offsetof(Terminal,mPosition.z),4,8);
	mTerminalBinding->addField(swganh::database::DFT_uint32,offsetof(Terminal,mTerminalType),4,9);

    // elevator terminal main data
    mElevatorMainDataBinding = mDatabase->createDataBinding(13);
    mElevatorMainDataBinding->addField(swganh::database::DFT_uint64,offsetof(ElevatorTerminal,mId),8,0);
    mElevatorMainDataBinding->addField(swganh::database::DFT_uint64,offsetof(ElevatorTerminal,mParentId),8,1);
	mElevatorMainDataBinding->addField(swganh::database::DFT_stdstring,offsetof(ElevatorTerminal,template_string_),256,10);
    mElevatorMainDataBinding->addField(swganh::database::DFT_bstring,offsetof(ElevatorTerminal,mName),64,11);
    mElevatorMainDataBinding->addField(swganh::database::DFT_bstring,offsetof(ElevatorTerminal,mNameFile),64,12);
	mElevatorMainDataBinding->addField(swganh::database::DFT_stdu16string,offsetof(ElevatorTerminal,custom_name_),256,15);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDirection.x),4,2);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDirection.y),4,3);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDirection.z),4,4);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDirection.w),4,5);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mPosition.x),4,6);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mPosition.y),4,7);
    mElevatorMainDataBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mPosition.z),4,8);

    // elevator terminal upper destination
    mElevetorDataUpBinding = mDatabase->createDataBinding(9);
    mElevetorDataUpBinding->addField(swganh::database::DFT_uint64,offsetof(ElevatorTerminal,mDstCellUp),8,1);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirUp.x),4,2);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirUp.y),4,3);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirUp.z),4,4);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirUp.w),4,5);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosUp.x),4,6);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosUp.y),4,7);
    mElevetorDataUpBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosUp.z),4,8);
    mElevetorDataUpBinding->addField(swganh::database::DFT_uint32,offsetof(ElevatorTerminal,mEffectUp),4,9);

    // elevator terminal lower destination
    mElevetorDataDownBinding = mDatabase->createDataBinding(9);
    mElevetorDataDownBinding->addField(swganh::database::DFT_uint64,offsetof(ElevatorTerminal,mDstCellDown),8,1);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirDown.x),4,2);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirDown.y),4,3);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirDown.z),4,4);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstDirDown.w),4,5);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosDown.x),4,6);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosDown.y),4,7);
    mElevetorDataDownBinding->addField(swganh::database::DFT_float,offsetof(ElevatorTerminal,mDstPosDown.z),4,8);
    mElevetorDataDownBinding->addField(swganh::database::DFT_uint32,offsetof(ElevatorTerminal,mEffectDown),4,9);
}

//=============================================================================

void TerminalFactory::_destroyDatabindings()
{
    
    mDatabase->destroyDataBinding(mTravelMainDataBinding);
    mDatabase->destroyDataBinding(mTerminalBinding);
    
    mDatabase->destroyDataBinding(mElevatorMainDataBinding);
    mDatabase->destroyDataBinding(mElevetorDataUpBinding);
    mDatabase->destroyDataBinding(mElevetorDataDownBinding);
}

//=============================================================================
