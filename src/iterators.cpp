#include <iterators.h>
#include <section.h>

namespace minimorph
{
class Section;

template <typename T> Iterator<T>::Iterator(const Section& section) {
    container.push(section);
}

template <typename T> bool Iterator<T>::operator==(Iterator other) const {
    return container == other.container;
}

template <typename T> bool Iterator<T>::operator!=(Iterator other) const {
    return !(*this == other);
}

template <typename T> Iterator<T>& Iterator<T>::operator++()
{
    const auto& section = *(*this);
    container.pop();
    for(const auto& child: section.getChildren())
        container.push(child);
    return *this;
}

template <typename T> Iterator<T> Iterator<T>::operator++(int){
    Iterator retval = *this;
    ++(*this);
    return retval;
}

// Specializations
template <> Section depth_iterator::operator*() const { return container.top();}
template <> Section breadth_iterator::operator*() const { return container.front();}

template <> upstream_iterator::Iterator(const Section& section) { container = std::make_shared<Section>(section); }
template <> Section upstream_iterator::operator*() const { return *container;}
template <> upstream_iterator& upstream_iterator::operator++(){
    const auto& section = *(*this);
    container = section.getParent();
    return *this;
}

// Instantiations
template class Iterator<std::stack<Section>>;
template class Iterator<std::queue<Section>>;
template class Iterator<std::shared_ptr<Section>>;
}
