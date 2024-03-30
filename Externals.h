// Supported with union (c) 2020 Union team
// Union HEADER file
#include <list>
#include <concepts>

#define MakeDaedalusExternal(function) Externals::DaedalusExternal<#function,function>
#define MakeDaedalusExternalWithCondition(function, condition) Externals::DaedalusExternal<#function,function,condition>

namespace GOTHIC_ENGINE
{
	enum class eParser
	{
		GAME,
		MENU,
		MUSIC,
		SFX,
		PFX,
		VFX,
		MAX
	};

#define ExternalDefinition(ParserEnum,...) \
	namespace Externals\
	{\
		template<>\
		struct ExternalTable<ParserEnum>\
		{\
			static constexpr const auto s_parser = ParserEnum; \
			using Table = ExternalsTuple\
			<\
			__VA_ARGS__\
			>; \
		}; \
	}\



	namespace Externals
	{	
		struct DaedalusFunction 
		{
			int m_index{ -1 };
		};
		
		template<typename T>
		concept ScriptData =
			std::is_same_v<std::decay_t<T>, int>
			|| std::is_same_v<std::decay_t<T>, DaedalusFunction>
			|| std::is_same_v<std::decay_t<T>, float>
			|| std::is_same_v<std::decay_t<T>, zSTRING>
			|| (std::is_pointer_v<std::decay_t<T>> && !std::is_pointer_v<std::remove_pointer_t<T>>);

		template<typename T>
		concept ScriptReturn =
			std::is_same_v<T, int>
			|| std::is_same_v<T, DaedalusFunction>
			|| std::is_same_v<T, float>
			|| std::is_same_v<T, zSTRING>
			|| std::is_same_v<T, void>
			|| (std::is_pointer_v<std::decay_t<T>> && !std::is_pointer_v<std::remove_pointer_t<T>>);
		
		
		inline static constexpr zCParser* GetParserByEnum(const eParser t_enum)
		{
			switch (t_enum)
			{
			case eParser::GAME:
				return parser;

			case eParser::MUSIC:
				return parserMusic;

			case eParser::SFX:
				return parserSoundFX;

			case eParser::PFX:
				return parserParticleFX;

			case eParser::VFX:
				return parserParticleFX;

			case eParser::MENU:
				return parserVisualFX;

			case eParser::MAX:
				std::unreachable();
			}

			return {};
		}


		template<ScriptData T>
		inline constexpr auto GetData(zCParser* const t_parser)
		{
			if constexpr (std::is_same_v<T, int> || std::is_same_v<T, DaedalusFunction>)
			{
				int parameter;
				t_parser->GetParameter(parameter);
				return T{ parameter };
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				float parameter;
				t_parser->GetParameter(parameter);
				return parameter;
			}
			else if constexpr (std::is_same_v<T, zSTRING>)
			{
				return std::cref(*t_parser->PopString());
			}
			else if constexpr (std::is_pointer_v<T>)
			{
				return static_cast<T>(t_parser->GetInstance());
			}
		}


		template<ScriptReturn T>
		inline constexpr int TypeToEnum()
		{
			if constexpr (std::is_same_v<T, int>)
			{
				return zPAR_TYPE_INT;
			}
			else if constexpr (std::is_same_v<T, DaedalusFunction>)
			{
				return zPAR_TYPE_FUNC;
			}
			else if constexpr (std::is_same_v<T, zSTRING>)
			{
				return zPAR_TYPE_STRING;
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				return zPAR_TYPE_FLOAT;
			}
			else if constexpr (std::is_pointer_v<T>)
			{
				return zPAR_TYPE_INSTANCE;
			}
			else if constexpr (std::is_void_v<T>)
			{
				return zPAR_TYPE_VOID;
			}
		}

		class StringPool : public MappedInstance<StringPool, const zCParser*>
		{
		public:

			StringPool(const KeyType& t_key) : MappedBase(t_key){}

			zSTRING& GetNewString()
			{
				return m_strings.emplace_back();
			}

			void ClearPool()
			{
				m_strings.clear();
			}

			static void ClearPool(const zCPar_DataStack* t_stack)
			{
				const auto ResetPool = [&t_stack](const auto& t_pool)
					{
						return std::addressof(t_pool.m_key->datastack) == t_stack;
					};

				if (const auto it = FindIf(ResetPool); it != End())
				{
					it->ClearPool();
				}
			}

		private:
			std::list<zSTRING> m_strings;
		};

