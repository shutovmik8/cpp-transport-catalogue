#include "svg.h"

#include <string>
#include <variant>

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    Circle::RenderAttrs(out);
    out << "/>"sv;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    int size = points_.size();
    for (int i = 0; i < size; ++i) {
        out << points_[i].x << "," << points_[i].y;
        if (i != size - 1) {
            out << " ";
        }
    }
    out << "\""sv;
    Polyline::RenderAttrs(out);
    out << "/>"sv;
}
    
Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}
    
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";
    Text::RenderAttrs(out);
    out << " x=\"" << pos_.x << "\" y=\"" << pos_.y << "\" dx=\"" << offset_.x << "\" dy=\"" << offset_.y << "\" ";
    out << "font-size=\"" << size_ << "\"";
    if (font_family_ != "") {
        out << " font-family=\"" << font_family_ << "\"";
    }
    if (font_weight_ != "") {
        out << " font-weight=\"" << font_weight_ << "\"";
    }
    out << ">";
    for (const auto& symbol : data_) {
        if (symbol == '\"') {
            out << "&quot;";
            continue;
        }
        if (symbol == '\'') {
            out << "&apos;";
            continue;
        }
        if (symbol == '<') {
            out << "&lt;";
            continue;
        }
        if (symbol == '>') {
            out << "&gt;";
            continue;
        }
        if (symbol == '&') {
            out << "&amp;";
            continue;
        }
        out << symbol;
    }
    out << "</text>";
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}    
    
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
} 
    
Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}
    
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}
    
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}
    
Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}
    
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.push_back(std::move(obj));
}   

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;
    for (const auto& item : objects_) {
        item->Render(RenderContext(out, 0, 2));
    }
    out << "</svg>"sv;
}
    
std::ostream& operator<<(std::ostream& out, const StrokeLineCap& item) {
    if (item == StrokeLineCap::BUTT) {
        out << "butt";
    }
    else if (item == StrokeLineCap::ROUND) {
        out << "round";
    }
    else if (item == StrokeLineCap::SQUARE) {
        out << "square";
    }
    return out;
}
   
std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& item) {
    if (item == StrokeLineJoin::ARCS) {
        out << "arcs";
    }
    else if (item == StrokeLineJoin::BEVEL) {
        out << "bevel";
    }
    else if (item == StrokeLineJoin::MITER) {
        out << "miter";
    }
    else if (item == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip";
    }
    else if (item == StrokeLineJoin::ROUND) {
        out << "round";
    }
    return out;
}

struct SolutionPrinter {
    SolutionPrinter(std::ostream& out = std::cout) : out_(out) {}
    
    std::ostream& out_;
    
    void operator()(std::monostate) const {
        out_ << "none"sv;
    }
    void operator()(const std::string& item) const {
        out_ << item;
    }
    void operator()(const Rgb& item) const {
        out_ << "rgb(" << static_cast<int>(item.red) << "," << static_cast<int>(item.green) << "," << static_cast<int>(item.blue) << ")";
    }
    void operator()(const Rgba& item) const {
        out_ << "rgba(" << static_cast<int>(item.red) << "," << static_cast<int>(item.green) << "," << static_cast<int>(item.blue) << "," << item.opacity << ")";
    }
};
    
std::ostream& operator<<(std::ostream& out, const Color& item) {
    std::visit(SolutionPrinter{out}, item);
    return out;
}
    
}  // namespace svg