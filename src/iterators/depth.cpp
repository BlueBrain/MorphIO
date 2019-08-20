#include <morphio/iterators/depth.h>
#include <morphio/section.h>

namespace morphio {

class Section;

depth_iterator::depth_iterator() : container()
{
}

depth_iterator::depth_iterator(const Section& section)
{
	container.push(section);
}

depth_iterator::depth_iterator(const Morphology& morphology)
{
    auto roots = morphology.rootSections();
    for (auto it = roots.rbegin(); it != roots.rend(); ++it)
        container.push(*it);
}

bool depth_iterator::operator==(depth_iterator other) const
{
    return container == other.container;
}

bool depth_iterator::operator!=(depth_iterator other) const
{
    return !(*this == other);
}

Section depth_iterator::operator*() const
{
	return container.top();
}

depth_iterator& depth_iterator::operator++()
{
	const auto& section = *(*this);
	container.pop();
	auto& children = section.children();
	for (auto it = children.rbegin(); it != children.rend(); ++it)
		container.push(*it);
	return *this;
}
} // namespace morphio
