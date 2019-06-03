#include <morphio/section.h>
#include <morphio/mut/section.h>
#include <morphio/morphology.h>
#include <morphio/mut/morphology.h>
#include <morphio/tools.h>


namespace morphio
{

bool diff(const Morphology& left, const Morphology& right, morphio::enums::LogLevel verbose)
{
    if (left._properties->_cellLevel.diff(right._properties->_cellLevel, verbose))
        return true;

    if (left.rootSections().size() != right.rootSections().size()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Different number of root sections" << std::endl;
        return true;
    }

    for (unsigned int i = 0; i < left.rootSections().size(); ++i)
        if (diff(left.rootSections()[i], right.rootSections()[i], verbose))
            return true;

    return false;
}

bool diff(const Section& left, const Section& right, morphio::enums::LogLevel verbose)
{
    if (left.type() != right.type()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: section type differ" << std::endl;
        return true;
    }

    if (left.points() != right.points()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: points differ" << std::endl;
        return true;
    }

    if (left.diameters() != right.diameters()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: diameters differ" << std::endl;
        return true;
    }

    if (left.perimeters() != right.perimeters()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: perimeters differ" << std::endl;
        return true;
    }

    if (left.children().size() != right.children().size()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: different number of children" << std::endl;
        return true;
    }

    for (unsigned int i = 0; i < left.children().size(); ++i)
        if (diff(left.children()[i], right.children()[i], verbose)) {
            if (verbose > LogLevel::ERROR)
            {
                std::cout << "Summary: children of ";
                ::operator<<(std::cout, left);
                std::cout << " differ. See the above \"Reason\" to know in what they differ." << std::endl;
            }

            return true;
        }

    return false;
}

namespace mut
{
bool diff(const Section& left, const Section& right, morphio::enums::LogLevel verbose)
{
    if (left.type() != right.type()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: section type differ" << std::endl;
        return true;
    }

    if (left.points() != right.points()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: points differ" << std::endl;
        return true;
    }

    if (left.diameters() != right.diameters()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: diameters differ" << std::endl;
        return true;
    }

    if (left.perimeters() != right.perimeters()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: perimeters differ" << std::endl;
        return true;
    }

    if (left.children().size() != right.children().size()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Reason: different number of children" << std::endl;
        return true;
    }

    for (unsigned int i = 0; i < left.children().size(); ++i)
        if (diff(*left.children()[i], *right.children()[i], verbose)) {
            if (verbose > LogLevel::ERROR)
            {
                std::cout << "Summary: children of ";
                ::operator<<(std::cout, left);
                std::cout << " differ. See the above \"Reason\" to know in what they differ." << std::endl;
            }
            return true;
        }

    return false;
}

bool diff(const Morphology& left, const Morphology& right, morphio::enums::LogLevel verbose)
{
    if (left._cellProperties->diff(*right._cellProperties, verbose))
        return true;

    if (left.rootSections().size() != right.rootSections().size()) {
        if (verbose > LogLevel::ERROR)
            std::cout << "Different number of root sections" << std::endl;
        return true;
    }

    for (unsigned int i = 0; i < left.rootSections().size(); ++i)
        if (diff(*left.rootSections()[i], *right.rootSections()[i], verbose))
            return true;

    return false;
}


} // namespace mut
} // namespace morphio
