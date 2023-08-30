#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace transport_catalogue
{
namespace detail
{
namespace json
{

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;



class ParsingError : public std::runtime_error
{
public:
    using runtime_error::runtime_error;
};


class Node final:  public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
{
public:
    using variant::variant;
    using Value = variant;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    bool AsBool() const;
    const std::string& AsString() const;

    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;

    const Value& GetValue() const
    {
        return *this;
    };

private:

};

inline bool operator==(const Node& lhs, const Node& rhs)
{
    return lhs.GetValue() == rhs.GetValue();
}

inline bool operator!=(const Node& lhs, const Node& rhs)
{
    return !(lhs == rhs);
}

class Document
{
public:
    explicit Document(Node root);
    Document() = default;
    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

inline bool operator==(const Document& lhs, const Document& rhs)
{
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs)
{
    return !(lhs == rhs);
}

void Print(const Document& doc, std::ostream& output);

}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
