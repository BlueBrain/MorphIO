#include <morphio/section.h>
#include <morphio/tools.h>
#include <morphio/vector_types.h>

namespace morphio {


template <typename Family>
typename Family::Type Node<Family>::type() const {
    return static_cast<typename Family::Type>(
        this -> _properties-> template get<Property::SectionType>()[0]
        );
}

template <typename Family>
depth_iterator_t<Node<Family>> Node<Family>::depth_begin() const {
    return depth_iterator_t<Node<Family>>(*this);
}

template <typename Family>
depth_iterator_t<Node<Family>> Node<Family>::depth_end() const {
    return depth_iterator_t<Node<Family>>();
}

template <typename Family>
breadth_iterator_t<Node<Family>> Node<Family>::breadth_begin() const {
    return breadth_iterator_t<Node<Family>>(*this);
}

template <typename Family>
breadth_iterator_t<Node<Family>> Node<Family>::breadth_end() const {
    return breadth_iterator_t<Node<Family>>();
}

template <typename Family>
upstream_iterator_t<Node<Family>> Node<Family>::upstream_begin() const {
    return upstream_iterator_t<Node<Family>>(*this);
}

template <typename Family>
upstream_iterator_t<Node<Family>> Node<Family>::upstream_end() const {
    return upstream_iterator_t<Node<Family>>();
}

template <typename Family>
range<const Point> Node<Family>::points() const {
    return this-> template get<Property::Point>();
}

template <typename Family>
range<const floatType> Node<Family>::diameters() const {
    return this -> template get<Property::Diameter>();
}

template <typename Family>
range<const floatType> Node<Family>::perimeters() const {
    return this -> template get<Property::Perimeter>();
}

template class Node<CellFamily::NEURON>;
template class Node<CellFamily::GLIA>;

}  // namespace morphio

template <typename Family>
std::ostream& operator<<(std::ostream& os, const morphio::Node<Family>& section) {
    const auto& points = section.points();
    if (points.empty()) {
        os << "Section(id=" << section.id() << ", points=[])";
    } else {
        os << "Section(id=" << section.id() << ", points=[(" << points[0] << "),..., (";
        os << points[points.size() - 1] << ")])";
    }
    return os;
}

template std::ostream& operator<<(std::ostream& os, const morphio::NeuronalSection& section);
template std::ostream& operator<<(std::ostream& os, const morphio::GlialSection& section);


// operator<< must be defined in the global namespace to be usable there
std::ostream& operator<<(std::ostream& os, const morphio::range<const morphio::Point>& points) {
    for (const auto& point : points) {
        os << point[0] << ' ' << point[1] << ' ' << point[2] << '\n';
    }
    return os;
}
