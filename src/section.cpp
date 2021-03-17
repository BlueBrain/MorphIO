#include <morphio/section.h>
#include <morphio/tools.h>
#include <morphio/vector_types.h>

namespace morphio {


template <typename CellType>
typename CellType::Type Node<CellType>::type() const {
    return static_cast<typename CellType::Type>(
        this -> _properties-> template get<Property::SectionType>()[this->_id]
        );
}

template <typename CellType>
depth_iterator_t<Node<CellType>> Node<CellType>::depth_begin() const {
    return depth_iterator_t<Node<CellType>>(*this);
}

template <typename CellType>
depth_iterator_t<Node<CellType>> Node<CellType>::depth_end() const {
    return depth_iterator_t<Node<CellType>>();
}

template <typename CellType>
breadth_iterator_t<Node<CellType>> Node<CellType>::breadth_begin() const {
    return breadth_iterator_t<Node<CellType>>(*this);
}

template <typename CellType>
breadth_iterator_t<Node<CellType>> Node<CellType>::breadth_end() const {
    return breadth_iterator_t<Node<CellType>>();
}

template <typename CellType>
upstream_iterator_t<Node<CellType>> Node<CellType>::upstream_begin() const {
    return upstream_iterator_t<Node<CellType>>(*this);
}

template <typename CellType>
upstream_iterator_t<Node<CellType>> Node<CellType>::upstream_end() const {
    return upstream_iterator_t<Node<CellType>>();
}

template <typename CellType>
range<const Point> Node<CellType>::points() const {
    return this-> template get<Property::Point>();
}

template <typename CellType>
range<const floatType> Node<CellType>::diameters() const {
    return this -> template get<Property::Diameter>();
}

template <typename CellType>
range<const floatType> Node<CellType>::perimeters() const {
    return this -> template get<Property::Perimeter>();
}

template class Node<CellFamily::NEURON>;
template class Node<CellFamily::GLIA>;

}  // namespace morphio

template <typename CellType>
std::ostream& operator<<(std::ostream& os, const morphio::Node<CellType>& section) {
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
