#include "morphologySWC.h"
#include "utils.h"

#include <cctype>         // isdigit
#include <cstdint>        // uint32_t
#include <memory>         // std::shared_ptr
#include <string>         // std::string
#include <unordered_map>  // std::unordered_map
#include <vector>         // std::vector

#include <morphio/errorMessages.h>
#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/mut/soma.h>
#include <morphio/properties.h>

namespace {
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
  explicit SWCTokenizer(std::string contents, const morphio::readers::ErrorMessages& err)
      : contents_(std::move(contents))
      , err_(err) {
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
      std::size_t pos = contents_.find_first_not_of(" \t", pos_);
      if (pos == std::string::npos) {
          pos_ = contents_.size();
      }
      pos_ = pos;
  }

  void advance_to_number() {
      while (consume_line_and_trailing_comments()) {
      }

      if (done()) {
          throw morphio::RawDataError(err_.EARLY_END_OF_FILE(line_));
      }

      auto c = contents_.at(pos_);
      if (std::isdigit(c) != 0 || c == '-' || c == '+' || c == '.') {
          return;
      }

      throw morphio::RawDataError(err_.ERROR_LINE_NON_PARSABLE(line_));
  }

  int64_t read_int() {
      advance_to_number();
      auto parsed = stn_.toInt(contents_, pos_);
      pos_ = std::get<1>(parsed);
      return std::get<0>(parsed);
  }

  morphio::floatType read_float() {
      advance_to_number();
      auto parsed = stn_.toFloat(contents_, pos_);
      pos_ = std::get<1>(parsed);
      return std::get<0>(parsed);
  }

  bool consume_line_and_trailing_comments() {
      bool found_newline = false;

      advance_to_non_whitespace();
      while (!done() && (contents_.at(pos_) == '#' || contents_.at(pos_) == '\n')) {
          switch (contents_.at(pos_)) {
          case '#':
              skip_to('\n');
              break;
          case '\n':
              ++line_;
              ++pos_;
              found_newline = true;
              break;
          }
          advance_to_non_whitespace();
      }
      return found_newline || done();
  }

private:
  size_t pos_ = 0;
  size_t line_ = 1;
  std::string contents_;
  morphio::StringToNumber stn_{};
  morphio::readers::ErrorMessages err_;
};

std::vector<morphio::readers::Sample> readSamples(const std::string& contents,
                                                  const morphio::readers::ErrorMessages& err) {
    std::vector<morphio::readers::Sample> samples;
    morphio::readers::Sample sample;

    SWCTokenizer tokenizer{contents, err};
    tokenizer.consume_line_and_trailing_comments();

    while (!tokenizer.done()) {
        sample.lineNumber = static_cast<unsigned int>(tokenizer.lineNumber());

        int64_t id = tokenizer.read_int();
        if (id < 0) {
            throw morphio::RawDataError(err.ERROR_NEGATIVE_ID(sample.lineNumber));
        }

        sample.id = static_cast<unsigned int>(id);

        sample.type = static_cast<morphio::SectionType>(tokenizer.read_int());

        for (auto& point : sample.point) {
            point = tokenizer.read_float();
        }

        sample.diameter = 2 * tokenizer.read_float();

        int64_t parentId = tokenizer.read_int();
        if (parentId < -1) {
            throw morphio::RawDataError(err.ERROR_NEGATIVE_ID(sample.lineNumber));
        } else if (parentId == SWC_UNDEFINED_PARENT) {
            sample.parentId = SWC_ROOT;
        } else {
            sample.parentId = static_cast<unsigned int>(parentId);
        }

        if (!tokenizer.consume_line_and_trailing_comments()) {
            throw morphio::RawDataError(err.ERROR_LINE_NON_PARSABLE(sample.lineNumber));
        }
        samples.push_back(sample);
    }
    return samples;
}

}  // unnamed namespace

namespace morphio {
namespace readers {
namespace swc {
/**
  Parsing SWC according to this specification:
http://www.neuronland.org/NLMorphologyConverter/MorphologyFormats/SWC/Spec.html
 **/
class SWCBuilder
{
  public:
    explicit SWCBuilder(const std::string& path)
        : err_(path) {}

    void raiseIfBrokenSoma(const Sample& sample) {
        if (sample.type != SECTION_SOMA) {
            return;
        }

        if (sample.parentId != SWC_ROOT && !children_[sample.id].empty()) {
            std::vector<Sample> soma_bifurcations;
            for (unsigned int id : children_[sample.id]) {
                if (samples_[id].type == SECTION_SOMA) {
                    soma_bifurcations.push_back(samples_[id]);
                } else {
                    neurite_wrong_root_.push_back(samples_[id]);
                }
            }

            if (soma_bifurcations.size() > 1) {
                throw morphio::SomaError(err_.ERROR_SOMA_BIFURCATION(sample, soma_bifurcations));
            }
        }

        if (sample.parentId != SWC_ROOT && samples_.count(sample.parentId) > 0 &&
            samples_.at(sample.parentId).type != SECTION_SOMA) {
            throw morphio::SomaError(err_.ERROR_SOMA_WITH_NEURITE_PARENT(sample));
        }
    }

