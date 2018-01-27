
#include "StdInc.h"
#include "Main.h"

using namespace Galactic3D;

// The table name, so Module.HelloWorld.anything would be where the functions are
REGISTER_MODULE("MySQL");

// Returns a new class instance of TestClass
static bool FunctionTest(IScriptState* pState, Sint32 argc, void* pUser)
{
	CModule* pModule = (CModule*)pUser;

	Referenceable* pReferenceable = Reflection::New(nullptr, pModule->m_pTestClass);

	pState->ReturnReferenceable(pReferenceable);

	pReferenceable->Release();

	return true;
}

// Gets integer argument, if its more than 10 throws an error otherwise returns true...
static bool FunctionTest2(IScriptState* pState, Sint32 argc, void* pUser)
{
	CModule* pModule = (CModule*)pUser;

	Uint32 Number;
	if (!pState->CheckNumber(0, &Number))
		return false;

	if (Number > 10)
		return pState->Error("Number is more than 10!");

	pState->ReturnBoolean(true);

	return true;
}

void* SDLCALL MyRegisterModule(ReflectedNamespace* pNamespace)
{
	CModule* pModule = new CModule;

	static ReflectedClassDecl TestClass("TestClass");
	pModule->m_pTestClass = pNamespace->NewClass(&TestClass);

	static ScriptFunction SFTest = { "test","",FunctionTest };
	pNamespace->RegisterFunction(&SFTest, pModule);

	static ScriptFunction SFTest2 = { "test2","",FunctionTest2 };
	pNamespace->RegisterFunction(&SFTest2, pModule);

	return pModule;
}

void SDLCALL MyUnregisterModule(void* pUser)
{
	delete (CModule*)pUser;
}
