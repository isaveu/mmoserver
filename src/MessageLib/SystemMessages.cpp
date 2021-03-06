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

#include "MessageLib.h"

#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/PlayerStructure.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"



//======================================================================================================================

void MessageLib::sendBanktipMail(PlayerObject* playerObject, PlayerObject* targetObject, uint32 amount)
{

	CreatureObject* creature = playerObject->GetCreature();
	CreatureObject* target_creature = targetObject->GetCreature();

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("base_player","prose_wire_mail_target");
    aMS->addDI(amount);
    aMS->addTO(creature->getFirstName());
    aMS->addTextModule();


    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
    mMessageFactory->addUint64(creature->getId());
    mMessageFactory->addUint64(creature->getId());
    mMessageFactory->addString(target_creature->getFirstName());
    mMessageFactory->addString(BString("@base_player:wire_mail_subject"));
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(aMS->assemble());
    delete aMS;


    Message* newMessage = mMessageFactory->EndMessage();
    playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6);



    aMS = new atMacroString();

    aMS->addMBstf("base_player","prose_wire_mail_self");
    aMS->addDI(amount);
    aMS->addTO(target_creature->getFirstName());
    aMS->addTextModule();


    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
    mMessageFactory->addUint64(target_creature->getId());
    mMessageFactory->addUint64(target_creature->getId());
    mMessageFactory->addString(creature->getFirstName());
    mMessageFactory->addString(BString("@base_player:wire_mail_subject"));
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(aMS->assemble());
    delete aMS;


    newMessage = mMessageFactory->EndMessage();
    playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6);

}


//======================================================================================================================

void MessageLib::sendBoughtInstantMail(PlayerObject* newOwner, BString ItemName, BString SellerName, uint32 Credits, BString planet, BString region, int32 mX, int32 mY)
{

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("auction","buyer_success");
    aMS->addTO(ItemName);
    aMS->addTT(SellerName);
    aMS->addDI(Credits);
    aMS->addTextModule();


    aMS->addMBstf("auction","buyer_success_location");
    aMS->addTT(region);
    planet.toUpperFirst();
    aMS->addTO(planet);
    aMS->addTextModule();

    planet.toLowerFirst();
    aMS->setPlanetString(planet);
    aMS->setWP(static_cast<float>(mX), static_cast<float>(mY), 0, ItemName);
    aMS->addWaypoint();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
	mMessageFactory->addUint64(newOwner->GetCreature()->getId());
    mMessageFactory->addUint64(newOwner->GetCreature()->getId());
    mMessageFactory->addString(BString("auctioner"));
    mMessageFactory->addString(BString("@auction:subject_auction_buyer"));
    mMessageFactory->addUint32(0);
    BString attachment = aMS->assemble();
    mMessageFactory->addString(attachment);

    Message* newMessage = mMessageFactory->EndMessage();


    newOwner->getClient()->SendChannelA(newMessage, newOwner->getAccountId(), CR_Chat, 6);

    delete aMS;

}

//======================================================================================================================

void MessageLib::sendSoldInstantMail(uint64 oldOwner, PlayerObject* newOwner, BString ItemName, uint32 Credits, BString planet, BString region)
{
    //seller_success       Your auction of %TO has been sold to %TT for %DI credits

    atMacroString* aMS = new atMacroString();

	

    aMS->addMBstf("auction","seller_success");
    aMS->addTO(ItemName);
	aMS->addTT(newOwner->GetCreature()->getFirstName().c_str());
    aMS->addDI(Credits);
    aMS->addTextModule();

    aMS->addMBstf("auction","seller_success_location");
    aMS->addTT(region);
    planet.toUpperFirst();
    aMS->addTO(planet);
    aMS->addTextModule();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
    mMessageFactory->addUint64(oldOwner);
    mMessageFactory->addUint64(oldOwner);
    mMessageFactory->addString(BString("auctioner"));
    mMessageFactory->addString(BString("@auction:subject_instant_seller"));
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(aMS->assemble());
    delete aMS;

    Message* newMessage = mMessageFactory->EndMessage();
    newOwner->getClient()->SendChannelA(newMessage, newOwner->getAccountId(), CR_Chat, 6);

}

void MessageLib::sendNewbieMail(PlayerObject* playerObject, BString subject, BString bodyDir, BString bodyStr)
{
    atMacroString* aMS = new atMacroString();

    aMS->addMBstf(bodyDir, bodyStr);
    aMS->addTextModule();
    //needed this extra one for some reason
    //aMS->addTextModule();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
    mMessageFactory->addUint64(playerObject->getId());
    mMessageFactory->addUint64(playerObject->getId());
    mMessageFactory->addString("Star Wars Galaxies");
    mMessageFactory->addString(subject);
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(aMS->assemble());
    delete aMS;

    Message* newMessage = mMessageFactory->EndMessage();
    playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6);
}

//======================================================================================================================

void MessageLib::sendConstructionComplete(PlayerObject* playerObject, PlayerStructure* structure)
{

    atMacroString* aMS = new atMacroString();

    aMS->addMBstf("player_structure","construction_complete");
    aMS->addDI(playerObject->getLots());
    aMS->addTOstf(structure->getNameFile(),structure->getName());
    aMS->addTextModule();

    BString planet;
    planet = gWorldManager->getPlanetNameThis();
    planet.toLowerFirst();

    BString wText = "";
    std::string name = std::string(structure->getCustomName().begin(), structure->getCustomName().end());
    
    wText << name.c_str();

    if(!structure->getCustomName().length())    {
        wText <<"@"<<structure->getNameFile().getAnsi()<<":"<<structure->getName().getAnsi();
    }

    aMS->setPlanetString(planet);
    aMS->setWP(static_cast<float>(structure->mPosition.x), static_cast<float>(structure->mPosition.y), 0, wText);
    aMS->addWaypoint();


    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opIsmSendSystemMailMessage);
    mMessageFactory->addUint64(playerObject->getId());
    mMessageFactory->addUint64(playerObject->getId());
    //mMessageFactory->addString(targetObject->getFirstName());
    mMessageFactory->addString(BString("@player_structure:construction_complete_sender"));
    mMessageFactory->addString(BString("@player_structure:construction_complete_subject"));
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(aMS->assemble());
    delete aMS;


    Message* newMessage = mMessageFactory->EndMessage();
    playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6);





}
