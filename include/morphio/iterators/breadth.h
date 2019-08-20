#pragma once

#include <queue>

#include <morphio/types.h>
#include <morphio/section.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class breadth_iterator
{
	friend class Section;
	friend class Morphology;

	std::queue<std::queue<Section>> container;

public:
	breadth_iterator();
	breadth_iterator(const Section& section);
	breadth_iterator(const Morphology& morphology);
	bool operator==(breadth_iterator other) const;
	bool operator!=(breadth_iterator other) const;
	Section operator*() const;
	breadth_iterator& operator++();
	breadth_iterator operator++(int);
};

}
