// Supported with union (c) 2020 Union team
// Union HEADER file
#include <list>
#include <concepts>
#include <list>
#include <string_view>
#include <type_traits>
#include <array>
#include <ranges>
#include <vector>
#include <algorithm>

#define BetterDaedalusExternal(function) new BetterDaedalusExternals::DaedalusExternal<#function,function>{}
#define BetterDaedalusExternalWithCondition(function, condition) new BetterDaedalusExternals::DaedalusExternal<#function,function,condition>{}

#define BetterExternalDefinition(parserEnum, ...)\
inline const BetterDaedalusExternals::ExternalTable g_externalTable_##parserEnum{ BetterDaedalusExternals::eParser::parserEnum, __VA_ARGS__ }; \
template<> struct BetterDaedalusExternals::ExternalTableGuard<BetterDaedalusExternals::eParser::parserEnum> {}\


namespace GOTHIC_ENGINE
{
	namespace BetterDaedalusExternals
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

		constexpr bool MpBuild()
		{
#if (defined(__G1) + defined(__G1A) + defined(__G2) + defined(__G2A)) > 1
			return true;
#else
			return false;
#endif
		}

		template<typename T, typename... Args>
		struct FunctionTraitsBase
		{
			using ReturnType = T;
			using ArgTypes = std::tuple<Args...>;
			static constexpr std::size_t ArgCount = sizeof...(Args);
			static constexpr bool HasReturn = !std::is_void_v<ReturnType>;
		};

		template<typename T> struct FunctionTraits;

		template<typename R, typename... Args>
		struct FunctionTraits<R(*)(Args...)>
			: FunctionTraitsBase<R, Args...>
		{
			using Pointer = R(*)(Args...);
			using ArgTypes = std::tuple<Args...>;
		};

		template<typename R>
		struct FunctionTraits<R(*)()>
			: FunctionTraitsBase<R>
		{
			using Pointer = R(*)();
		};

		template<std::size_t N>
		struct FixedStr
		{
			static inline constexpr char CharToUpperSimple(const char t_char)
			{
				return t_char >= 'a' && t_char <= 'z'
					? static_cast<char>(static_cast<unsigned char>(t_char) - ('a' + 'A'))
					: t_char;
			}

			static inline constexpr char CharToLowerSimple(const char t_char)
			{
				return t_char >= 'A' && t_char <= 'Z'
					? static_cast<char>(static_cast<unsigned char>(t_char) + ('a' - 'A'))
					: t_char;
			}

			constexpr FixedStr(const char(&source)[N + 1])
			{
				std::copy(std::cbegin(source), std::cend(source), begin());
			}

			template<std::size_t LeftSize, std::size_t RightSize>
			constexpr FixedStr(const FixedStr<LeftSize>& t_left, const FixedStr<RightSize>& t_right)
			{
				static_assert(LeftSize + RightSize == Size);

				std::copy(t_left.cbegin(), t_left.cend(), begin());
				std::copy(t_right.cbegin(), t_right.cend(), std::next(begin(), static_cast<std::ptrdiff_t>(t_left.size())));
			}

			constexpr auto& Upper()
			{
				std::ranges::transform(m_array, std::begin(m_array), CharToUpperSimple);
				return *this;
			}

			constexpr auto& Lower()
			{
				std::ranges::transform(m_array, std::begin(m_array), CharToLowerSimple);
				return *this;
			}

			[[nodiscard]]
			constexpr size_t Size() const noexcept
			{
				return Size;
			}

			[[nodiscard]]
			constexpr char* begin()
			{
				return m_array.data();
			}

			[[nodiscard]]
			constexpr char* end()
			{
				return std::next(m_array.data(), Size());
			}

			[[nodiscard]]
			constexpr const char* cbegin() const
			{
				return m_array.data();
			}

			[[nodiscard]]
			constexpr const char* cend() const
			{
				return std::next(m_array.data(), static_cast<std::ptrdiff_t>(Size()));
			}

			[[nodiscard]]
			constexpr std::string_view Data() const noexcept
			{
				return std::string_view{ m_array.data(), m_array.size() };
			}

			template<std::size_t LeftSize, std::size_t RightSize>
			friend constexpr auto operator+(const FixedStr<LeftSize>& t_left, const FixedStr<RightSize>& t_right);

			std::array<char, N + 1> m_array{};
		};

		template<std::size_t Size>
		FixedStr(const char(&)[Size]) -> FixedStr<Size - 1>;

		template<std::size_t LeftSize, std::size_t RightSize>
		FixedStr(FixedStr<LeftSize> t_left, FixedStr<RightSize> t_right) -> FixedStr<LeftSize + RightSize>;

