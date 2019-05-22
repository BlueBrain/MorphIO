#include <morphio/types.h>

namespace morphio
{

/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, bool verbose=true);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, bool verbose=true);

namespace mut
{
/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, bool verbose=true);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, bool verbose=true);
}

}
