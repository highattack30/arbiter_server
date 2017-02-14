#ifndef ACCOUNTDATA_H
#define ACCOUNTDATA_H

#include "DBHandler.h"
#include "typeDefs.h"
#include "serverEnums.h"

#include <memory>

class connection;

typedef struct account
{
	char username[33];
	char password[33]; //md5 + \0

	bool isGm;

	uint32
		id;

} account, *lpaccount_data;


bool account_perform_login(std::shared_ptr<connection>,sql::Connection * con , byte ticket[33], byte username[SC_PLAYER_NAME_MAX_LENGTH]);
void WINAPI account_load_client_settings(std::shared_ptr<connection> p, sql::Connection * con);

#endif // 
