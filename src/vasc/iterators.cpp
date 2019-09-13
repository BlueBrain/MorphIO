#include <cstddef>  // std::size_t
#include <morphio/vasc/section.h>


namespace morphio {
namespace vasculature {
graph_iterator::graph_iterator(const Section& vasculatureSection)
{
    container.push(vasculatureSection);
}

graph_iterator::graph_iterator(const Vasculature& vasculatureMorphology)
{
    auto sections = vasculatureMorphology.sections();
    for (std::size_t i = 0; i < sections.size(); ++i) {
        if (sections[i].predecessors().empty()) {
            container.push(sections[i]);
            visited.insert(sections[i]);
        }
    }
}
bool graph_iterator::operator==(graph_iterator other) const
{
    return container == other.container;
}

bool graph_iterator::operator!=(graph_iterator other) const
{
    return !(*this == other);
}

Section graph_iterator::operator*() const
{
    return container.top();
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

graph_iterator graph_iterator::operator++(int)
{
    graph_iterator retval = *this;
    ++(*this);
    return retval;
}
}  // namespace vasculature
}  // namespace morphio
