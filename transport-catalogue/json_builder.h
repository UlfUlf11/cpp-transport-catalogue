#pragma once

#include "json.h"
#include <stack>
#include <string>
#include <memory>

namespace transport_catalogue
{
namespace detail
{
namespace json
{

class KeyContext;
class DictItemContext;
class ArrayContext;

class Builder
{
public:
    Node ParseNode(Node::Value value_);
    void AddNode(Node node);

    /*
    При определении словаря задаёт строковое значение ключа для очередной пары ключ-значение.
    Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение
    с помощью метода Value или начинать его определение с помощью StartDict или StartArray.
    */
    KeyContext Key(std::string key_);


    /*
     Начинает определение сложного значения-словаря. Вызывается в тех же контекстах, что и Value.
     Следующим вызовом обязательно должен быть Key или EndDict.
    */
    DictItemContext StartDict();

    /*
    Начинает определение сложного значения-массива. Вызывается в тех же контекстах, что и Value.
    Следующим вызовом обязательно должен быть EndArray или любой, задающий новое значение: Value,
    StartDict или StartArray.
    */
    ArrayContext StartArray();

    /*
     Задаёт значение, соответствующее ключу при определении словаря, очередной элемент массива или,
     если вызвать сразу после конструктора json::Builder, всё содержимое конструируемого JSON-объекта.
     Может принимать как простой объект — число или строку — так и целый массив или словарь.
     Здесь Node::Value — это синоним для базового класса Node, шаблона variant с набором возможных
     типов-значений. Смотрите заготовку кода.
    */
    Builder& Value(Node::Value value);

    //Завершает определение сложного значения-словаря. Последним незавершённым вызовом Start*
    //должен быть StartDict.
    Builder& EndDict();

    //Завершает определение сложного значения-массива. Последним незавершённым вызовом Start*
    //должен быть StartArray.
    Builder& EndArray();

    /*
     Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов.
     К этому моменту для каждого Start* должен быть вызван соответствующий End*. При этом сам объект
     должен быть определён, то есть вызов json::Builder{}.Build() недопустим.
    */
    Node Build();

private:
    //сам конструируемый объект
    Node root_;

    //стек указателей на те вершины JSON, которые ещё не построены:
    //то есть текущее описываемое значение и цепочка его родителей.
    //Он поможет возвращаться в нужный контекст после вызова End-методов.
    std::vector<std::unique_ptr<Node>> nodes_stack_;

};

class BaseContext
{
public:
    BaseContext(Builder& builder);

    KeyContext Key(std::string key);
    DictItemContext StartDict();
    ArrayContext StartArray();

    Builder& Value(Node::Value value);

    Builder& EndDict();

    Builder& EndArray();

protected:
    Builder& builder_;

};

class KeyContext : public BaseContext
{
public:
    KeyContext(Builder& builder);

    KeyContext Key(std::string key) = delete;

    BaseContext EndDict() = delete;
    BaseContext EndArray() = delete;

    DictItemContext Value(Node::Value value);
};

class DictItemContext : public BaseContext
{
public:
    DictItemContext(Builder& builder);

    DictItemContext StartDict() = delete;

    ArrayContext StartArray() = delete;
    Builder& EndArray() = delete;

    Builder& Value(Node::Value value) = delete;
};

class ArrayContext : public BaseContext
{
public:
    ArrayContext(Builder& builder);

    KeyContext Key(std::string key) = delete;

    Builder& EndDict() = delete;

    ArrayContext Value(Node::Value value);
};

}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
