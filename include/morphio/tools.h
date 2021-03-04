#pragma once

#include <morphio/types.h>

namespace morphio {

/**
   Perform a diff on 2 morphologies, returns True if items differ
**/
bool diff(const TMorphology& left,
          const TMorphology& right,
          morphio::enums::LogLevel verbose = morphio::enums::LogLevel::INFO);

/**
   Perform a diff on 2 sections, returns True if items differ
**/
bool diff(const Section& left,
          const Section& right,
          morphio::enums::LogLevel verbose = morphio::enums::LogLevel::INFO);

namespace mut {
/**
   Perform a diff on 2 morphologies, returns True if items differ
**/
template <typename Type>
bool diff(const TMorphology<Type>& left,
          const TMorphology<Type>& right,
          morphio::enums::LogLevel verbose = morphio::enums::LogLevel::INFO);

/**
   Perform a diff on 2 sections, returns True if items differ
**/
bool diff(const Section& left,
          const Section& right,
          morphio::enums::LogLevel verbose = morphio::enums::LogLevel::INFO);
}  // namespace mut

}  // namespace morphio
