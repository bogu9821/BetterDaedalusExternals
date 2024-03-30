# BetterDaedalusExternals
Improved method for creating C++ externals for the Daedalus scripting language. Requires C++23.<br>
Thanks to C++ templates and constexpr, you no longer need to define external types yourself or call parser methods to handle arguments from the stack or return values. <br>
This simplifies code writing and reduces the errors.
Compile-time errors occur if you attempt to redefine an external.<br>
There are many errors in the engine with returning values in void functions, which may cause stack overflow.<br>
Additionally, zSTRING externals have a pooled return value and you don't have to return static variables.<br>
zSTRINGs can be also taken from the stack as const references, eliminating the need for copying.

# Example
To ensure everything works properly, make sure to include BetterExternals.h in your project and place your external header before ExternalsDefinition.h in that file due to constexpr checks.

BetterExternals.h:
```cpp
#include "ExternalUtility.h"
#include "Externals.h"

//include your files here
//e.g.
//#include "MyExternals.h"

#include "ExternalsDefinition.h"
```
Your code e.g. in MyExternals.h
```c++  
    int EXT_RGBA(const int r, const int g, const int b, const int a)
    {
        const zCOLOR color
        {
            static_cast<unsigned char>(r),
            static_cast<unsigned char>(g),
            static_cast<unsigned char>(b),
            static_cast<unsigned char>(a)
        };

        return static_cast<int>(color.dword);
    }

    int EXT_Log_GetTopicStatus(const zSTRING& t_topicName)
    {
        auto list = oCLogManager::GetLogManager().m_lstTopics.next;
        while (list)
        {
            auto const log = list->data;
            if (log->m_strDescription == t_topicName)
            {
                return static_cast<int>(log->m_enuStatus);
            }

            list = list->next;
        }

        return static_cast<int>(oCLogTopic::zELogTopicStatus_Free);
    }

    zSTRING GetHeroName()
    {
        return player->name[0];
    }
    
    ExternalDefinition(eParser::GAME, 
        MakeDaedalusExternal(EXT_Log_GetTopicStatus),
        MakeDaedalusExternalWithCondition(GetHeroName, []() -> bool { return (rand() % 2) == 1; })
    )

    ExternalDefinition(eParser::MENU, MakeDaedalusExternal(EXT_RGBA))
```

# How it looked before
Here's how you typically add externals without using my library:
```cpp
//zCParser::DefineExternal accepts only int function pointers
int Log_GetTopicStatus()
{
	zCParser* par = zCParser::GetParser();

	//copy zSTRING from the stack
	zSTRING logName;
	par->GetParameter(logName);

	auto list = oCLogManager::GetLogManager().m_lstTopics.next;
	while (list)
	{
		auto const log = list->data;
		if (log->m_strDescription == logName)
		{
			par->SetReturn(static_cast<int>(log->m_enuStatus));
			return 0;
		}

		list = list->next;
	}

	//you have to remember to call zCParser::SetReturn before every return of a C++ function.
	par->SetReturn(static_cast<int>(oCLogTopic::zELogTopicStatus_Free));
	return 0;
};

int Npc_DoTakeItem()
{
	auto const par = zCParser::GetParser();

	oCItem* item;
	oCNpc* npc;

	//all arguments must be popped in a reverse order
	item = static_cast<oCItem*>(par->GetInstance());
	npc = static_cast<oCNpc*>(par->GetInstance());

	//you have to pop every argument before return
	if (!item || !npc)
	{
		return 0;
	}

	npc->DoTakeVob(item);

	return 0;
}

//hooked engine function
void Game_DefineExternals()
{
    //for every parser you have to define name, all script function types and return value
    //it uses C va args so at the end must be 0
    //if you will pop different type in your external you will get a crash
    parser->DefineExternal("Log_GetTopicStatus", Log_GetTopicStatus, zPAR_TYPE_INT, zPAR_TYPE_STRING, 0);

    if(condition)
    {
        menuParser->DefineExternal("Npc_DoTakeItem", Npc_DoTakeItem, zPAR_TYPE_VOID, zPAR_TYPE_INSTANCE, zPAR_TYPE_INSTANCE, 0);
    }
}

```
