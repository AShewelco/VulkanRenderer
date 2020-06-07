#pragma once

namespace vkr::algorithm {
    template <class C, class P>
    constexpr auto contains(const C& container, P predicate) -> bool {
        for (const auto& element : container) {
            if (predicate(element)) {
                return true;
            }
        }
        return false;
    }

    template <class T, class S>
    constexpr auto select(const std::vector<T>& container, S selector) {
        std::vector<std::invoke_result_t<S, T>> result;
        result.reserve(container.size());
        for (const auto& element : container) {
            result.push_back(selector(element));
        }
        return result;
    }
}
