// Supported with union (c) 2020 Union team
// Union HEADER file
#include <list>
#include <string_view>
#include <type_traits>
#include <array>
#include <ranges>
#include <vector>
#include <algorithm>

namespace GOTHIC_ENGINE
{
	namespace Externals
	{
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


	}
}
