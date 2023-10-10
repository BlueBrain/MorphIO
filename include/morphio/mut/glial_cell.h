/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <string>  // std::string

#include <morphio/mut/morphology.h>  // mut::Morphology

namespace morphio {
namespace mut {

/** Mutable(editable) morphio::GlialCell */
class GlialCell: public Morphology
{
  public:
    GlialCell();
    explicit GlialCell(const std::string& source);
};

}  // namespace mut
}  // namespace morphio
