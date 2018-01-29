
#include "StdInc.h"
#include "Main.h"

#include <mysql.h>

using namespace Galactic3D;

REGISTER_MODULE("MySQL");

static bool MYSQL_Connect(IScriptState* pState, Sint32 argc, void* pUser) {
	CModule* pModule = (CModule*)pUser;

	const char *szHostname = pState->CheckString(0);
	const char *szUsername = pState->CheckString(1);
	const char *szPassword = pState->CheckString(2);
	const char *szDB = pState->CheckString(3);

	if (!szHostname || !szUsername || !szPassword || !szDB) return false;

	int iPort = 0;
	if (!pState->CheckNumber(4, &iPort)) return false;

	if (strcmp(szHostname, "") == 0) {
		printf("[mysql_connect] warning: hostname is empty\n");
	}

	if (strcmp(szUsername, "") == 0) {
		printf("[mysql_connect] warning: username is empty\n");
	}

	if (strcmp(szPassword, "") == 0) {
		printf("[mysql_connect] warning: password is empty\n");
	}

	if (strcmp(szDB, "") == 0) {
		printf("[mysql_connect] warning: database is empty\n");
	}

	MYSQL *mysql = 0;
	if (!(mysql = mysql_init(NULL))) {
		pState->ReturnNull();

		pState->Error("[mysql_connect] error: MySQL initialization failed");

		return false;
	}

	if (!mysql_real_connect(mysql, szHostname, szUsername, szPassword, szDB, iPort, 0, 0)) {
		mysql_close(mysql);

		pState->ReturnNull();

		pState->Error("[mysql_connect] error: %s (%i)", mysql_error(mysql), mysql_errno(mysql));

		return false;
	}

	Referenceable* pRef = Reflection::New(0, pModule->m_pMySQL);
	pRef->SetPrivate(mysql);

	pState->ReturnReferenceable(pRef);
	
	pRef->Release();

	return true;
}

static bool MYSQL_Close(IScriptState* pState, Sint32 argc, void* pUser) {
	CModule* pModule = (CModule*)pUser;
	MYSQL *mysql = 0;
	Referenceable *pRef = 0;

	if (!pState->CheckClass(pModule->m_pMySQL, 0, false, &pRef)) return false;

	mysql = (MYSQL*)pRef->GetPrivate();

	if (!mysql) return false;

	mysql_close(mysql);

	pRef->SetPrivate(nullptr);

	pState->ReturnBoolean(true);

	return true;
}

static bool MYSQL_Query(IScriptState* pState, Sint32 argc, void* pUser) {
	CModule* pModule = (CModule*)pUser;
	MYSQL *mysql = 0;

	Referenceable *pRef = 0;

	if (!pState->CheckClass(pModule->m_pMySQL, 0, false, &pRef)) return false;

	mysql = (MYSQL*)pRef->GetPrivate();

	if (!mysql) return false;

	const char *szQuery = pState->CheckString(1);
	if (!szQuery) return false;

	if (mysql_query(mysql, szQuery)) {
		pState->Error("[mysql_query] error: %s (%i)", mysql_error(mysql), mysql_errno(mysql));

		pState->ReturnBoolean(false);

		return false;
	}

	MYSQL_RES *pResult = 0;
	if (!(pResult = mysql_store_result(mysql))) {
		pState->ReturnNull();

		return true;
	}

	Referenceable* pResultRef = Reflection::New(0, pModule->m_pQuery);
	pResultRef->SetPrivate(pResult);

	pState->ReturnReferenceable(pResultRef);

	pResultRef->Release();

	return true;
}

static bool MYSQL_FetchAssoc(IScriptState* pState, Sint32 argc, void* pUser) {
	CModule* pModule = (CModule*)pUser;

	MYSQL_RES* pResult = 0;
	MYSQL_ROW pRow = 0;
	unsigned int ui = 0;

	Referenceable *pRef = 0;

	if (!pState->CheckClass(pModule->m_pQuery, 0, false, &pRef)) return false;

	pResult = (MYSQL_RES*)pRef->GetPrivate();

	if (!pResult) return false;

	pRow = mysql_fetch_row(pResult);

	if (!pRow) return false;
	
	unsigned int uiColumns = mysql_num_fields(pResult);

	if (!uiColumns) return false;

	//sq->newtable( v );

	mysql_field_seek(pResult, 0);

	int field_i = 0, field_f = 0.0f;
	MYSQL_FIELD* pField = 0;
	for (unsigned int ui = 0; ui < uiColumns; ui++) {
		pField = mysql_fetch_field(pResult);

		//sq->pushstring( v, pField->name, pField->name_length );

		if (!pField || !pRow[ui]) continue;

		switch (pField->type)
		{
		case MYSQL_TYPE_TINY:
		case MYSQL_TYPE_SHORT:
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_LONGLONG:
		case MYSQL_TYPE_INT24:
		case MYSQL_TYPE_YEAR:
		case MYSQL_TYPE_BIT:

			field_i = (int)(atoi(pRow[ui]));
			//sq->pushinteger(v, i);
			//sq->arrayappend(v, -2);

			break;

		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_FLOAT:
		case MYSQL_TYPE_DOUBLE:

			field_f = (float)(atof(pRow[ui]));
			//sq->pushfloat(v, f);
			//sq->arrayappend(v, -2);

			break;

		case MYSQL_TYPE_NULL:

			//sq->pushnull(v);
			//sq->arrayappend(v, -2);

			break;

		default:

			//sq->pushstring(v, pRow[ui], -1);
			//sq->arrayappend(v, -2);

			break;
		}
	}
}

// ---------------------------------------------------------------------------------

void* SDLCALL MyRegisterModule(ReflectedNamespace* pNamespace) {
	CModule* pModule = new CModule;

	static ReflectedClassDecl MySQLDecl("MySQL"), MySQLQuery("MySQLQuery");
	pModule->m_pMySQL = pNamespace->NewClass(&MySQLDecl);
	pModule->m_pQuery = pNamespace->NewClass(&MySQLQuery);

	static ScriptFunction SFConnect = { "Connect", "ssss|i", MYSQL_Connect };
	pNamespace->RegisterFunction(&SFConnect, pModule);

	static ScriptFunction SFClose = { "Close", "x", MYSQL_Close };
	pNamespace->RegisterFunction(&SFClose, pModule);

	static ScriptFunction SFQuery = { "Query", "xs", MYSQL_Query };
	pNamespace->RegisterFunction(&SFQuery, pModule);

	static ScriptFunction SFFetchAssoc = { "FetchAssoc", "x", MYSQL_FetchAssoc };
	pNamespace->RegisterFunction(&SFFetchAssoc, pModule);

	return pModule;
}

void SDLCALL MyUnregisterModule(void* pUser) {
	delete (CModule*)pUser;
}
