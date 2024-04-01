#pragma once

template<int first, int last>
struct StaticFor
{
	template<typename callback>
	static inline constexpr void Apply(callback const& f)
	{
		if constexpr(first < last) {
			f(std::integral_constant<int, first>{});
			StaticFor<first + 1, last>::Apply(f);
		}
	}
};

template<int n>
struct StaticFor<n, n>
{
	template <typename callback>
	static inline constexpr void Apply(callback const& f) {}
};