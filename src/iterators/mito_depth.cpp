#include <morphio/iterators/mito_depth.h>

namespace morphio {

mito_depth_iterator::mito_depth_iterator() : container()
{
}

mito_depth_iterator::mito_depth_iterator(const MitoSection& section)
{
	container.push(section);
}

bool mito_depth_iterator::operator==(mito_depth_iterator other) const
{
    return container == other.container;
}

bool mito_depth_iterator::operator!=(mito_depth_iterator other) const
{
    return !(*this == other);
}

MitoSection mito_depth_iterator::operator*() const
{
	return container.top();
}

mito_depth_iterator& mito_depth_iterator::operator++()
{
	const auto& section = *(*this);
	container.pop();
	auto& children = section.children();
	for (auto it = children.rbegin(); it != children.rend(); ++it)
		container.push(*it);
	return *this;
}
} // namespace morphio
