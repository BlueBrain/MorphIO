#pragma once

#include <fstream>
#include <morphio/types.h>

namespace morphio {

namespace readers
{
namespace h5
{
Property::Properties load(const std::string& uri);
Property::Properties load(const HighFive::Group& group);
}

namespace swc
{
Property::Properties load(const std::string& uri, unsigned int options);
}

namespace asc
{
Property::Properties load(const std::string& uri, unsigned int options);
}
}


void buildChildren(std::shared_ptr<Property::Properties> properties);


Property::Properties loadURI(const std::string& source, unsigned int options);




/** Read access a TTree file.
 *
 * Following RAII, this class is ready to use after the creation and will ensure
 * release of resources upon destruction.
 */
template <typename Node, typename CRTP, typename Mut>
class TTree
{
  public:
    using breadth_iterator = breadth_iterator_t<Node>;
    using depth_iterator = depth_iterator_t<Node>;

    ~TTree();

    TTree& operator=(const TTree<Node, CRTP, Mut>&);
    TTree(TTree<Node, CRTP, Mut>&&) noexcept;
    TTree& operator=(TTree<Node, CRTP, Mut>&&) noexcept;

    /** @name Read API */
    //@{
    /** Open the given source to a TTree file and parse it.

        options is the modifier flags to be applied. All flags are defined in
       their enum: morphio::enum::Option and can be composed.

        Example:
            TTree("neuron.asc", TWO_POINTS_SECTIONS | SOMA_SPHERE);
     */
    TTree(const std::string& source, unsigned int options = NO_MODIFIER);
    TTree(const HighFive::Group& group, unsigned int options = NO_MODIFIER);
    TTree(Mut);

    /**
     * Return a vector of all root sections
     * (sections whose parent ID are -1)
     **/
    std::vector<Node> rootSections() const;

    /**
     * Return a vector containing all section objects.
     **/
    std::vector<Node> sections() const;

    /**
     * Return the Section with the given id.
     *
     * @throw RawDataError if the id is out of range
     */
    Node section(uint32_t id) const;

    /**
     * Return a vector with all points from all sections
     * (soma points are not included)
     **/
    const Points& points() const noexcept;

    /**
     * Returns a list with offsets to access data of a specific section in the points
     * and diameters arrays.
     *
     * Example: accessing diameters of n'th section will be located in the DIAMETERS
     * array from DIAMETERS[sectionOffsets(n)] to DIAMETERS[sectionOffsets(n+1)-1]
     *
     * Note: for convenience, the last point of this array is the points() array size
     * so that the above example works also for the last section.
     **/
    std::vector<uint32_t> sectionOffsets() const;

    /**
     * Return a vector with all diameters from all sections
     * (soma points are not included)
     **/
    const std::vector<morphio::floatType>& diameters() const;

    /**
     * Return a vector with all perimeters from all sections
     **/
    const std::vector<morphio::floatType>& perimeters() const;

    /**
     * Return a vector with the section type of every section
     **/
    const std::vector<typename Node::Type> sectionTypes() const;

    /**
     * Return the graph connectivity of the TTree where each section
     * is seen as a node
     * Note: -1 is the soma node
     **/
    const std::map<int, std::vector<unsigned int>>& connectivity() const;


    /**
       Depth first iterator starting at a given section id

       If id == -1, the iteration will start at each root section, successively
    **/
    depth_iterator depth_begin() const;
    depth_iterator depth_end() const;
    /**
       Breadth first iterator

       If id == -1, the iteration will be successively performed starting
       at each root section
    **/
    breadth_iterator breadth_begin() const;
    breadth_iterator breadth_end() const;

    /**
     * Return the version
     **/
    const MorphologyVersion& version() const;

  protected:
    friend class mut::Morphology;
    friend class mut::GlialCell;
    TTree(const Property::Properties& properties, unsigned int options);

    std::shared_ptr<Property::Properties> _properties;

