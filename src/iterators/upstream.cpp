#include <morphio/iterators/upstream.h>
#include <morphio/section.h>

namespace morphio {

class Section;

upstream_iterator::upstream_iterator() : container()
{
}

upstream_iterator::upstream_iterator(const Section& section)
{
	container.push_back(section);
}

bool upstream_iterator::operator==(upstream_iterator other) const
{
    return container == other.container;
}

bool upstream_iterator::operator!=(upstream_iterator other) const
{
    return !(*this == other);
}

Section upstream_iterator::operator*() const
{
	return container.front();
}

upstream_iterator& upstream_iterator::operator++()
{
	const auto& section = *(*this);
	if (section.isRoot())
		container.pop_back();
	else
		container[0] = section.parent();
	return *this;
}
} // namespace morphio
