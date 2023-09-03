#include "json_builder.h"

namespace transport_catalogue
{
namespace detail
{
namespace json
{


BaseContext::BaseContext(Builder& builder) : builder_(builder)
{
    
}

KeyContext BaseContext::Key(std::string key)
{
    return builder_.Key(key);
}

Builder& BaseContext::Value(Node::Value value)
{
    return builder_.Value(value);
}

DictItemContext BaseContext::StartDict()
{
    return DictItemContext(builder_.StartDict());
}

Builder& BaseContext::EndDict()
{
    return builder_.EndDict();
}

ArrayContext BaseContext::StartArray()
{
    return ArrayContext(builder_.StartArray());
}
Builder& BaseContext::EndArray()
{
    return builder_.EndArray();
}


KeyContext::KeyContext(Builder& builder) : BaseContext(builder) {
    
}

DictItemContext  KeyContext::Value(Node::Value value)
{
    return BaseContext::Value(std::move(value));
}


DictItemContext::DictItemContext(Builder& builder) : BaseContext(builder) {
    
}


ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder) {
    
}

ArrayContext ArrayContext::Value(Node::Value value)
{
    return BaseContext::Value(move(value));
}


Node Builder::ParseNode(Node::Value value_)
{
    Node node;

    if (std::holds_alternative<bool>(value_))
    {
        bool bool_ = std::get<bool>(value_);
        node = Node(bool_);

    }
    else if (std::holds_alternative<int>(value_))
    {
        int int_ = std::get<int>(value_);
        node = Node(int_);

    }
    else if (std::holds_alternative<double>(value_))
    {
        double double_ = std::get<double>(value_);
        node = Node(double_);

    }
    else if (std::holds_alternative<std::string>(value_))
    {
        std::string str = std::get<std::string>(value_);
        node = Node(std::move(str));

    }
    else if (std::holds_alternative<Array>(value_))
    {
        Array array = std::get<Array>(value_);
        node = Node(std::move(array));

    }
    else if (std::holds_alternative<Dict>(value_))
    {
        Dict dictionary = std::get<Dict>(value_);
        node = Node(std::move(dictionary));

    }
    else
    {
        node = Node();
    }

    return node;
}

void Builder::AddNode(Node node)
{
    if (nodes_stack_.empty())
    {

        if (!root_.IsNull())
        {
            throw std::logic_error("root has been already added");
        }

        root_ = node;
        return;

    }
    else
    {

        if (nodes_stack_.back()->IsArray())
        {
            // добавляем в массив данные
            Array array = nodes_stack_.back()->AsArray();
            array.emplace_back(node);

            nodes_stack_.pop_back();
            auto arr_ptr = std::make_unique<Node>(array);
            nodes_stack_.emplace_back(std::move(arr_ptr));

            return;
        }

        if (nodes_stack_.back()->IsString())
        {
            std::string str = nodes_stack_.back()->AsString();
            nodes_stack_.pop_back();

            if (nodes_stack_.back()->IsMap())
            {
                Dict dictionary = nodes_stack_.back()->AsMap();
                dictionary.emplace(std::move(str), node);

                nodes_stack_.pop_back();
                auto dictionary_ptr = std::make_unique<Node>(dictionary);
                nodes_stack_.emplace_back(std::move(dictionary_ptr));
            }

            return;
        }
    }
}

KeyContext Builder::Key(std::string key_)
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("unable to create map key");
    }

    auto key_ptr = std::make_unique<Node>(key_);

    if (nodes_stack_.back()->IsMap())
    {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }

    return KeyContext(*this);
}

Builder& Builder::Value(Node::Value value_)
{
    AddNode(ParseNode(value_));

    return *this;
}

DictItemContext Builder::StartDict()
{
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));

    return DictItemContext(*this);
}

Builder& Builder::EndDict()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("unable to close as without opening");
    }

    Node node = *nodes_stack_.back();

    if (!node.IsMap())
    {
        throw std::logic_error("object isn't map");
    }

    nodes_stack_.pop_back();
    AddNode(node);

    return *this;
}

ArrayContext Builder::StartArray()
{
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));

    return ArrayContext(*this);
}

Builder& Builder::EndArray()
{
    if (nodes_stack_.empty())
    {
        throw std::logic_error("Array isn't started");
    }

    Node node = *nodes_stack_.back();

    if (!node.IsArray())
    {
        throw std::logic_error("object isn't array");
    }

    nodes_stack_.pop_back();
    AddNode(node);

    return *this;
}

Node Builder::Build()
{
    if (root_.IsNull())
    {
        throw std::logic_error("json is empty");
    }

    if (!nodes_stack_.empty())
    {
        throw std::logic_error("unable to build json : some action missed");
    }

    return root_;
}

}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
