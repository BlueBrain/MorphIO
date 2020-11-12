#pragma once

#include <morphio/morphology.h>
#include <morphio/types.h>

namespace morphio {
class  Spine: public Morphology {
public:
    Spine(const std::string& source);

private:
    Soma soma() const;

};
}
