#include <morphio/types.h>

namespace morphio
{

/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, int verbose=3);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, int verbose=3);

namespace mut
{
/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, int verbose=3);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, int verbose=3);
}

}
