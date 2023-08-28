#pragma once
#define _USE_MATH_DEFINES

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <optional>
#include <variant>

#include <cmath>

namespace svg
{

struct Rgb
{
public:
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red)
        , green(green)
        , blue(blue) {};
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};


struct Rgba
{
public:
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity) : red(red)
        , green(green)
        , blue(blue)
        , opacity(opacity) {};
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{ "none" };

//Класс-посетитель для Color
struct PrintColor
{
    std::ostream& out;

    void operator()(std::monostate)
    {
        using namespace std::literals;
        out << "none"sv;
    }

    void operator()(Rgb rgb)
    {
        using namespace std::literals;
        out << "rgb("sv << static_cast<short>(rgb.red) << ","sv
            << static_cast<short>(rgb.green) << ","sv
            << static_cast<short>(rgb.blue) << ")"sv;
    }

    void operator()(Rgba rgba)
    {
        using namespace std::literals;
        out << "rgba("sv << static_cast<short>(rgba.red) << ","sv
            << static_cast<short>(rgba.green) << ","sv
            << static_cast<short>(rgba.blue) << ","sv
            << (rgba.opacity) << ")"sv;
    }


    void operator()(std::string color)
    {
        out << color;
    }
};

inline std::ostream& operator<<(std::ostream& out, const Color& color)
{

    std::visit(PrintColor{ out }, color);

    return out;
}


enum class StrokeLineCap
{
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin
{
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap)
{

    using namespace std::literals;

    if (stroke_line_cap == StrokeLineCap::BUTT)
    {
        out << "butt"sv;
    }
    else if (stroke_line_cap == StrokeLineCap::ROUND)
    {
        out << "round"sv;
    }
    else if (stroke_line_cap == StrokeLineCap::SQUARE)
    {
        out << "square"sv;
    }
    return out;
}

inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join)
{

    using namespace std::literals;

    if (stroke_line_join == StrokeLineJoin::ARCS)
    {
        out << "arcs"sv;
    }
    else if (stroke_line_join == StrokeLineJoin::BEVEL)
    {
        out << "bevel"sv;
    }
    else if (stroke_line_join == StrokeLineJoin::MITER)
    {
        out << "miter"sv;
    }
    else if (stroke_line_join == StrokeLineJoin::MITER_CLIP)
    {
        out << "miter-clip"sv;
    }
    else if (stroke_line_join == StrokeLineJoin::ROUND)
    {
        out << "round"sv;
    }
    return out;
}

template<typename Owner>
class PathProps
{
public:
    //Задаёт значение свойства fill — цвет заливки. По умолчанию свойство не выводится.
    Owner& SetFillColor(Color color)
    {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    //Задает значение свойства stroke — цвет контура. По умолчанию свойство не выводится.
    Owner& SetStrokeColor(Color color)
    {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    //Задаёт значение свойства stroke-width — толщину линии. По умолчанию свойство не выводится.
    Owner& SetStrokeWidth(double width)
    {
        stroke_width_ = width;
        return AsOwner();
    }

    //Задаёт значение свойства stroke-linecap — тип формы конца линии. По умолчанию свойство не выводится.
    Owner& SetStrokeLineCap(StrokeLineCap line_cap)
    {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }

    //Задаёт значение свойства stroke-linejoin — тип формы соединения линий. По умолчанию свойство не выводится.
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
    {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

protected:

    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const
    {
        using namespace std::literals;

        if (fill_color_ != std::nullopt)
        {
            out << " fill=\""sv << fill_color_.value() << "\" "sv;
        }
        if (stroke_color_ != std::nullopt)
        {
            out << " stroke=\""sv << stroke_color_.value() << "\" "sv;
        }
        if (stroke_width_ != std::nullopt)
        {
            out << " stroke-width=\""sv << stroke_width_.value() << "\" "sv;
        }
        if (stroke_line_cap_ != std::nullopt)
        {
            out << " stroke-linecap=\""sv << stroke_line_cap_.value() << "\" "sv;
        }
        if (stroke_line_join_ != std::nullopt)
        {
            out << " stroke-linejoin=\""sv << stroke_line_join_.value() << "\" "sv;
        }
    }

private:
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;

    Owner& AsOwner()
    {
        return static_cast<Owner&>(*this);
    }
};

struct Point
{
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y)
    {
    }
    double x = 0.0;
    double y = 0.0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext
{
    RenderContext(std::ostream& out)
        : out(out)
    {
    }


    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent)
    {
    }


    RenderContext Indented() const
    {
        return { out, indent_step, indent + indent_step };
    }

    void RenderIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object
{
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle final : public Object, public PathProps<Circle>
{
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_ = { 0.0, 0.0 };
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline>
{
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);
    //Возвращает координаты ломаной линии в виде строки
    std::string GetPointsLine() const;

private:
    std::vector<Point> points_;
    void RenderObject(const RenderContext& context) const override;
};


class Text final : public Object, public PathProps<Text>
{
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    Point position_ = { 0.0, 0.0 };
    Point offset_ = { 0.0, 0.0 };
    uint32_t font_size_ = 1;
    std::string data_;
    //следующие два значения по умолчанию не выводятся
    std::string font_family_;
    std::string font_weight_;

    void RenderObject(const RenderContext& context) const override;
};

//ObjectContainer задаёт интерфейс для доступа к контейнеру SVG-объектов. Через этот интерфейс Drawable-объекты могут визуализировать себя, добавляя в контейнер SVG-примитивы.
class ObjectContainer
{
public:

    template<typename Obj>
    void Add(Obj obj);

    virtual void AddPtr(std::unique_ptr<Object>&&) = 0;

protected:
    std::vector<std::unique_ptr<Object>> objects_;

    ~ObjectContainer() = default;
};

//Интерфейс Drawable унифицирует работу с объектами, которые можно нарисовать, подключив SVG-библиотеку. Для этого в нём есть метод Draw, принимающий ссылку на интерфейс ObjectContainer.
class Drawable
{
public:

    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};


class Document : public ObjectContainer
{
public:

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

};

}  //end namespace svg

template <typename Obj>
void svg::ObjectContainer::Add(Obj obj)
{
    objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
}
