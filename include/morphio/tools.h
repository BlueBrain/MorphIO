#include <morphio/types.h>

namespace morphio
{

/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, morphio::enums::LogLevel verbose=morphio::enums::LogLevel::INFO);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, morphio::enums::LogLevel verbose=morphio::enums::LogLevel::INFO);

namespace mut
{
/**
   Perform a diff on 2 morphologies
**/
bool diff(const Morphology& left, const Morphology& right, morphio::enums::LogLevel verbose=morphio::enums::LogLevel::INFO);

/**
   Perform a diff on 2 sections
**/
bool diff(const Section& left, const Section& right, morphio::enums::LogLevel verbose=morphio::enums::LogLevel::INFO);
}

}
