#include "svg.h"

namespace svg
{

using namespace std::literals;

// ---------- Color ------------------





// ---------- Render ------------------

void Object::Render(const RenderContext& context) const
{
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)
{
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)
{
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point)
{
    points_.emplace_back(point);
    return *this;
}

std::string Polyline::GetPointsLine() const
{
    std::stringstream out;
    bool is_first = true;
    for (const auto& point : points_)
    {
        if (!is_first)
        {
            out << " "s;
        }
        is_first = false;
        out << point.x << ","s << point.y;
    }
    out << R"(" )";
    return out.str();
}

void Polyline::RenderObject(const RenderContext& context) const
{
    auto& out = context.out;
    out << R"(<polyline points=")" << GetPointsLine();
    RenderAttrs(context.out);
    out << R"(/>)";
}

// ---------- Text ------------------

Text& Text::SetPosition(Point pos)
{
    position_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size)
{
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data)
{
    data_ = std::move(data);
    return *this;
}

//словарь замен для специальных символов
const std::map<char, std::string> special_symbols{ {'"', "&quot;"}, {'\'', "&apos;"}, {'<', "&lt;"}, {'>', "&gt;"}, {'&', "&amp;"} };

void Text::RenderObject(const RenderContext& context) const
{

    std::ostream& out = context.out;

    out << R"(<text )";
    RenderAttrs(context.out);
    out << R"(x=")" << position_.x
        << R"(" y=")" << position_.y
        << R"(" )"
        << R"(dx=")" << offset_.x
        << R"(" dy=")" << offset_.y
        << R"(" )"
        << R"(font-size=")" << font_size_
        << R"(")";

    if (!font_family_.empty())
    {
        out << R"( font-family=")" << font_family_ << R"(")";
    }

    if (!font_weight_.empty())
    {
        out << R"( font-weight=")" << font_weight_ << R"(")";
    }

    out << ">"s;

    for (const char c : data_)
    {
        if (special_symbols.count(c))
        {
            out << special_symbols.at(c);
        }
        else
        {
            out << c;
        }
    }

    out << "</text>"s;

}

// ---------- Document ------------------

void Document::AddPtr(std::unique_ptr<Object>&& obj)
{
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const
{
    int indent = 2;
    int indent_step = 2;

    RenderContext context(out, indent_step, indent);

    out << R"(<?xml version="1.0" encoding="UTF-8" ?>)" << std::endl;
    out << R"(<svg xmlns="http://www.w3.org/2000/svg" version="1.1">)" << std::endl;

    for (const auto& object : objects_)
    {
        object->Render(context);
    }

    out << R"(</svg>)";
}

}  // namespace svg
