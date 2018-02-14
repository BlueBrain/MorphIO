#include "point.hpp"
#include "print_vector.hpp"

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <ostream>

#include <algorithm>
#include <iterator>

#include <map>
#include <vector>

using SectionID = int32_t;
using SectionType = int32_t;
using Points = std::vector<Point>;

struct NeuronBuilder
{
    struct SectionOrder
    {
        SectionID parent_id;
        size_t start;
        SectionType type;
    };
    using SectionArrangement = std::vector<SectionOrder>;

    struct DataBlock
    {
        Points points;
        SectionArrangement order;
    };

    struct SectionInfo
    {
        SectionID section_id;
        SectionID parent_id;
        SectionType type;
        Points points;
    };

    SectionID add_section(SectionID section_id, SectionID parent_id,
                          SectionType section_type, Points& p);

    DataBlock data_block() const;

    std::map<SectionID, SectionInfo> sections_;
    std::map<SectionID, std::vector<SectionID>> children_;
};

SectionID NeuronBuilder::add_section(SectionID section_id, SectionID parent_id,
                                     SectionType section_type, Points& p)
{
    assert(sections_.count(section_id) == 0 && "Cannot replace section");

    // std::cout << "add_section: " << section_id << ' ' << parent_id << ' '
    //          << section_type << ' ' << p << std::endl;

    sections_[section_id] = {section_id, parent_id, section_type, p};
    children_[parent_id].push_back(section_id);
    return sections_.size() - 1;
}

NeuronBuilder::DataBlock NeuronBuilder::data_block() const
{
    // duplicate endpoint of a parent section as the first point
    // of a child section
    const bool duplicate_endpoint_at_start = true;

    Points points;
    SectionArrangement order;

    // TODO: points.reserve() for # of points
    order.reserve(sections_.size());

    // depth first traversal
    std::vector<SectionID> stack;

    // TODO: keep track of the 'root' node, instead of assuming it's -1
    if (!sections_.empty())
        stack.push_back(-1);
    while (!stack.empty())
    {
        SectionID parent_id(stack.back());
        stack.pop_back();

        for (const auto& child_id : children_.at(parent_id))
        {
            size_t start = points.size();
            const SectionInfo& si = sections_.at(child_id);

            order.push_back({parent_id, start, si.type});

            if (duplicate_endpoint_at_start && parent_id != -1)
            {
                const SectionInfo& parent_si = sections_.at(parent_id);
                points.push_back(parent_si.points.back());
            }

            std::copy(si.points.begin(), si.points.end(),
                      std::back_inserter(points));
            if (0 < children_.count(child_id))
            {
                stack.push_back(child_id);
            }
        }
    }

    return {points, order};
}

std::ostream& operator<<(std::ostream& s, const NeuronBuilder& nb)
{
    for (auto sec : nb.sections_)
    {
        const auto& si = sec.second;
        s << "id: " << si.section_id << " pid: " << si.parent_id
          << " type: " << si.type << " {" << si.points.size() << ": "
          << si.points << "}\n";
    }
    return s;
}

void print_h5(const NeuronBuilder& nb)
{
    using std::setw;

    NeuronBuilder::DataBlock db = nb.data_block();

    int i = 0;
    std::cout << setw(7) << "Row" << setw(7) << "PID" << setw(7) << "Start"
              << setw(7) << "Type" << std::endl;
    for (auto& o : db.order)
    {
        std::cout << setw(6) << i << ' ' << setw(6) << o.parent_id << ' '
                  << setw(6) << o.start << ' ' << setw(6) << o.type
                  << std::endl;
        ++i;
    }

    std::cout << "Points: " << db.points.size()
              << " Sections: " << db.order.size() << std::endl;
}

#if 0
int main(int argc, const char *argv[])
{
  (void)argc;
  (void)argv;

  std::cout << "Welcome to NeuronBuilder\n";
  NeuronBuilder nb;

  for(int i = 0; i < 10; ++i){
    Points ps;
    for(int j = 0; j < 10; ++j){
      double v = j;
      Point p = {(double)i, v, v};
      ps.push_back(p);
    }

    nb.add_section(i, /* parent_id */ i - 1, 0, ps);
  }

  std::cout << nb << std::endl;
  print_h5(nb);

  return 0;
}
#endif