    void checkSoma() {
        std::vector<Sample> somata;
        for (auto id : children_[SWC_ROOT]) {
            if (samples_.at(id).type == SECTION_SOMA) {
                somata.push_back(samples_[id]);
            }
        }

        if (somata.size() > 1) {
            throw morphio::SomaError(err_.ERROR_MULTIPLE_SOMATA(somata));
        }

        if (somata.empty()) {
            printError(Warning::NO_SOMA_FOUND, err_.WARNING_NO_SOMA_FOUND());
        } else {
            for (const auto& sample_pair : samples_) {
                const auto& sample = sample_pair.second;
                if (sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA) {
                    printError(Warning::DISCONNECTED_NEURITE, err_.WARNING_DISCONNECTED_NEURITE(sample));
                }
            }
        }
    }

    bool isRootPoint(const Sample& sample) {
        bool isOrphanNeurite = sample.parentId == SWC_ROOT && sample.type != SECTION_SOMA;
        return isOrphanNeurite ||
               (sample.type != SECTION_SOMA &&
                samples_.at(sample.parentId).type == SECTION_SOMA);  // Exclude soma bifurcations
    }

    bool isSectionStart(const Sample& sample) {
        return (isRootPoint(sample) ||
                (sample.parentId != SWC_ROOT &&
                 isSectionEnd(samples_.at(sample.parentId))));  // Standard section
    }

    bool isSectionEnd(const Sample& sample) {
        return sample.id == lastSomaPoint_ ||        // End of soma
               children_[sample.id].empty() ||       // Reached leaf
               (children_[sample.id].size() >= 2 &&  // Reached neurite bifurcation
                sample.type != SECTION_SOMA);
    }

    template <typename T>
    void appendSample(const std::shared_ptr<T>& somaOrSection, const Sample& sample) {
        somaOrSection->points().push_back(sample.point);
        somaOrSection->diameters().push_back(sample.diameter);
    }

    std::vector<unsigned int> constructDepthFirstSamples() {
        std::vector<unsigned int> ret;
        ret.reserve(samples_.size());
        const auto pushChildren = [&](const auto& f, unsigned int id) -> void {
            for (unsigned int childId : children_[id]) {
                ret.push_back(childId);
                f(f, childId);
            }
        };

        pushChildren(pushChildren, SWC_ROOT);

        return ret;
    }

    void _checkNeuroMorphoSoma(const Sample& root, const std::vector<Sample>& _children) {
        floatType x = root.point[0];
        floatType y = root.point[1];
        floatType z = root.point[2];
        floatType d = root.diameter;
        floatType r = root.diameter / 2;
        const Sample& child1 = _children[0];
        const Sample& child2 = _children[1];

        // whether the soma should be checked for the special case of 3 point soma
        // for details see https://github.com/BlueBrain/MorphIO/issues/273
        bool isSuited = std::fabs(child1.diameter - d) < morphio::epsilon &&
                        std::fabs(child2.diameter - d) < morphio::epsilon &&
                        std::fabs(child1.point[0] - x) < morphio::epsilon &&
                        std::fabs(child2.point[0] - x) < morphio::epsilon &&
                        std::fabs(child1.point[2] - z) < morphio::epsilon &&
                        std::fabs(child2.point[2] - z) < morphio::epsilon;
        if (!isSuited) {
            return;
        }
        // If the 2nd and the 3rd point have the same x,z,d values then the only valid soma is:
        // 1 1 x   y   z r -1
        // 2 1 x (y-r) z r  1
        // 3 1 x (y+r) z r  1
        if (child1.point[0] != x || child2.point[0] != x || child1.point[1] != y - r ||
            child2.point[1] != y + r || child1.point[2] != z || child2.point[2] != z ||
            child1.diameter != d || child2.diameter != d) {
            printError(Warning::SOMA_NON_CONFORM,
                       err_.WARNING_NEUROMORPHO_SOMA_NON_CONFORM(root, child1, child2));
        }
    }

    SomaType somaType() {
        switch (morph_.soma()->points().size()) {
        case 0: {
            return SOMA_UNDEFINED;
        }
        case 1: {
            return SOMA_SINGLE_POINT;
        }
        case 2: {
            return SOMA_CYLINDERS;
        }
            // NeuroMorpho format is characterized by a 3 points soma
            // with a bifurcation at soma root
        case 3: {
            uint32_t somaRootId = children_[SWC_ROOT][0];
            const auto& somaChildren = children_[somaRootId];

            std::vector<Sample> children_soma_points;
            for (unsigned int child : somaChildren) {
                if (samples_.at(child).type == SECTION_SOMA) {
                    children_soma_points.push_back(samples_.at(child));
                }
            }

            if (children_soma_points.size() == 2) {
                //  NeuroMorpho is the main provider of morphologies, but they
                //  with SWC as their default file format: they convert all
                //  uploads to SWC.  In the process of conversion, they turn all
                //  somas into their custom 'Three-point soma representation':
                //   http://neuromorpho.org/SomaFormat.html

                if (!ErrorMessages::isIgnored(Warning::SOMA_NON_CONFORM)) {
                    _checkNeuroMorphoSoma(samples_.at(somaRootId), children_soma_points);
                }

                return SOMA_NEUROMORPHO_THREE_POINT_CYLINDERS;
            }
            return SOMA_CYLINDERS;
        }
        default:
            return SOMA_CYLINDERS;
        }
    }

