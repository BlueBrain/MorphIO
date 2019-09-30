
#include <set>
#include <stack>

namespace morphio {
namespace vasculature {

template<typename SectionT, typename VasculatureT>
class graph_iterator_t
{
    std::set<SectionT> visited;
    std::stack<SectionT> container;

public:
graph_iterator_t() = default;

explicit graph_iterator_t(const SectionT& vasculatureSection)
{
    container.push(vasculatureSection);
}

explicit graph_iterator_t(const VasculatureT& vasculatureMorphology)
{
    auto sections = vasculatureMorphology.sections();
    for (std::size_t i = 0; i < sections.size(); ++i) {
        if (sections[i].predecessors().empty()) {
            container.push(sections[i]);
            visited.insert(sections[i]);
        }
    }
}

bool operator==(const graph_iterator_t& other) const
{
    return container == other.container;
}

bool operator!=(const graph_iterator_t& other) const
{
    return !(*this == other);
}

SectionT operator*() const
{
    return container.top();
}

graph_iterator_t& operator++()
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
graph_iterator_t operator++(int)
{
    graph_iterator_t retval = *this;
    ++(*this);
    return retval;
}
};
}  // namespace vasculature
}  // namespace morphio
