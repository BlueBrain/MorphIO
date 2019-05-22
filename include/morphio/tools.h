#include <morphio/types.h>

namespace morphio
{

bool diff(const Morphology& left, const Morphology& right, bool verbose=true);
bool diff(const Section& left, const Section& right, bool verbose=true);

namespace mut
{
bool diff(const Morphology& left, const Morphology& right, bool verbose=true);
bool diff(const Section& left, const Section& right, bool verbose=true);
}

}