    template <typename Property>
    const std::vector<typename Property::Type>& get() const;
};


template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::TTree(const Property::Properties& properties, unsigned int options)
    : _properties(std::make_shared<Property::Properties>(properties)) {
    buildChildren(_properties);
    // For SWC and ASC, sanitization and modifier application are already taken care of by
    // their respective loaders
    if (properties._cellLevel.fileFormat() == "h5") {
        Mut mutable_morph(*static_cast<CRTP*>(this));
        mutable_morph.sanitize();
        if (options) {
            mutable_morph.applyModifiers(options);
        }
        _properties = std::make_shared<Property::Properties>(mutable_morph.buildReadOnly());
        buildChildren(_properties);
    }
}

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::TTree(const HighFive::Group& group, unsigned int options)
    : TTree(readers::h5::load(group), options) {}

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::TTree(const std::string& source, unsigned int options)
    : TTree(loadURI(source, options), options) {}

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::TTree(Mut morphology) {
    morphology.sanitize();
    _properties = std::make_shared<Property::Properties>(morphology.buildReadOnly());
    buildChildren(_properties);
}

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::TTree(TTree&&) noexcept = default;

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>& TTree<Node, CRTP, Mut>::operator=(TTree&&) noexcept = default;

template <typename Node, typename CRTP, typename Mut>
Node TTree<Node, CRTP, Mut>::section(uint32_t id) const {
    return {id, _properties};
}

template <typename Node, typename CRTP, typename Mut>
TTree<Node, CRTP, Mut>::~TTree() = default;


template <typename Node, typename CRTP, typename Mut>
std::vector<Node> TTree<Node, CRTP, Mut>::rootSections() const {
    std::vector<Node> result;
    try {
        const std::vector<uint32_t>& children =
            _properties->children<morphio::Property::Section>().at(-1);
        result.reserve(children.size());
        for (auto id : children) {
            result.push_back(section(id));
        }

        return result;
    } catch (const std::out_of_range&) {
        return result;
    }
}

template <typename Node, typename CRTP, typename Mut>
std::vector<Node> TTree<Node, CRTP, Mut>::sections() const {
    // TODO: Make this more performant when needed
    std::vector<Node> sections_;
    auto count = _properties->get<morphio::Property::Section>().size();
    sections_.reserve(count);
    for (unsigned int i = 0; i < count; ++i) {
        sections_.emplace_back(section(i));
    }
    return sections_;
}

template <typename Node, typename CRTP, typename Mut>
template <typename Property>
const std::vector<typename Property::Type>& TTree<Node, CRTP, Mut>::get() const {
    return _properties->get<Property>();
}

template <typename Node, typename CRTP, typename Mut>
const Points& TTree<Node, CRTP, Mut>::points() const noexcept {
    return get<Property::Point>();
}

template <typename Node, typename CRTP, typename Mut>
std::vector<uint32_t> TTree<Node, CRTP, Mut>::sectionOffsets() const {
    const std::vector<Property::Section::Type>& indices_and_parents = get<Property::Section>();
    auto size = indices_and_parents.size();
    std::vector<uint32_t> indices(size + 1);
    std::transform(indices_and_parents.begin(),
                   indices_and_parents.end(),
                   indices.begin(),
                   [](const Property::Section::Type& pair) { return pair[0]; });
    indices[size] = static_cast<uint32_t>(points().size());
    return indices;
}

template <typename Node, typename CRTP, typename Mut>
const std::vector<morphio::floatType>& TTree<Node, CRTP, Mut>::diameters() const {
    return get<Property::Diameter>();
}

template <typename Node, typename CRTP, typename Mut>
const std::vector<morphio::floatType>& TTree<Node, CRTP, Mut>::perimeters() const {
    return get<Property::Perimeter>();
}

template <typename Node, typename CRTP, typename Mut>
const std::vector<typename Node::Type> TTree<Node, CRTP, Mut>::sectionTypes() const {
    std::vector<typename Node::Type> res;
    for(auto type: get<Property::SectionType>())
        res.push_back(static_cast<typename Node::Type>(type));
    return res;
}

/*
template <typename Node, typename CRTP, typename Mut>
const CellFamily& TTree<Node, CRTP, Mut>::cellFamily() const {
    return _properties->cellFamily();
}
*/

template <typename Node, typename CRTP, typename Mut>
const std::map<int, std::vector<unsigned int>>& TTree<Node, CRTP, Mut>::connectivity() const {
    return _properties->children<Property::Section>();
}

template <typename Node, typename CRTP, typename Mut>
const MorphologyVersion& TTree<Node, CRTP, Mut>::version() const {
    return _properties->version();
}

template <typename Node, typename CRTP, typename Mut>
depth_iterator_t<Node> TTree<Node, CRTP, Mut>::depth_begin() const {
    return depth_iterator(rootSections());
}

template <typename Node, typename CRTP, typename Mut>
depth_iterator_t<Node> TTree<Node, CRTP, Mut>::depth_end() const {
    return depth_iterator();
}

template <typename Node, typename CRTP, typename Mut>
breadth_iterator_t<Node> TTree<Node, CRTP, Mut>::breadth_begin() const {
    return breadth_iterator(rootSections());
}

template <typename Node, typename CRTP, typename Mut>
breadth_iterator_t<Node> TTree<Node, CRTP, Mut>::breadth_end() const {
    return breadth_iterator();
}

SomaType getSomaType(long unsigned int nSomaPoints);

} // namespace morphio
