#pragma once

#include <vector>

#include <morphio/types.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class upstream_iterator
{
	friend class Section;

	std::vector<Section> container;

public:
	upstream_iterator();
	upstream_iterator(const Section& section);
	bool operator==(upstream_iterator other) const;
	bool operator!=(upstream_iterator other) const;
	Section operator*() const;
	upstream_iterator& operator++();
	upstream_iterator operator++(int);
};

}