		template<std::size_t LeftSize, std::size_t RightSize>
		constexpr auto operator+(const FixedStr<LeftSize>& t_left, const FixedStr<RightSize>& t_right)
		{
			return FixedStr(t_left, t_right);
		};


		template<class Value, typename Key,
			typename Container = std::vector<Value>>
			struct MappedInstance
		{
			using KeyType = Key;
			using ValueType = Value;
			using MappedBase = MappedInstance<Value, Key, Container>;

			constexpr MappedInstance(const Key& t_key)
				: m_key(t_key)
			{
			}

			//TODO use static constexpr Value& operator[] if MSVC will get this implemented
			[[nodiscard]] static Value& Get(const Key& t_key)
			{
#define Unlikely [[unlikely]]
#define Likely	[[unlikely]]

				const auto SameKey = [&t_key](const auto& t_object)
					{
						return t_object.m_key == t_key;
					};

				auto& value = [&]() -> Value&
					{
						if (const auto it = std::ranges::find_if(s_objects, SameKey);
							it != End()) Likely
						{
							return *it;
						}
						else Unlikely
						{
							return s_objects.emplace_back(Value{t_key});
						}

					}();
#undef Unlikely
#undef Likely
					return value;
			}

			static void ForEach(auto&& t_callable)
			{
				std::ranges::for_each(s_objects, t_callable);
			}

			static typename Container::iterator FindIf(auto&& t_callable)
			{
				return std::ranges::find_if(s_objects, t_callable);
			}

			static typename Container::iterator Find(const Key& t_key)
			{
				const auto SameKey = [&t_key](const auto& t_value)
					{
						return t_key == t_value.m_key;
					};

				return std::ranges::find_if(s_objects, SameKey);
			}

			static auto Begin()
			{
				return std::begin(s_objects);
			}

			static auto End()
			{
				return std::end(s_objects);
			}

			static void Clear()
			{
				s_objects.clear();
			}

			Key m_key;


			inline static Container s_objects;
		};


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

			StringPool(const KeyType& t_key) : MappedBase(t_key) {}

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
			virtual void DefineExternal([[maybe_unused]] zCParser* const t_parser) const {}

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
		struct DaedalusExternal final : public BaseExternal
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

			virtual void DefineExternal(zCParser* const t_parser) const
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

		struct BaseExternalTable
		{
			BaseExternalTable()
			{
				s_tables.push_back(this);
			}

			virtual void Define() const = 0;

			static void DefineAll()
			{
				for (const auto table : s_tables)
				{
					table->Define();
				}
			}
			
			inline static std::vector<BaseExternalTable*> s_tables;
		};

		template<typename... Args>
			requires (are_externals_unique_v<Args...> && are_base_of_v<BaseExternal, Args...>)
		struct ExternalTable final : public BaseExternalTable
		{
			ExternalTable(const eParser t_parserEnum, Args*... t_args)
				: m_parser(GetParserByEnum(t_parserEnum))

			{
				((m_externals.push_back(t_args)),...);
			}

			void Define() const override
			{
				for (const auto& ext : m_externals)
				{
					ext->DefineExternal(m_parser);
				}

			}

			~ExternalTable()
			{
				for (auto ptr : m_externals)
				{
					delete ptr;
				}

				m_externals.clear();
			}

			std::vector<BaseExternal*> m_externals;
			zCParser* m_parser;

			
		};


		template<eParser Parser>
		struct ExternalTableGuard
		{
		};

		void __fastcall zCPar_DataStack__Clear(zCPar_DataStack* t_this, void* t_reg);
		inline HOOK Hook_zCPar_DataStack__Clear PATCH(&zCPar_DataStack::Clear, &zCPar_DataStack__Clear);

		void __fastcall oCGame__DefineExternals_Ulfi(oCGame* t_this, void* t_reg, zCParser* t_parser);
		inline HOOK Hook_oCGame__DefineExternals_Ulfi PATCH(oCGame::DefineExternals_Ulfi, &oCGame__DefineExternals_Ulfi);

		void DefineExternals()
		{
			BaseExternalTable::DefineAll();
		}

		void __fastcall oCGame__DefineExternals_Ulfi(oCGame* t_this, void* t_reg, zCParser* t_parser)
		{
			Hook_oCGame__DefineExternals_Ulfi(t_this, t_reg, t_parser);
			DefineExternals();

		}
		void __fastcall zCPar_DataStack__Clear(zCPar_DataStack* t_this, void* t_reg)
		{
			Hook_zCPar_DataStack__Clear(t_this, t_reg);
			StringPool::ClearPool(t_this);
		}
	}

}