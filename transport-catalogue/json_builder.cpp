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


            KeyContext::KeyContext(Builder& builder) : BaseContext(builder)
            {

            }

            DictItemContext  KeyContext::Value(Node::Value value)
            {
                return BaseContext::Value(std::move(value));
            }


            DictItemContext::DictItemContext(Builder& builder) : BaseContext(builder)
            {

            }


            ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder)
            {

            }

            ArrayContext ArrayContext::Value(Node::Value value)
            {
                return BaseContext::Value(move(value));
            }


            void Builder::AddNode(Node node)
            {
                if (nodes_stack_.empty())
                {

                    if (!root_.IsNull())
                    {
                        throw std::logic_error("root has been added");
                    }

                    root_ = node;
                    return;

                }
                else
                {
                    if (nodes_stack_.back()->IsArray())
                    {
                        Array arr = nodes_stack_.back()->AsArray();
                        arr.emplace_back(node);

                        nodes_stack_.pop_back();
                        auto arr_ptr = std::make_unique<Node>(arr);
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
                Node node = std::visit([](auto val)
                    {
                        return Node(val);
                    }, value_);

                AddNode(node);
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
