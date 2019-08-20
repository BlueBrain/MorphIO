#pragma once

#include <vector>

#include <morphio/types.h>

namespace morphio {

/**
	 An iterator class to iterate through sections;
**/
class mito_upstream_iterator
{
	friend class MitoSection;

	std::vector<MitoSection> container;

public:
	mito_upstream_iterator();
	mito_upstream_iterator(const MitoSection& section);
	bool operator==(mito_upstream_iterator other) const;
	bool operator!=(mito_upstream_iterator other) const;
	MitoSection operator*() const;
	mito_upstream_iterator& operator++();
	mito_upstream_iterator operator++(int);
};

}
