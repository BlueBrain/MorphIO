#include <morphio/iterators/mito_upstream.h>
#include <morphio/mito_section.h>

namespace morphio {

mito_upstream_iterator::mito_upstream_iterator() : container()
{
}

mito_upstream_iterator::mito_upstream_iterator(const MitoSection& section)
{
	container.push_back(section);
}

bool mito_upstream_iterator::operator==(mito_upstream_iterator other) const
{
    return container == other.container;
}

bool mito_upstream_iterator::operator!=(mito_upstream_iterator other) const
{
    return !(*this == other);
}

MitoSection mito_upstream_iterator::operator*() const
{
	return container.front();
}

mito_upstream_iterator& mito_upstream_iterator::operator++()
{
	const auto& section = *(*this);
	if (section.isRoot())
		container.pop_back();
	else
		container[0] = section.parent();
	return *this;
}
} // namespace morphio
