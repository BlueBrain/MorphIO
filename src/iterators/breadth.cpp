#include <morphio/iterators/breadth.h>
#include <morphio/section.h>

namespace morphio {

class Section;

breadth_iterator::breadth_iterator() : container()
{
}

breadth_iterator::breadth_iterator(const Section& section)
{
	std::queue<Section> q;
	q.push(section);
	container.push(q);
}

breadth_iterator::breadth_iterator(const Morphology& morphology)
{
	for (auto root : morphology.rootSections()) {
		std::queue<Section> q;
		q.push(root);
		container.push(q);
	}
}

bool breadth_iterator::operator==(breadth_iterator other) const
{
    return container == other.container;
}

bool breadth_iterator::operator!=(breadth_iterator other) const
{
    return !(*this == other);
}

Section breadth_iterator::operator*() const
{
	return container.front().front();
}

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
} // namespace morphio
