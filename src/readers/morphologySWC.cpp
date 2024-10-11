/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "morphologySWC.h"

#include <cctype>         // isdigit
#include <cstdint>        // uint32_t
#include <memory>         // std::shared_ptr
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <utility>
#include <vector>         // std::vector

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>
#include <morphio/warning_handling.h>

#include "../error_message_generation.h"
#include "../shared_utils.hpp"
#include "utils.h"

namespace morphio {
namespace details {

// It's not clear if -1 is the only way of identifying a root section.
const int SWC_UNDEFINED_PARENT = -1;
const unsigned int SWC_ROOT = 0xFFFFFFFD;

/* simple stream parser for SWC file format which is a line oriented format
 *
 * This parser advances across comments and blank lines, and allows the caller
 * to get integers and floats
 */
class SWCTokenizer
{
public:
  explicit SWCTokenizer(std::string contents, std::string path)
      : contents_(std::move(contents))
      , path_(std::move(path)) {
      // ensure null termination
      (void) contents_.c_str();
  }

  bool done() const noexcept {
      return pos_ >= contents_.size();
  }

  size_t lineNumber() const noexcept {
      return line_;
  }

  void skip_to(char value) {
      std::size_t pos = contents_.find_first_of(value, pos_);
      if (pos == std::string::npos) {
          pos_ = contents_.size();
      }
      pos_ = pos;
  }

  void advance_to_non_whitespace() {
      if (done()) {
          return;
      }
      std::size_t pos = contents_.find_first_not_of(" \t\r", pos_);
      if (pos == std::string::npos) {
          pos_ = contents_.size();
          return;
      }
      pos_ = pos;
  }

  void advance_to_number() {
      advance_to_non_whitespace();

      if (done()) {
          details::ErrorMessages err(path_);
          throw RawDataError(err.EARLY_END_OF_FILE(line_));
      }

      auto c = contents_.at(pos_);
      if (std::isdigit(c) != 0 || c == '-' || c == '+' || c == '.') {
          return;
      }

      details::ErrorMessages err(path_);
      throw RawDataError(err.ERROR_LINE_NON_PARSABLE(line_));
  }

  int64_t read_int() {
      advance_to_number();
      try {
          auto parsed = stn_.toInt(contents_, pos_);
          pos_ = std::get<1>(parsed);
          return std::get<0>(parsed);
      } catch(std::invalid_argument&e) {
          throw RawDataError(e.what());
      }
  }

  floatType read_float() {
      advance_to_number();
      auto parsed = stn_.toFloat(contents_, pos_);
      pos_ = std::get<1>(parsed);
      return std::get<0>(parsed);
  }

  void skip_blank_lines_and_comments() {
      advance_to_non_whitespace();

      while (!done() && (contents_.at(pos_) == '#' || contents_.at(pos_) == '\n')) {
          if (contents_.at(pos_) == '#') {
              skip_to('\n');
          }

          if (!done() && contents_.at(pos_) == '\n') {
              ++line_;
              ++pos_;
          }
          advance_to_non_whitespace();
      }
  }

  void finish_line() {
      skip_to('\n');
      if (!done() && contents_.at(pos_) == '\n') {
          ++line_;
          ++pos_;
      }
  }


private:
  size_t pos_ = 0;
  size_t line_ = 1;
  std::string contents_;
  StringToNumber stn_;
  std::string path_;
};

struct SWCSample {
    enum : unsigned int { UNKNOWN_ID = 0xFFFFFFFE };

