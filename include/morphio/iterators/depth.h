#pragma once

#include <stack>

#include <morphio/types.h>
#include <morphio/section.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class depth_iterator
{
	friend class Section;

	std::stack<Section> container;

public:
	depth_iterator();
	depth_iterator(const Section& section);
	depth_iterator(const Morphology& morphology);
	bool operator==(depth_iterator other) const;
	bool operator!=(depth_iterator other) const;
	Section operator*() const;
	depth_iterator& operator++();
	depth_iterator operator++(int);
};

}
