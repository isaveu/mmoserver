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

#ifndef ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H
#define ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <cstdint>
#include <memory>
#include <string>

#include "DatabaseManager/DatabaseImplementation.h"
#include "DatabaseManager/declspec.h"

namespace sql {
    class Connection;
    class ResultSet;
    class Statement;
}

class DataBinding;
class DatabaseResult;

class DBMANAGER_API DatabaseImplementationMySql : public DatabaseImplementation {
public:
    DatabaseImplementationMySql(const std::string& host, uint16_t port, const std::string& user, const std::string& pass, const std::string& schema);
    virtual ~DatabaseImplementationMySql();

    virtual DatabaseResult* executeSql(const char* sql, bool procedure = false);
    virtual DatabaseWorkerThread* destroyResult(DatabaseResult* result);

    virtual void getNextRow(DatabaseResult* result, DataBinding* binding, void* object) const;
    virtual void resetRowIndex(DatabaseResult* result, uint64_t index = 0) const;

    virtual uint32_t escapeString(char* target, const char* source, uint32_t length);

private:
    void processFieldBinding_(std::unique_ptr<sql::ResultSet>& result, DataBinding* binding, uint32_t field_id, void* object) const;

    std::unique_ptr<sql::Connection> connection_;
    std::unique_ptr<sql::Statement> statement_;
};

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#endif // ANH_DATABASEMANAGER_DATABASEIMPLEMENTATIONMYSQL_H
