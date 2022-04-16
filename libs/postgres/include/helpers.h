#pragma once

#include <libs/common/include/stringify.h>
#include <libs/postgres/include/pg_pool.h>

#include <optional>
#include <string>
#include <pqxx/pqxx>

namespace vm_scheduler::pg {

pqxx::result execQuery(const std::string& sql, pqxx::transaction_base& w);

template<typename T>
requires std::is_same_v<T, std::string>
void formatValue(
    const pqxx::transaction_base& w, const T& value, std::stringstream& out)
{
    out << w.quote(value);
}

template<typename T>
requires std::is_integral_v<T>
void formatValue(
    const pqxx::transaction_base& w, const T& value, std::stringstream& out)
{
    out << value;
}

template<typename T>
requires std::is_enum_v<T>
void formatValue(
    const pqxx::transaction_base& w, const T& value, std::stringstream& out)
{
    out << w.quote(toString(value).data());
}

template<typename TItem>
using FormatValue = std::function<void(const TItem&, std::stringstream&)>;

template<typename TContainer>
std::string asFormattedList(
    const TContainer& items, const pqxx::transaction_base& txn)
{
    using TItem = typename TContainer::value_type;
    const auto format = [&txn](const TItem& item, std::stringstream& stream) {
        formatValue(txn, item, stream);
    };

    return vm_scheduler::asFormattedList(items, format);
}

template<typename T>
concept IsEnum = std::is_enum<T>::value;

template<typename TContainer>
requires std::forward_iterator<typename TContainer::iterator> &&
    IsEnum<typename TContainer::value_type>
        std::string asFormattedList(const TContainer& items)
{
    using EnumType = typename TContainer::value_type;
    const auto formatValue = [](const EnumType& item, std::stringstream& stream) {
        stream << "'" << toString(item) << "'";
    };

    return asFormattedList(items, formatValue);
}

template<typename T>
std::vector<T> extractIds(const pqxx::result& pgResult)
{
    std::vector<T> ids;
    ids.reserve(pgResult.size());
    std::transform(
        pgResult.begin(),
        pgResult.end(),
        std::back_inserter(ids),
        [](const pqxx::row row) -> T { return row.at("id").as<T>(); });
    return ids;
}

PgPool createPool(
    const std::optional<std::string>& connectionString = std::nullopt,
    const std::optional<size_t>& poolSize = std::nullopt);

} // namespace vm_scheduler::pg
