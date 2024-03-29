# BetterDaedalusExternals
Improvements way to create c++ externals for Daedalus scripting language. Requires C++23.

# Example
Include BetterExternals.h in your project and add to BetterExternals.h your externals header before ExternalsDefinition.h

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