    floatType diameter = -1.;
    Point point{};
    SectionType type = SECTION_UNDEFINED;
    unsigned int parentId = UNKNOWN_ID;
    unsigned int id = UNKNOWN_ID;
    unsigned int lineNumber = 0;
};

static std::vector<unsigned int> gatherLineNumbers(const std::vector<SWCSample>& samples) {
    std::vector<unsigned int> lineNumbers;
    std::transform(samples.begin(),
                   samples.cend(),
                   std::back_inserter(lineNumbers),
                   [](const SWCSample& sample) { return sample.lineNumber; });
    return lineNumbers;
}

static std::vector<SWCSample> readSamples(const std::string& contents, const std::string& path) {
    std::vector<SWCSample> samples;
    SWCSample sample;

    SWCTokenizer tokenizer{contents, path};

    tokenizer.skip_blank_lines_and_comments();
    while (!tokenizer.done()) {
        sample.lineNumber = static_cast<unsigned int>(tokenizer.lineNumber());

        int64_t id = tokenizer.read_int();
        if (id < 0) {
            details::ErrorMessages err(path);
            throw RawDataError(err.ERROR_NEGATIVE_ID(sample.lineNumber));
        }else if(id > std::numeric_limits<unsigned int>::max()){
            throw RawDataError("SWC does not support ids larger than" + std::to_string(std::numeric_limits<unsigned int>::max()));
        }

        sample.id = static_cast<unsigned int>(id);

        sample.type = static_cast<SectionType>(tokenizer.read_int());

        for (auto& point : sample.point) {
            point = tokenizer.read_float();
        }

        sample.diameter = 2 * tokenizer.read_float();

        int64_t parentId = tokenizer.read_int();
        if (parentId < -1) {
            details::ErrorMessages err(path);
            throw RawDataError(err.ERROR_NEGATIVE_ID(sample.lineNumber));
        }else if(parentId > std::numeric_limits<unsigned int>::max()){
            throw RawDataError("SWC does not support parent ids larger than" + std::to_string(std::numeric_limits<unsigned int>::max()));
        } else if (parentId == SWC_UNDEFINED_PARENT) {
            sample.parentId = SWC_ROOT;
        } else {
            sample.parentId = static_cast<unsigned int>(parentId);
        }

        samples.push_back(sample);

        // Normally one would just `skip_blank_lines_and_comments` directly,
        // but we support "extra columns" that are ignored; so since people can put anything
        // in these "extra columns", we have to ignore everything until the end of the line
        tokenizer.finish_line();

        tokenizer.skip_blank_lines_and_comments();
    }

    if (!tokenizer.done()) {
        details::ErrorMessages err(path);
        throw RawDataError(err.EARLY_END_OF_FILE(0));
    }

    return samples;
}

/**
  Parsing SWC according to this specification:
http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 **/

enum class DeclaredID : unsigned int {};

class SWCBuilder
{
    using MorphioID = uint32_t;
    using PhysicalID = size_t;
    using Samples = std::vector<SWCSample>;

  public:
    SWCBuilder(std::string path, WarningHandler* warning_handler, unsigned int options)
        : path_(std::move(path))
        , warning_handler_(warning_handler)
        , options_(options) {}

    Property::Properties buildProperties(const std::string& contents) {
        const Samples samples = readSamples(contents, path_);
        buildSWC(samples);
        morph_.applyModifiers(options_);
        return morph_.buildReadOnly();
    }

  private:
    void build_soma(const Samples& soma_samples) {
        auto& soma = morph_.soma();

        if (soma_samples.empty()) {
            soma->type() = SOMA_UNDEFINED;
            warning_handler_->emit(std::make_unique<NoSomaFound>(path_));
            return;
        } else if (soma_samples.size() == 1) {
            SWCSample sample = soma_samples[0];

            if (sample.parentId != SWC_ROOT && samples_.at(sample.parentId).type != SECTION_SOMA) {
                details::ErrorMessages err_(path_);
                throw SomaError(err_.ERROR_SOMA_WITH_NEURITE_PARENT(sample.lineNumber));
            }

            soma->type() = SOMA_SINGLE_POINT;
            soma->points() = {sample.point};
            soma->diameters() = {sample.diameter};
            return;
        } else if (soma_samples.size() == 3 &&
                   (soma_samples[0].id == 1 && soma_samples[0].parentId == SWC_ROOT &&
                    soma_samples[1].id == 2 && soma_samples[1].parentId == 1 &&
                    soma_samples[2].id == 3 && soma_samples[2].parentId == 1)) {
            const std::array<Point, 3> points = {
                soma_samples[0].point,
                soma_samples[1].point,
                soma_samples[2].point,
            };
            // All soma that bifurcate with the first parent having two children are considered
            // SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS
            const details::ThreePointSomaStatus status =
                details::checkNeuroMorphoSoma(points, soma_samples[0].diameter / 2);
            if (status != details::ThreePointSomaStatus::Conforms) {
                std::stringstream stream;
                stream << status;
                warning_handler_->emit(std::make_unique<SomaNonConform>(path_, stream.str()));
            }
            soma->type() = SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
            soma->points() = std::vector<Point>(points.begin(), points.end());
            soma->diameters() = {
                soma_samples[0].diameter,
                soma_samples[1].diameter,
                soma_samples[2].diameter,
            };
            return;
        }
        // might also have 3 points at this point, as well

        // a "normal" SWC soma
        soma->type() = SOMA_CYLINDERS;
        auto& points = soma->points();
        auto& diameters = soma->diameters();
        points.reserve(soma_samples.size());
        diameters.reserve(soma_samples.size());

        size_t parent_count = 0;
        for (const auto& s : soma_samples) {
            if (s.parentId == SWC_ROOT) {
                parent_count++;
            } else if (samples_.at(s.parentId).type != SECTION_SOMA) {
                details::ErrorMessages err_(path_);
                throw SomaError(err_.ERROR_SOMA_WITH_NEURITE_PARENT(s.lineNumber));
            }

            if (children_.count(s.id) > 0 && children_.at(s.id).size() > 1) {
                std::vector<SWCSample> soma_bifurcations;
                for (auto id : children_.at(s.id)) {
                    if (samples_[id].type == SECTION_SOMA && s.parentId != SWC_ROOT) {
                        soma_bifurcations.push_back(samples_[id]);
                    }
                }
                if (soma_bifurcations.size() > 1) {
                    details::ErrorMessages err_(path_);
                    throw SomaError(
                        err_.ERROR_SOMA_BIFURCATION(s.lineNumber,
                                                    gatherLineNumbers(soma_bifurcations)));
                }
            }
            points.push_back(s.point);
            diameters.push_back(s.diameter);
        }

        if (parent_count > 1) {
            details::ErrorMessages err_(path_);
            throw SomaError(err_.ERROR_MULTIPLE_SOMATA(gatherLineNumbers(soma_samples)));
        }
    }

