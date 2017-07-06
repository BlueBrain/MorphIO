/*
 * Copyright (C) 2017 Tristan Carel <tristan.carel@epfl.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <fstream>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <hadoken/format/format.hpp>
#include <morpho/morpho_swc.hpp>

namespace morpho {

namespace swc_v1 {

using hadoken::format::scat;

typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
typedef tokenizer::iterator iterator;

const char COMMENT_CHARACTER = '#';

morpho_reader::morpho_reader(const std::string& filename)
    : filename(filename) {}

static const iterator& ensure_has_token(const iterator& token,
                                        const tokenizer& tokenizer,
                                        size_t linenum) {

    if (token == tokenizer.end()) {
        throw std::runtime_error(
            scat("Parse error line ", linenum, ": unexpected EOL"));
    }
    return token;
}

/**
 * Build a morpho_tree from a SWC filename
 * see
 * http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 * for format specifications
 * @return loaded morphology
 */
morpho_tree morpho_reader::create_morpho_tree() const {
    morpho_tree eax;

    const boost::char_separator<char> sep(" ");
    std::ifstream istr(filename);
    std::string line;
    std::size_t linenum = 0;
    bool root = true;

    while (std::getline(istr, line)) {
        ++linenum;
        if (line.size() > 0 && line[0] == COMMENT_CHARACTER) {
            continue;
        }
        point position;
        double radius;
        tokenizer tok(line, sep);
        tokenizer::iterator token = tok.begin();

        if (token == tok.end())
            continue;
        // first field is not used (Sample number)
        ensure_has_token(++token, tok, linenum);
        const int structure_identifier = std::stoi(*token);
        ensure_has_token(++token, tok, linenum);
        const double x = std::atof(token->c_str());
        ensure_has_token(++token, tok, linenum);
        const double y = std::atof(token->c_str());
        ensure_has_token(++token, tok, linenum);
        const double z = std::atof(token->c_str());
        position = point(x, y, z);
        ensure_has_token(++token, tok, linenum);
        radius = std::atof(token->c_str());
        ensure_has_token(++token, tok, linenum);
        int parent = std::stoi(*token) - 1; // index starts at 1 in SWC
        if (parent == -2) {                 // root
            parent = -1;
        }
        ++token;
        if (token != tok.end()) {
            throw std::runtime_error(scat("Parse error line ", linenum,
                                          ". Expected EOL but found '", *token,
                                          '\''));
        }
        switch (structure_identifier) {
        case 0: // undefined
            break;
        case 1: // soma
        {
            if (root) {
                eax.add_node(parent,
                             std::make_shared<neuron_soma>(position, radius));
                root = false;
            } else {
                eax.add_node(parent, std::make_shared<neuron_section>(
                                         neuron_struct_type::soma,
                                         std::vector<point>({position}),
                                         std::vector<double>({radius})));
            }
        } break;
        case 2: // axon
            eax.add_node(parent, std::make_shared<neuron_section>(
                                     neuron_struct_type::axon,
                                     std::vector<point>({position}),
                                     std::vector<double>({radius})));
            break;
        case 3: // basal dendrite
            eax.add_node(parent, std::make_shared<neuron_section>(
                                     neuron_struct_type::dentrite_basal,
                                     std::vector<point>({position}),
                                     std::vector<double>({radius})));
            break;
        case 4: // apical dendrite
            eax.add_node(parent, std::make_shared<neuron_section>(
                                     neuron_struct_type::dentrite_basal,
                                     std::vector<point>({position}),
                                     std::vector<double>({radius})));
            break;
        default:
            throw std::runtime_error(scat("Parser error line ", linenum,
                                          ". Unknown structure identifier ",
                                          structure_identifier));
        }
    }
    return eax;
}

} // namespace morpho
} // namespace swc_v1
