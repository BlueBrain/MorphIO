#pragma once

#include <queue>

#include <morphio/types.h>
#include <morphio/mito_section.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class mito_breadth_iterator
{
	friend class MitoSection;

	std::queue<std::queue<MitoSection>> container;

public:
	mito_breadth_iterator();
	mito_breadth_iterator(const MitoSection& section);
	mito_breadth_iterator(const Morphology& morphology);
	bool operator==(mito_breadth_iterator other) const;
	bool operator!=(mito_breadth_iterator other) const;
	MitoSection operator*() const;
	mito_breadth_iterator& operator++();
	mito_breadth_iterator operator++(int);
};

}
