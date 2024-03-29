#include <tuple>

namespace GOTHIC_ENGINE
{
	namespace Externals
	{
		void __fastcall zCPar_DataStack__Clear(zCPar_DataStack* t_this, void* t_reg);
		inline HOOK Hook_zCPar_DataStack__Clear PATCH(&zCPar_DataStack::Clear, &zCPar_DataStack__Clear);

		void __fastcall oCGame__DefineExternals_Ulfi(oCGame* t_this, void* t_reg, zCParser* t_parser);
		inline HOOK Hook_oCGame__DefineExternals_Ulfi PATCH(oCGame::DefineExternals_Ulfi, &oCGame__DefineExternals_Ulfi);

		template<size_t N>
		using NthTable = ExternalTable<static_cast<eParser>(N)>;

		template<typename T1, typename T2>
		constexpr bool ExternalSameAs()
		{
			if (!std::is_same_v<T1, T2>)
			{
				return false;
			}

			return T1::s_name.Data() == T2::s_name.Data();
		}

		template<typename T, typename... Types>
		constexpr bool are_externals_unique_v = ((!ExternalSameAs<T, Types>()) && ...) && are_externals_unique_v<Types...>;

		template<typename T>
		constexpr bool are_externals_unique_v<T> = true;

		template <typename T, typename... Ts>
		constexpr bool are_base_of_v = (std::is_base_of_v<T, Ts> && ...);

		void DefineExternals()
		{
			[[msvc::flatten]]
			[] <std::size_t... Is>(std::index_sequence<Is...>)
			{

				((std::apply([par = GetParserByEnum(static_cast<eParser>(Is))](const auto... t_externals) [[msvc::forceinline]]
					{
						static_assert(are_base_of_v<BaseExternal, decltype(t_externals)...>, "Wrong external type");
						static_assert(are_externals_unique_v<decltype(t_externals)...>, "No external duplicates allowed");

						((decltype(t_externals)::DefineExternal(par)), ...);

					}, typename NthTable<Is>::Table{})), ...);

			}(std::make_index_sequence<std::to_underlying(eParser::MAX)>());
			/*
						[[msvc::flatten]]
						[] <std::size_t... Is>(std::index_sequence<Is...>)
						{
							auto Declare = [] <std::size_t... Is2>(const auto table, std::index_sequence<Is2...>)
							{
								const auto par = GetParserByEnum(table.s_parser);
								((std::tuple_element_t<Is2, typename decltype(table)::ExternalTable>::DefineExternal(par)), ...);
							};

							(Declare(NthTable<Is>{}, std::make_index_sequence<
								std::tuple_size_v<typename NthTable<Is>::ExternalTable>>{}
							), ...);

						}(std::make_index_sequence<std::to_underlying(eParser::MAX)>());
			*/
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