    Property::Properties buildProperties(const std::string& contents, unsigned int options) {
        const auto samples = readSamples(contents, err_);

        for (const auto& sample: samples) {
            if (sample.diameter < morphio::epsilon) {
                printError(Warning::ZERO_DIAMETER, err_.WARNING_ZERO_DIAMETER(sample));
            }

            if (sample.parentId == sample.id) {
                throw morphio::RawDataError(err_.ERROR_SELF_PARENT(sample));
            }

            if (sample.type >= morphio::SECTION_OUT_OF_RANGE_START || sample.type <= 0) {
                throw morphio::RawDataError(
                    err_.ERROR_UNSUPPORTED_SECTION_TYPE(sample.lineNumber, sample.type));
            }

            if (!samples_.insert({sample.id, sample}).second) {
                throw RawDataError(err_.ERROR_REPEATED_ID(samples[sample.id], sample));
            }

            children_[sample.parentId].push_back(sample.id);

            if (sample.type == SECTION_SOMA) {
                lastSomaPoint_ = sample.id;
            }
        }

        for (const auto& sample: samples) {
            raiseIfBrokenSoma(sample);

            if (sample.parentId != SWC_ROOT && samples_.count(sample.parentId) == 0) {
                throw morphio::MissingParentError(err_.ERROR_MISSING_PARENT(sample));
            }
        }

        checkSoma();

        // The process might occasionally creates empty section before
        // filling them so the warning is ignored
        bool originalIsIgnored = err_.isIgnored(morphio::Warning::APPENDING_EMPTY_SECTION);
        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, true);

        std::vector<unsigned int> depthFirstSamples = constructDepthFirstSamples();
        for (unsigned int id : depthFirstSamples) {
            const Sample& sample = samples_.at(id);

            // Bifurcation right at the start
            if (isRootPoint(sample) && isSectionEnd(sample)) {
                continue;
            }

            if (isSectionStart(sample)) {
                _processSectionStart(sample);
            } else if (sample.type != SECTION_SOMA) {
                swcIdToSectionId_[sample.id] = swcIdToSectionId_[sample.parentId];
            }

            if (sample.type == SECTION_SOMA) {
                appendSample(morph_.soma(), sample);
            } else {
                appendSample(morph_.section(swcIdToSectionId_.at(sample.id)), sample);
            }
        }

        if (morph_.soma()->points().size() == 3 && !neurite_wrong_root_.empty()) {
            printError(morphio::WRONG_ROOT_POINT, err_.WARNING_WRONG_ROOT_POINT(neurite_wrong_root_));
        }

        morph_.applyModifiers(options);

        Property::Properties properties = morph_.buildReadOnly();
        properties._cellLevel._somaType = somaType();

        set_ignored_warning(morphio::Warning::APPENDING_EMPTY_SECTION, originalIsIgnored);

        return properties;
    }

    /**
      - Append last point of previous section if current section is not a root section
      - Update the parent ID of the new section
     **/
    void _processSectionStart(const Sample& sample) {
        Property::PointLevel properties;

        uint32_t id = 0;

        if (isRootPoint(sample)) {
            id = morph_.appendRootSection(properties, sample.type)->id();
        } else {
            // Duplicating last point of previous section if there is not already a duplicate
            Sample& parent_sample = samples_.at(sample.parentId);
            if (sample.point != parent_sample.point) {
                properties._points.push_back(parent_sample.point);
                properties._diameters.push_back(parent_sample.diameter);
            }

            // Handle the case, bifurcatation at root point
            if (isRootPoint(parent_sample)) {
                id = morph_.appendRootSection(properties, sample.type)->id();
            } else {
                id = morph_.section(swcIdToSectionId_[sample.parentId])
                    ->appendSection(properties, sample.type)
                    ->id();
            }
        }

        swcIdToSectionId_[sample.id] = id;
    }

  private:
    // SWC Id of the last point of a section to morphio::mut::Section ID
    std::unordered_map<uint32_t, uint32_t> swcIdToSectionId_;

    // Neurite that do not have parent ID = 1, allowed for soma contour, not
    // 3-pts soma
    std::vector<Sample> neurite_wrong_root_;

    unsigned int lastSomaPoint_ = 0;
    std::unordered_map<unsigned int, std::vector<unsigned int>> children_;
    std::unordered_map<unsigned int, Sample> samples_;
    mut::Morphology morph_;
    ErrorMessages err_;
};

Property::Properties load(const std::string& path,
                          const std::string& contents,
                          unsigned int options) {
    auto properties = SWCBuilder(path).buildProperties(contents, options);

    properties._cellLevel._cellFamily = NEURON;
    properties._cellLevel._version = {"swc", 1, 0};
    return properties;
}

}  // namespace swc
}  // namespace readers
}  // namespace morphio
