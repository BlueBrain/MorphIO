#include <sstream>

#include <minimorph/sectionBuilder.h>

namespace minimorph
{

namespace builder
{

Morphology::Morphology(const minimorph::Morphology& morphology) : _counter(0), _soma(morphology.getSoma()){
    for(const auto& rootSection: morphology.getRootSections())
        _rootSections.insert(new Section(this, rootSection));
}


uint32_t Morphology::appendSection(Section* parent, const minimorph::Section& section, bool recursive){
    Section *node = new Section(this, section, recursive);
    parent->_children.insert(node);
    node->_parent = parent;
}

uint32_t Morphology::appendSection(Section* parent, SectionType type, const Property::PointLevel &pointProperties){
    Section *section = new Section(this, _counter, type, pointProperties);
    parent->_children.insert(section);
    section->_parent = parent;
}

void Morphology::_register(Section* section){
    _counter = std::max(_counter, section->getID()+1);
    if(_sections[section->getID()]){
        std::stringstream ss;
        ss << "Cannot register section (" << section->getID() << "). The morphology has already a section with the same ID." << std::endl;
        LBTHROW(ss.str());
    }
    _sections[section->getID()]= section;
}

Soma::Soma(const minimorph::Soma &soma){
    _pointProperties = Property::PointLevel(soma._properties->_pointLevel,
                                            soma._range);
}
Section::Section(Morphology *morphology, int id, SectionType type, const Property::PointLevel& pointProperties) : _pointProperties(pointProperties), _id(id), _sectionType(type){
    morphology->_register(this);
}

Section::Section(Morphology *morphology, const minimorph::Section &section, bool recursive) :
    Section(morphology,
            section.getID(),
            section.getType(),
            Property::PointLevel(section._properties->_pointLevel,
                                 section._range))
{
    if(recursive){
        for(const auto& child: section.getChildren())
            _children.insert(new Section(morphology, child));
    }
}

const Section* const Section::getParent() {
    return _parent;
}


const std::set<Section*>& Section::getChildren(){
    return _children;
}

void Morphology::deleteSection(Section* section, bool recursive){
    _sections.erase(section->getID());
    _rootSections.erase(section);
    for(auto child: section->_children){
        if(recursive)
            deleteSection(section, recursive);
        else {
            if(section->_parent)
                section->_parent->_children.insert(child);
            child->_parent = section->_parent;
        }
    }
    delete section;
}

void Morphology::traverse(void (*fun)(Morphology* morphology, Section* section), Section* rootSection){
    auto& sections = rootSection ? std::set<Section*>{rootSection} : getRootSections();
    for(auto root: sections){
        root->traverse(this, fun);
    }
}

void Section::traverse(Morphology* morphology,
                       void (*fun)(Morphology* morphology, Section* section)){
    // depth first traversal
    std::vector<Section*> stack;
    stack.push_back(this);
    while (!stack.empty())
    {
        Section* parent = stack.back();
        stack.pop_back();
        fun(morphology, parent);
        for (auto child : parent->getChildren())
        {
            stack.push_back(child);
        }
    }
}

namespace writer
{
// void h5(const Morphology& morphology)
// {
//     using std::setw;


//     int i = 0;

//     int start = 0;
//     int sectionIdOnDisk = 0;
//     auto writeSection = [&start, &sectionIdOnDisk](Morphology* morphology, Section* section) {
//         std::cout << setw(6) << sectionIdOnDisk++ << ' ' << setw(6) << section->getParent()->getID() << ' '
//         << setw(6) << start << ' ' << setw(6) << section->getType()
//                   << std::endl;
//         start += section->getPoints().size();
//     };

//     morphology.traverse(writeSection);

//     std::cout
//         << "Points: " << db.points.size()
//         << " Sections: " << db.order.size()
//         << std::endl;
// }
} // end namespace writer

}
}
