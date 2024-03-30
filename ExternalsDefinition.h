#include <tuple>

namespace GOTHIC_ENGINE
{
	namespace Externals
	{
		void __fastcall zCPar_DataStack__Clear(zCPar_DataStack* t_this, void* t_reg);
		inline HOOK Hook_zCPar_DataStack__Clear PATCH(&zCPar_DataStack::Clear, &zCPar_DataStack__Clear);

		void __fastcall oCGame__DefineExternals_Ulfi(oCGame* t_this, void* t_reg, zCParser* t_parser);
		inline HOOK Hook_oCGame__DefineExternals_Ulfi PATCH(oCGame::DefineExternals_Ulfi, &oCGame__DefineExternals_Ulfi);

		void DefineExternals()
		{		
			//Probably not needed with this way but make sure that the code is inlined
			[[msvc::flatten]]
			[] <std::size_t... Is>(std::index_sequence<Is...>)
			{
				auto Declare = [] <std::size_t... Is2>(const auto table, std::index_sequence<Is2...>)
				{
					const auto par = GetParserByEnum(table.s_parser);
					((std::tuple_element_t<Is2, typename decltype(table)::Table>::DefineExternal(par)), ...);
				};

				(Declare(NthTable<Is>{}, std::make_index_sequence<
					std::tuple_size_v<typename NthTable<Is>::Table>>{}
				), ...);

			}(std::make_index_sequence<std::to_underlying(eParser::MAX)>());

		}

		void __fastcall oCGame__DefineExternals_Ulfi(oCGame* t_this, void* t_reg, zCParser* t_parser)
		{
			Hook_oCGame__DefineExternals_Ulfi(t_this, t_reg, t_parser);
			DefineExternals();

		}
		void __fastcall zCPar_DataStack__Clear(zCPar_DataStack* t_this, void* t_reg)
		{
			Hook_zCPar_DataStack__Clear(t_this, t_reg);
			Externals::StringPool::ClearPool(t_this);
		}

	}
}

