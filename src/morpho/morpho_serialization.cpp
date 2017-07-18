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

#include <morpho/morpho_tree.hpp>

#include <cereal/details/helpers.hpp>

template <class T>
inline cereal::NameValuePair<T> make_nvp(const std::string& name, T&& value) {
    return {name.c_str(), std::forward<T>(value)};
}

//
// morpho_tree serialization functions
//

template <class Archive>
void save(Archive& archive, morpho::morpho_tree const& tree,
          std::uint32_t const version) {
    (void)version; // unused parameter
    archive(make_nvp("nodes", tree.get_nodes()),
            make_nvp("parents", tree.get_parents()));
}

template <class Archive>
void load(Archive& archive, morpho::morpho_tree& tree,
          std::uint32_t const version) {
    if (version == 0) {
        morpho::const_morpho_nodes_t nodes;
        std::vector<int> parents;
        archive(make_nvp("nodes", nodes), make_nvp("parents", parents));
        const auto nodes_size = nodes.size();
        for (size_t i = 0; i < nodes_size; ++i) {
            tree.add_node(
                parents[i],
                std::const_pointer_cast<morpho::morpho_node>(nodes[i]));
        }
    } else {
        throw std::invalid_argument("Unknown serialization version");
    }
}

//
// point serialization function
//

template <class Archive>
void serialize(Archive& archive, morpho::point& point) {
    archive(make_nvp("x", point(0)), make_nvp("y", point(1)),
            make_nvp("z", point(2)));
}

//
// neuron_soma serialization functions
//

enum soma_type : unsigned char { soma_sphere = '\0', soma_line_loop = '\1' };

template <class Archive>
void save(Archive& archive, morpho::neuron_soma const& n,
          std::uint32_t const version) {
    (void)version; // unused parameter
    soma_type kind = soma_sphere;
    auto line_loop = n.get_line_loop();
    if (line_loop.size() == 1) {
        archive(make_nvp("kind", kind), make_nvp("center", line_loop[0]),
                make_nvp("radius", n.get_sphere().get_radius()));
    } else {
        kind = soma_line_loop;
        archive(make_nvp("kind", kind), make_nvp("line_loop", line_loop));
    }
}

template <class Archive>
void load(Archive& archive, morpho::neuron_soma& n,
          std::uint32_t const version) {
    if (version == 0) {
        soma_type kind;
        archive(make_nvp("kind", kind));
        if (kind == soma_sphere) {
            morpho::point center;
            double radius;
            archive(make_nvp("center", center), make_nvp("radius", radius));
            n.set_sphere(center, radius);
        } else if (kind == soma_line_loop) {
            std::vector<morpho::point> line_loop;
            archive(make_nvp("line_loop", line_loop));
            n.set_line_loop(std::move(line_loop));
        } else {
            throw std::runtime_error("Unknown soma type: " + kind);
        }
    } else {
        throw std::runtime_error("Unknown neuron_soma serialization version");
    }
}

//
// neuron_section serialization
//

template <class Archive>
void serialize(Archive& archive, morpho::neuron_section& section,
               std::uint32_t const version) {
    if (version == 0) {
        archive(make_nvp("type", section.get_section_type()),
                make_nvp("points", section.get_points()),
                make_nvp("radius", section.get_radius()));
    } else {
        throw std::runtime_error("Unknown neuron_section serialization version");
    }
}

// Those headers MUST be include AFTER serialization functions above

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>
#include <morpho/morpho_serialization.hpp>

CEREAL_REGISTER_TYPE_WITH_NAME(morpho::neuron_soma, "soma");
CEREAL_REGISTER_TYPE_WITH_NAME(morpho::neuron_section, "section");
CEREAL_REGISTER_POLYMORPHIC_RELATION(morpho::morpho_node, morpho::neuron_soma);
CEREAL_REGISTER_POLYMORPHIC_RELATION(morpho::morpho_node,
                                     morpho::neuron_section);

CEREAL_CLASS_VERSION(morpho::morpho_tree, 0);
CEREAL_CLASS_VERSION(morpho::neuron_soma, 0);
CEREAL_CLASS_VERSION(morpho::neuron_section, 0);

namespace morpho {

template <class Archive>
void save_morpho(const morpho::morpho_tree& tree, std::ostream& ostr) {
    Archive archive(ostr);
    archive(tree);
}

template <typename Archive>
morpho::morpho_tree load_morpho(std::istream& istr) {
    Archive archive(istr);
    morpho::morpho_tree tree;
    archive(tree);
    return tree;
}

void serialize(const morpho_tree& tree, std::ostream& stream,
               const serialization_format& format) {
    switch (format) {
    case BINARY:
        save_morpho<cereal::BinaryOutputArchive>(tree, stream);
        break;
    case PORTABLE_BINARY:
        save_morpho<cereal::PortableBinaryOutputArchive>(tree, stream);
        break;
    case XML:
        save_morpho<cereal::XMLOutputArchive>(tree, stream);
        break;
    case JSON:
        save_morpho<cereal::JSONOutputArchive>(tree, stream);
        break;
    default:
        throw std::invalid_argument("Unexpected serialization format: " +
                                    format);
    }
}

morpho_tree deserialize(std::istream& stream,
                        const serialization_format& format) {
    switch (format) {
    case BINARY:
        return load_morpho<cereal::BinaryInputArchive>(stream);
    case PORTABLE_BINARY:
        return load_morpho<cereal::PortableBinaryInputArchive>(stream);
    case XML:
        return load_morpho<cereal::XMLInputArchive>(stream);
    case JSON:
        return load_morpho<cereal::JSONInputArchive>(stream);
    default:
        throw std::invalid_argument("Unexpected serialization format: " +
                                    format);
    }
}

} // namespace morpho
