#pragma once

#include <tuple>

namespace grlx {
// Credits to Evg in https://stackoverflow.com/questions/59018585/variadic-templates-unfold-arguments-in-groups


namespace impl {
template <std::size_t k, class Fn, class Tuple, std::size_t... js>
void unfold_nk(Fn fn, Tuple&& tuple, std::index_sequence<js...>) {
  fn(std::get<k + js>(std::forward<Tuple>(tuple))...);
}

template <std::size_t n, class Fn, class Tuple, std::size_t... is>
void unfold_n(Fn fn, Tuple&& tuple, std::index_sequence<is...>) {
  (unfold_nk<n * is>(fn, std::forward<Tuple>(tuple),
                     std::make_index_sequence<n>{}),
   ...);
}
} // namespace impl

// template <std::size_t n, class Fn, typename... Args>
// void unfold(Fn fn, Args... args) {
//   static_assert(sizeof...(Args) % n == 0);
//   impl::unfold_n<n>(fn, std::forward_as_tuple(std::forward<Args>(args)...),
//                     std::make_index_sequence<sizeof...(Args) / n>{});
// }

template <std::size_t n, class Fn, typename... Args>
void unfold(Fn fn, Args&&... args) {
  static_assert(sizeof...(Args) % n == 0);
  impl::unfold_n<n>(fn, std::forward_as_tuple(std::forward<Args>(args)...),
                    std::make_index_sequence<sizeof...(Args) / n>{});
}

}