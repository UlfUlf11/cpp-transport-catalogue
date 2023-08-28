#include "json.h"

using namespace std;

namespace transport_catalogue
{
namespace detail
{
namespace json
{

namespace
{

Node LoadNode(istream& input);

Node LoadArray(istream& input)
{
    Array result;

    for (char c; input >> c && c != ']';)
    {
        if (c != ',')
        {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (!input)
    {
        throw ParsingError("Array loading error"s);
    }

    return Node(move(result));
}

Node LoadTrue(istream& input)
{
    int true_word = 4;
    string s;

    for (int i = 0; i < true_word; ++i)
    {
        char ch = static_cast<char>(input.get());
        s.push_back(ch);
    }

    if (s == "true"s)
    {
        return Node(true);
    }
    else
    {
        throw ParsingError("True loading error");
    }
}

Node LoadFalse(istream& input)
{
    int false_word = 5;
    string s;

    for (int i = 0; i < false_word; ++i)
    {
        char ch = static_cast<char>(input.get());
        s.push_back(ch);
    }

    if (s == "false"s)
    {
        return Node(false);
    }
    else
    {
        throw ParsingError("False loading error");
    }
}

Node LoadNull(istream& input)
{
    int null_word = 4;
    string s;

    for (int i = 0; i < null_word; ++i)
    {
        char ch = static_cast<char>(input.get());
        s.push_back(ch);
    }

    if (s == "null"s)
    {
        return Node(nullptr);
    }
    else
    {
        throw ParsingError("Null loading error");
    }
}

Node LoadNumber(std::istream& input)
{
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input]
    {
        parsed_num += static_cast<char>(input.get());
        if (!input)
        {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char]
    {
        if (!std::isdigit(input.peek()))
        {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek()))
        {
            read_char();
        }
    };

    if (input.peek() == '-')
    {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0')
    {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else
    {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.')
    {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E')
    {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-')
        {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try
    {
        if (is_int)
        {
            // Сначала пробуем преобразовать строку в int
            try
            {
                return std::stoi(parsed_num);
            }
            catch (...)
            {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    }
    catch (...)
    {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input)
{
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true)
    {
        if (it == end)
        {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"')
        {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\')
        {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end)
            {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char)
            {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else
        {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadDict(istream& input)
{
    Dict result;

    for (char c; input >> c && c != '}';)
    {
        if (c == ',')
        {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }

    if (!input)
    {
        throw ParsingError("Dictionary loading error"s);
    }
    else
    {
        return Node(move(result));
    }
}

Node LoadNode(istream& input)
{
    char c;
    input >> c;

    if (!c)
    {
        throw ParsingError(""s);
    }

    if (c == '[')
    {
        return LoadArray(input);
    }
    else if (c == '{')
    {
        return LoadDict(input);
    }
    else if (c == '"')
    {
        return LoadString(input);
    }
    else if (c == 't')
    {
        input.putback(c);
        return LoadTrue(input);
    }
    else if (c == 'f')
    {
        input.putback(c);
        return LoadFalse(input);
    }
    else if (c == 'n')
    {
        input.putback(c);
        return LoadNull(input);
    }
    else
    {
        input.putback(c);
        return LoadNumber(input);
    }
}

}//end namespace

Node::Node(Array array) :
    value_(std::move(array))
{
}

Node::Node(std::nullptr_t) :
    Node() {}

Node::Node(bool value)
    : value_(value)
{
}

Node::Node(Dict map)
    : value_(move(map))
{
}

Node::Node(int value)
    : value_(value)
{
}

Node::Node(string value)
    : value_(move(value))
{
}

Node::Node(double value)
    : value_(value)
{
}

const Array& Node::AsArray() const
{
    using namespace std::literals;
    if (!IsArray())
    {
        throw std::logic_error("Error: not array"s);
    }
    return std::get<Array>(value_);
}

const Dict& Node::AsMap() const
{
    using namespace std::literals;
    if (!IsMap())
    {
        throw std::logic_error("Error: not dict"s);
    }
    return std::get<Dict>(value_);
}

const string& Node::AsString() const
{
    using namespace std::literals;
    if (!IsString())
    {
        throw std::logic_error("Error: not string"s);
    }
    return std::get<std::string>(value_);
}

int Node::AsInt() const
{
    using namespace std::literals;
    if (!IsInt())
    {
        throw std::logic_error("Error: not int"s);
    }
    return std::get<int>(value_);
}

double Node::AsDouble() const
{
    using namespace std::literals;
    if (!IsDouble())
    {
        throw std::logic_error("Error: not double"s);
    }
    if (IsPureDouble())
    {
        return std::get<double>(value_);
    }
    else
    {
        return AsInt();
    }
}

bool Node::AsBool() const
{
    using namespace std::literals;
    if (!IsBool())
    {
        throw std::logic_error("Error: not bool"s);
    }
    return std::get<bool>(value_);
}

bool Node::IsNull() const
{
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool Node::IsInt() const
{
    return std::holds_alternative<int>(value_);
}

bool Node::IsDouble() const
{
    return IsInt() || IsPureDouble();
}

bool Node::IsPureDouble() const
{
    return std::holds_alternative<double>(value_);
}

bool Node::IsBool() const
{
    return std::holds_alternative<bool>(value_);
}

bool Node::IsString() const
{
    return std::holds_alternative<std::string>(value_);
}

bool Node::IsArray() const
{
    return std::holds_alternative<Array>(value_);
}

bool Node::IsMap() const
{
    return std::holds_alternative<Dict>(value_);
}


Document::Document(Node root)
    : root_(move(root))
{
}

const Node& Document::GetRoot() const
{
    return root_;
}

Document Load(istream& input)
{
    return Document{ LoadNode(input) };
}

// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
struct PrintContext
{
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const
    {
        return { out, indent_step, indent_step + indent };
    }
};

void PrintNode(const Node& node, const PrintContext& context);


struct PrintValue
{

    const PrintContext& context;

    // Перегрузка функции PrintValue для вывода значений null
    void operator()(const std::nullptr_t&)
    {
        context.out << "null"s;
    }

    void operator()(const Array& nodes)
    {
        std::ostream& out = context.out;
        out << "[\n"sv;
        bool first = true;
        PrintContext indented_context = context.Indented();

        for (const Node& node : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }

            indented_context.PrintIndent();
            PrintNode(node, indented_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put(']');
    }

    void operator()(const Dict& nodes)
    {
        std::ostream& out = context.out;
        out << "{\n"sv;
        bool first = true;
        PrintContext indented_context = context.Indented();

        for (const auto& [key, node] : nodes)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                out << ",\n"sv;
            }

            indented_context.PrintIndent();
            PrintNode(key, context);
            out << ": "sv;
            PrintNode(node, indented_context);
        }

        out.put('\n');
        context.PrintIndent();
        out.put('}');
    }

    void operator()(const bool& value)
    {
        context.out << std::boolalpha << value;
    }

    //Словарь замен для escape-последовательностей
    std::map<char, std::string> escape_sequence{ {'\n', R"(\n)"}, {'\r', R"(\r)"}, {'"', R"(\")"},  {'\\', R"(\\)"} };

    void operator()(const std::string& value)
    {

        std::ostream& out = context.out;
        out.put('"');

        for (const char c : value)
        {
            if (escape_sequence.count(c))
            {
                out << escape_sequence.at(c);
            }
            else
            {
                out << c;
            }
        }

        out.put('"');
    }

    // Шаблон, подходящий для вывода double и int
    template <typename Value>
    void operator()(const Value& value)
    {
        context.out << value;
    }
};

void PrintNode(const Node& node, const PrintContext& context)
{

    std::visit(PrintValue{ context }, node.GetValue());
}

void Print(const Document& document, std::ostream& output)
{
    PrintNode(document.GetRoot(), PrintContext{ output });
}

}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue
