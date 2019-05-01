#include <morphio/iterators.h>
#include <morphio/section.h>
#include <morphio/vasc/section.h>

namespace morphio {

class Section;

template <typename T>
Iterator<T>::Iterator(const Section& section)
{
    container.push(section);
}

template <>
breadth_iterator::Iterator(const Section& section)
{
    std::queue<Section> q;
    q.push(section);
    container.push(q);
}

template <typename T>
Iterator<T>::Iterator(const Morphology& morphology)
{
    auto roots = morphology.rootSections();
    for (auto it = roots.rbegin(); it != roots.rend(); ++it)
        container.push(*it);
}

template <>
breadth_iterator::Iterator(const Morphology& morphology)
{
    for (auto root : morphology.rootSections()) {
        std::queue<Section> q;
        q.push(root);
        container.push(q);
    }
}

graph_iterator::graph_iterator(const vasculature::Section& vasculatureSection)
{
    container.push(vasculatureSection);
}

graph_iterator::graph_iterator(const vasculature::Vasculature& vasculatureMorphology)
{
    auto sections = vasculatureMorphology.sections();
    for (std::size_t i = 0; i < sections.size(); ++i) {
        if (sections[i].predecessors().empty()) {
            container.push(sections[i]);
            visited.insert(sections[i]);
        }
    }
}

#pragma GCC diagnostic ignored "-Wunused-parameter"
template <>
upstream_iterator::Iterator(const Morphology& morphology)
{
} // Unused

template <typename T>
bool Iterator<T>::operator==(Iterator other) const
{
    return container == other.container;
}

template <typename T>
bool Iterator<T>::operator!=(Iterator other) const
{
    return !(*this == other);
}

bool graph_iterator::operator==(graph_iterator other) const
{
    return container == other.container;
}

bool graph_iterator::operator!=(graph_iterator other) const
{
    return !(*this == other);
}

template <typename T>
Iterator<T> Iterator<T>::operator++(int)
{
    Iterator retval = *this;
    ++(*this);
    return retval;
}

template <typename T>
Iterator<T>::Iterator()
{
}

graph_iterator::graph_iterator()
{
}

graph_iterator graph_iterator::operator++(int)
{
    graph_iterator retval = *this;
    ++(*this);
    return retval;
}

// Specializations
template <>
Section depth_iterator::operator*() const
{
    return container.top();
}
template <>
Section breadth_iterator::operator*() const
{
    return container.front().front();
}
template <>
Section upstream_iterator::operator*() const
{
    return container[0];
}

vasculature::Section graph_iterator::operator*() const
{
    return container.top();
}

template <>
upstream_iterator::Iterator(const Section& section)
{
    container.push_back(section);
}

template <>
depth_iterator& depth_iterator::operator++()
{
    const auto& section = *(*this);
    container.pop();
    auto& children = section.children();
    for (auto it = children.rbegin(); it != children.rend(); ++it)
        container.push(*it);
    return *this;
}

template <>
breadth_iterator& breadth_iterator::operator++()
{
    const auto& section = *(*this);
    container.front().pop();
    for (auto& child : section.children())
        container.front().push(child);
    if (container.front().empty())
        container.pop();

    return *this;
}

template <>
upstream_iterator& upstream_iterator::operator++()
{
    const auto& section = *(*this);
    if (section.isRoot())
        container.pop_back();
    else
        container[0] = section.parent();
    return *this;
}


graph_iterator& graph_iterator::operator++()
{
    const auto& section = *(*this);
    container.pop();
    auto& neighbors = section.neighbors();
    for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it)
        if (visited.find(*it) == visited.end()) {
            container.push(*it);
            visited.insert(*it);
        }
    return *this;
}

// Instantiations
template class Iterator<std::stack<Section>>;
template class Iterator<std::queue<std::queue<Section>>>;
template class Iterator<std::vector<Section>>;

} // namespace morphio
