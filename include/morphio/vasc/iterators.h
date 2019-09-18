
#include <set>
#include <stack>

namespace morphio {
namespace vasculature {

class Section;
class Vasculature;

class graph_iterator
{
    friend class Section;
    friend class Vasculature;

    std::set<Section> visited;
    std::stack<Section> container;

    graph_iterator() = default;

public:
    explicit graph_iterator(const Section& vasculatureSection);
    explicit graph_iterator(const Vasculature& vasculatureMorphology);
    bool operator==(graph_iterator other) const;
    bool operator!=(graph_iterator other) const;
    Section operator*() const;
    graph_iterator& operator++();
    graph_iterator operator++(int);
};
}  // namespace vasculature
}  // namespace morphio
