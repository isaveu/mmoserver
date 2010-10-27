/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
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

#ifndef DATABASE_MANAGER_DATABASE_MANAGER_H_
#define DATABASE_MANAGER_DATABASE_MANAGER_H_

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <cstdint>
#include <list>
#include <memory>

#include "DatabaseManager/DatabaseType.h"
#include "DatabaseManager/declspec.h"

class Database;

class DBMANAGER_API DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    void process();

    Database* connect(DBType type, 
        const std::string& host, 
        uint16_t port, 
        const std::string& user, 
        const std::string& pass, 
        const std::string& dbname);

private:
    typedef std::list<std::shared_ptr<Database>> DatabaseList;
    DatabaseList database_list_;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif  // DATABASE_MANAGER_DATABASE_MANAGER_H_

