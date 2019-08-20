#pragma once

#include <stack>

#include <morphio/types.h>
#include <morphio/mito_section.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class mito_depth_iterator
{
	friend class MitoSection;

	std::stack<MitoSection> container;

public:
	mito_depth_iterator();
	mito_depth_iterator(const MitoSection& section);
	bool operator==(mito_depth_iterator other) const;
	bool operator!=(mito_depth_iterator other) const;
	MitoSection operator*() const;
	mito_depth_iterator& operator++();
	mito_depth_iterator operator++(int);
};

}
