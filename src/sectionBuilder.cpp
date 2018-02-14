#include <assert.h>

#include <sstream>
#include <string>

#include <minimorph/sectionBuilder.h>

namespace minimorph
{

namespace builder
{

Morphology::Morphology(const minimorph::Morphology& morphology) : _counter(0), _soma(morphology.soma()){
    for(const auto& rootSection: morphology.rootSections())
        _rootSections.insert(new Section(this, rootSection));
}


uint32_t Morphology::appendSection(Section* parent, const minimorph::Section& section, bool recursive){
    Section *node = new Section(this, section, recursive);
    parent->_children.insert(node);
    node->_parent = parent;
    return node->id();
}

uint32_t Morphology::appendSection(Section* parent, SectionType type, const Property::PointLevel &pointProperties){
    Section *section = new Section(this, _counter, type, pointProperties);
    parent->_children.insert(section);
    section->_parent = parent;
    return section->id();
}

uint32_t Morphology::createNeurite(const minimorph::Section& section, bool recursive){
    Section *node = new Section(this, section, recursive);
    _rootSections.insert(node);
    return node->id();
}

uint32_t Morphology::createNeurite(SectionType type, const Property::PointLevel &pointProperties){
    Section *section = new Section(this, _counter, type, pointProperties);
    _rootSections.insert(section);
    return section->id();
}

void Morphology::_register(Section* section){
    _counter = std::max(_counter, section->id()+1);
    if(_sections[section->id()]){
        std::stringstream ss;
        ss << "Cannot register section (" << section->id() << "). The morphology has already a section with the same ID." << std::endl;
        LBTHROW(ss.str());
    }
    _sections[section->id()]= section;
}

Soma& Morphology::soma() {
    return _soma;
}

const std::set<Section*>& Morphology::rootSections() {
    return _rootSections;
}


Morphology::~Morphology() {
    for(auto root: _rootSections)
        deleteSection(root, true);
}

Soma::Soma(const minimorph::Soma &soma){
    _somaType = soma.type();
    _pointProperties = Property::PointLevel(soma._properties->_pointLevel,
                                            soma._range);
}

Section::Section(Morphology *morphology, int id, SectionType type, const Property::PointLevel& pointProperties) : _pointProperties(pointProperties), _id(id), _sectionType(type), _parent(nullptr){
    morphology->_register(this);
}

Section::Section(Morphology *morphology, const minimorph::Section &section, bool recursive) :
    Section(morphology,
            section.id(),
            section.type(),
            Property::PointLevel(section._properties->_pointLevel,
                                 section._range))
{
    if(recursive){
        for(const auto& child: section.children())
            morphology->appendSection(this, child, true);
    }
}

Section* const Section::parent() {
    return _parent;
}


const std::set<Section*>& Section::children(){
    return _children;
}

void Morphology::deleteSection(Section* section, bool recursive){
    _sections.erase(section->id());
    _rootSections.erase(section);
    for(auto child: section->_children){
        if(recursive)
            deleteSection(child, recursive);
        else {
            if(section->_parent)
                section->_parent->_children.insert(child);
            child->_parent = section->_parent;
        }
    }
    delete section;
}

void Morphology::traverse(std::function<void(Morphology* morphology, Section* section)> fun, Section* rootSection){
    auto& sections = rootSection ? std::set<Section*>{rootSection} : rootSections();
    for(auto root: sections){
        root->traverse(this, fun);
    }
}

void Section::traverse(Morphology* morphology,
                       std::function<void(Morphology* morphology, Section* section)> fun){
    // depth first traversal
    std::vector<Section*> stack;
    stack.push_back(this);
    while (!stack.empty())
    {
        Section* parent = stack.back();
        stack.pop_back();
        fun(morphology, parent);
        for (auto child : parent->children())
        {
            stack.push_back(child);
        }
    }
}

namespace writer
{
void h5(Morphology& morphology)
{
    using std::setw;
    int i = 0;

    int start = 0;
    int sectionIdOnDisk = 0;
    std::map<uint32_t, int32_t> newIds;
    auto writeSection = [&start, &sectionIdOnDisk, &newIds](Morphology* morphology, Section* section) {
        std::cout
        << setw(6) << sectionIdOnDisk << ' '
        << setw(6) << (section->parent() ? newIds[section->parent()->id()] : -1) << ' '
        << setw(6) << start << ' '
        << setw(6) << section->type() << std::endl;

        newIds[section->id()] = sectionIdOnDisk++;
        start += section->points().size();
    };

    morphology.traverse(writeSection);


}

void swc(Morphology& morphology) {
    int sectionIdOnDisk = 1;
    std::map<uint32_t, int32_t> newIds;
    auto writeSection = [&sectionIdOnDisk, &newIds](Morphology* morphology, Section* section) {
        const auto& points = section -> points();
        const auto& diameters = section -> diameters();

        assert(points.size() > 0 && "Empty section");
        for(int i = 0; i<points.size(); ++i){
            std::cout
            << sectionIdOnDisk << ' '
            << section->type() << ' '
            << points[i][0] << ' '
            << points[i][1] << ' '
            << points[i][2] << ' '
            << diameters[i] / 2. << ' ';
            if(i>0)
                std::cout << sectionIdOnDisk-1 << std::endl;
            else
                std::cout << (section->parent() ? newIds[section->parent()->id()] : -1) << std::endl;
            ++sectionIdOnDisk;
        }
        newIds[section->id()] = sectionIdOnDisk-1;
    };

    morphology.traverse(writeSection);
}

void _write_asc_points(const Points& points, const std::vector<float>& diameters, int indentLevel) {
    for(int i = 0; i<points.size(); ++i){
        std::cout << std::string(indentLevel, ' ')
                  << "("
                  << points[i][0] << ' '
                  << points[i][1] << ' '
                  << points[i][2] << ' '
                  << diameters[i] << ')' << std::endl;
    }
}

void _write_asc_section(Section *section, int indentLevel){
    std::string indent(indentLevel, ' ');
    std::cout << indent << "(" << std::endl;
    _write_asc_points(section->points(), section->diameters(), indentLevel);

    if(!section->children().empty()){
        auto it = section->children().begin();
        _write_asc_section((*it++), indentLevel+2);
        for(; it != section->children().end(); ++it){
            std::cout << indent << "|" << std::endl;
            _write_asc_section((*it), indentLevel+2);
        }
    }

    std::cout << indent << ")" << std::endl;
}
void asc(Morphology& morphology) {
    std::map<minimorph::SectionType, std::string> header;
    header[SECTION_SOMA] = "(\"CellBody\"\n  (Color Red)\n  (CellBody)\n";
    header[SECTION_AXON] = "( (Color Cyan)\n  (Axon)\n";
    header[SECTION_DENDRITE] = "( (Color Red)\n  (Dendrite)\n";

    auto &soma = morphology.soma();
    std::cout << header[soma.type()];
    _write_asc_points(soma.points(), soma.diameters(), 2);
    std::cout << ")\n\n";

    for(auto& root: morphology.rootSections()){
        std::cout << header[root->type()];
        _write_asc_section(root, 2);
        std::cout << ")\n\n";
    }
}
} // end namespace writer
} // end namespace builder
} // end namespace minimorph
