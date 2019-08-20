#include <morphio/iterators/mito_breadth.h>

namespace morphio {

mito_breadth_iterator::mito_breadth_iterator() : container()
{
}

mito_breadth_iterator::mito_breadth_iterator(const MitoSection& section)
{
	std::queue<MitoSection> q;
	q.push(section);
	container.push(q);
}

bool mito_breadth_iterator::operator==(mito_breadth_iterator other) const
{
    return container == other.container;
}

bool mito_breadth_iterator::operator!=(mito_breadth_iterator other) const
{
    return !(*this == other);
}

MitoSection mito_breadth_iterator::operator*() const
{
	return container.front().front();
}

mito_breadth_iterator& mito_breadth_iterator::operator++()
{
	const auto& section = *(*this);
	container.front().pop();
	for (auto& child : section.children())
		container.front().push(child);
	if (container.front().empty())
		container.pop();

	return *this;
}
} // namespace morphio