    void buildSWC(const Samples& samples) {
        Samples soma_samples;
        Samples root_samples;

        for (const auto& sample: samples) {
            // { checks
            if (sample.diameter < morphio::epsilon) {
                details::ErrorMessages err_(path_);
                warning_handler_->emit(std::make_unique<ZeroDiameter>(path_, sample.lineNumber));
            }

            if (sample.parentId == sample.id) {
                details::ErrorMessages err_(path_);
                throw RawDataError(err_.ERROR_SELF_PARENT(sample.id));
            }

            if (sample.type >= SECTION_OUT_OF_RANGE_START || sample.type <= 0) {
                details::ErrorMessages err_(path_);
                throw RawDataError(
                    err_.ERROR_UNSUPPORTED_SECTION_TYPE(sample.lineNumber, sample.type));
            }
            if (sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA) {
                warning_handler_->emit(
                    std::make_unique<DisconnectedNeurite>(path_, sample.lineNumber));
            }
            // } checks

            if (sample.type == SECTION_SOMA) {
                soma_samples.push_back(sample);
            }

            if (sample.parentId == SWC_ROOT || sample.type == SECTION_SOMA) {
                root_samples.push_back(sample);
            }

            if (!samples_.insert({sample.id, sample}).second) {
                const auto& original = samples_[sample.id];
                details::ErrorMessages err_(path_);
                throw RawDataError(err_.ERROR_REPEATED_ID(original.id, original.lineNumber, sample.id));
            }

            children_[sample.parentId].push_back(sample.id);
        }

        // can only check for missing parents once all samples are loaded
        // since it's possible there may be forward references
        for (const auto& sample: samples) {
            if(sample.parentId != SWC_ROOT && samples_.count(sample.parentId) == 0){
                details::ErrorMessages err_(path_);
                throw MissingParentError(err_.ERROR_MISSING_PARENT(
                    sample.id, static_cast<int>(sample.parentId), sample.lineNumber));
            }
        }

        build_soma(soma_samples);

        std::unordered_map<DeclaredID, std::shared_ptr<morphio::mut::Section>> declared_to_swc;
        declared_to_swc.reserve(samples.size());

        for (const SWCSample& root_sample : root_samples) {
            if (children_.count(root_sample.id) == 0) {
                continue;
            }

            // https://neuromorpho.org/SomaFormat.html
            // "The second and third soma points, as well as all starting points
            // (roots) of dendritic and axonal arbors have this first point as
            // the parent (parent ID 1)."
            if (morph_.soma()->type() == SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS &&
                root_sample.type == SECTION_SOMA && root_sample.id != 1) {
                warning_handler_->emit(std::make_unique<WrongRootPoint>(
                    path_, std::vector<unsigned int>{root_sample.lineNumber}));
            }

            for (unsigned int child_id : children_.at(root_sample.id)) {
                if (samples_.at(child_id).type == SECTION_SOMA) {
                    continue;
                }
                if (root_sample.type == SECTION_SOMA) {
                    assembleSections(child_id,
                                     DeclaredID(root_sample.id),
                                     declared_to_swc,
                                     morph_.soma()->points()[0],
                                     morph_.soma()->diameters()[0],
                                     true);
                } else {
                    // this is neurite as the start
                    assembleSections(root_sample.id,
                                     DeclaredID(SWC_ROOT),
                                     declared_to_swc,
                                     root_sample.point,
                                     root_sample.diameter,
                                     true);
                    break;
                }
            }
        }
    }