		struct BaseExternal
		{
			static void DefineExternal([[maybe_unused]] zCParser* const t_parser) {}

			inline static zSTRING* nameBuffer = []()
				{
					if constexpr (MpBuild())
					{
						if (Union.GetEngineVersion() != ENGINE)
						{
							return nullptr;
						}
					}

					return new zSTRING;

				}();

		};


		template<FixedStr Name, auto Callable, auto ConditionFunc = nullptr>
		struct DaedalusExternal : public BaseExternal
		{
			using CallableType = decltype(Callable);
			using NameType = decltype(Name);

			using Type = DaedalusExternal<Name, Callable, ConditionFunc>;

			static constexpr bool IsFunctionPointer = std::is_pointer_v<CallableType>
				&& std::is_function_v<typename std::remove_pointer_t<CallableType>>;

			using CallableInfo = FunctionTraits<std::conditional_t<IsFunctionPointer, CallableType, decltype(+Callable)>>;
			using ReturnType = CallableInfo::ReturnType;
	

			static constexpr NameType s_name = Name;
			static constexpr CallableType s_callable = Callable;

			static bool Condition()
			{
				if constexpr (!std::is_same_v<decltype(ConditionFunc), std::nullptr_t>)
				{
					return ConditionFunc();
				}
				else
				{
					return true;
				}
			}

			static int __cdecl Definition()
			{
				[currentParser = zCParser::cur_parser] <std::size_t... Is>(std::index_sequence<Is...>) [[msvc::forceinline]]
					{
						if constexpr (CallableInfo::HasReturn)
						{
							[[msvc::flatten]]
							if constexpr (std::is_same_v<std::decay_t<ReturnType>, zSTRING>)
							{
								auto& str = StringPool::Get(currentParser).GetNewString();
								str = Callable(GetData<std::decay_t<std::tuple_element_t<Is, CallableInfo::ArgTypes>>>(currentParser)...);
								currentParser->SetReturn(str);
							}
							else
							{
								currentParser->SetReturn(Callable(GetData<std::decay_t<std::tuple_element_t<Is, CallableInfo::ArgTypes>>>(currentParser)...));
							}
						}
						else
						{
							[[msvc::flatten]]
							Callable(GetData<std::decay_t<std::tuple_element_t<Is, CallableInfo::ArgTypes>>>(currentParser)...);
						}


					}(std::make_index_sequence<CallableInfo::ArgCount>());

					return 0;
			}

			static void DefineExternal(zCParser* const t_parser)
			{
				if (!Condition())
				{
					return;
				}

				*BaseExternal::nameBuffer = Name.Data().data();

				if constexpr (CallableInfo::ArgCount != 0)
				{
					[t_parser] <std::size_t... Is>(std::index_sequence<Is...>)
					{
						t_parser->DefineExternal(*BaseExternal::nameBuffer, &Definition, TypeToEnum<std::decay_t<ReturnType>>(), TypeToEnum<std::decay_t<std::tuple_element_t<Is, typename CallableInfo::ArgTypes>>>()..., 0);

					}(std::make_index_sequence<CallableInfo::ArgCount>());

				}
				else
				{
					t_parser->DefineExternal(*BaseExternal::nameBuffer, &Definition, TypeToEnum<std::decay_t<ReturnType>>(), 0);
				}
			}
		};

		template<typename T1, typename T2>
		constexpr bool ExternalSameAs()
		{
			return T1::s_name.Data() == T2::s_name.Data();
		}

		template<typename T, typename... Types>
		constexpr bool are_externals_unique_v = ((!ExternalSameAs<T, Types>()) && ...) && are_externals_unique_v<Types...>;

		template<typename T>
		constexpr bool are_externals_unique_v<T> = true;

		template <typename T, typename... Ts>
		constexpr bool are_base_of_v = (std::is_base_of_v<T, Ts> && ...);

		template<typename...Args>
			requires (are_externals_unique_v<Args...> && are_base_of_v<BaseExternal,Args...>)
		struct ExternalsTuple
		{
			using Type = std::tuple<Args...>;
		};

		template<eParser Parser>
		struct ExternalTable
		{
			static constexpr const auto s_parser = Parser;

			using Table = ExternalsTuple
				<
				BaseExternal
				>;

		};

		template<size_t N> 
			requires(N < std::to_underlying(eParser::MAX))
		using NthTable = ExternalTable<static_cast<eParser>(N)>;


	}
}
