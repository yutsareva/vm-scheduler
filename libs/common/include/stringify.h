#pragma once

#include <iterator>
#include <functional>
#include <string>
#include <sstream>

namespace vm_scheduler {

template<typename... P>
std::string toString(const P&... params)
{
    std::stringstream stream;

    (stream << ... << params);

    return stream.str();
}

template <typename TItem>
using FormatValue = std::function<void(const TItem&, std::stringstream&)>;

template <typename TContainer>
requires std::forward_iterator<typename TContainer::iterator>
    std::string asFormattedList(
        const TContainer& items,
        const FormatValue<typename TContainer::value_type>& formatValue,
        const std::string& start = "(",
        const std::string& end = ")")
{
    std::stringstream formattedList;
    formattedList << start;
    for (auto it = items.begin(); it != items.end(); ++it) {
        formattedList << (it != items.begin() ? ", " : "");
        formatValue(*it, formattedList);
    }
    formattedList << end;
    return formattedList.str();
}

template <typename TContainer>
std::string joinSeq(const TContainer& items)
{
    using TItem = typename TContainer::value_type;
    const auto formatValue = [](const TItem& item, std::stringstream& stream)
    { stream << item; };

    return asFormattedList(items, formatValue, "", "");
}

} // namespace vm_scheduler