    void assembleSections(
        unsigned int id,
        DeclaredID parent_id,
        std::unordered_map<DeclaredID, std::shared_ptr<morphio::mut::Section>>& declared_to_swc,
        const Point& start_point,
        floatType start_diameter,
        bool is_root)
    {
        Property::PointLevel properties;
        auto& points = properties._points;
        auto& diameters = properties._diameters;

        auto appendSection = [&](DeclaredID section_id_, DeclaredID parent_id_, SectionType starting_section_type) {
            std::shared_ptr<morphio::mut::Section> new_section;
            if (is_root) {
                new_section = morph_.appendRootSection(properties, starting_section_type);
            } else {
                new_section = declared_to_swc.at(parent_id_)
                                  ->appendSection(properties, starting_section_type);
            }
            declared_to_swc[section_id_] = new_section;
        };

        auto get_child_count = [&](unsigned int child_id) {
            return children_.count(child_id) == 0 ? 0 : children_.at(child_id).size();
        };

        const SWCSample* sample = &samples_.at(id);

        // create duplicate point if needed
        if (!is_root && sample->point != start_point) {
            points.push_back(start_point);
            diameters.push_back(start_diameter);
        }

        // try and combine as many single samples into a single section as possible
        size_t children_count = get_child_count(id);
        while (children_count == 1) {
            sample = &samples_.at(id);
            if(sample->type != samples_.at(children_.at(id)[0]).type){
                if (options_ & ALLOW_UNIFURCATED_SECTION_CHANGE) {
                    warning_handler_->emit(
                        std::make_unique<SectionTypeChanged>(path_, sample->lineNumber));
                    break;
                }
                throw RawDataError("Section type changed without a bifucation at line: " +
                                   std::to_string(sample->lineNumber) +
                                   ", consider using UNIFURCATED_SECTION_CHANGE option");
            }
            points.push_back(sample->point);
            diameters.push_back(sample->diameter);
            id = children_.at(id)[0];
            children_count = get_child_count(id);
        }

        sample = &samples_.at(id);
        points.push_back(sample->point);
        diameters.push_back(sample->diameter);
        appendSection(DeclaredID(id), parent_id, sample->type);

        if (children_count == 0) {
            // section was already appended above, nothing to do
        } else if (children_count == 1) {
            // section_type changed
            size_t offset = properties._points.size() - 1;
            const Point& new_start_point = properties._points[offset];
            floatType new_start_diameter = properties._diameters[offset];
            assembleSections(children_.at(id)[0], DeclaredID(id), declared_to_swc, new_start_point, new_start_diameter, false);
        } else {
            size_t offset = properties._points.size() - 1;
            const Point& new_start_point = properties._points[offset];
            floatType new_start_diameter = properties._diameters[offset];
            for (unsigned int child_id : children_.at(id)) {
                assembleSections(child_id,
                                 DeclaredID(id),
                                 declared_to_swc,
                                 new_start_point,
                                 new_start_diameter,
                                 false);
            }
        }
    }

    std::unordered_map<unsigned int, std::vector<unsigned int>> children_;
    std::unordered_map<unsigned int, SWCSample> samples_;
    mut::Morphology morph_;
    std::string path_;
    WarningHandler* warning_handler_;
    unsigned int options_;
};

}  // namespace details

namespace readers {
namespace swc {
Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options,
                          std::shared_ptr<WarningHandler>& warning_handler) {
    auto properties =
        details::SWCBuilder(path, warning_handler.get(), options).buildProperties(contents);

    properties._cellLevel._cellFamily = NEURON;
    properties._cellLevel._version = {"swc", 1, 0};
    return properties;
}

}  // namespace swc
}  // namespace readers
}  // namespace morphio
