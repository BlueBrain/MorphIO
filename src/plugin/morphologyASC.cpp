#include <fstream>

#include <morphio/mut/morphology.h>
#include <morphio/mut/section.h>
#include <morphio/types.h>

#include "morphologyASC.h"

#include "lex.cpp"

namespace morphio {
namespace plugin {
namespace asc {
bool is_eof(Token type)
{
    return type == Token::EOF_;
}

bool is_end_of_branch(Token type)
{
    return (type == Token::GENERATED || type == Token::HIGH || type == Token::INCOMPLETE || type == Token::LOW || type == Token::NORMAL || type == Token::MIDPOINT);
}

bool is_neurite_type(Token id)
{
    return (id == Token::AXON || id == Token::APICAL || id == Token::DENDRITE || id == Token::CELLBODY);
}

bool is_end_of_section(Token id)
{
    return (id == Token::RPAREN || id == Token::PIPE);
}

bool skip_sexp(size_t id)
{
    return (id == +Token::WORD || id == +Token::STRING || id == +Token::COLOR || id == +Token::GENERATED || id == +Token::HIGH || id == +Token::INCOMPLETE || id == +Token::LOW || id == +Token::NORMAL);
}

class NeurolucidaParser
{
public:
    NeurolucidaParser(const std::string& uri)
        : uri_(uri)
        , lex_(uri)
        , debugInfo_(uri)
        , err_(uri){};

    NeurolucidaParser(NeurolucidaParser const&) = delete;
    NeurolucidaParser& operator=(NeurolucidaParser const&) = delete;

    morphio::mut::Morphology& parse()
    {
        std::ifstream ifs(uri_);
        std::string input((std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()));

        lex_.start_parse(input);

        parse_block();

        return nb_;
    }

private:
    std::tuple<Point, float> parse_point(NeurolucidaLexer& lex)
    {
        lex.expect(Token::LPAREN, "Point should start in LPAREN");
        std::array<float, 4> point; // X,Y,Z,R
        for (auto& p : point) {
            try {
                p = std::stod(lex.consume()->str());
            } catch (const std::invalid_argument& e) {
                throw RawDataError(
                    err_.ERROR_PARSING_POINT(lex.line_num(),
                        lex.current()->str()));
            }
        }

        lex.consume();

        if (lex.current()->id == +Token::WORD) {
            lex.consume(Token::WORD);
        }

        lex.consume(Token::RPAREN, "Point should end in RPAREN");

        return std::tuple<Point, float>{{point[0], point[1], point[2]},
            point[3]};
    }

    bool parse_neurite_branch(int32_t parent_id, Token token)
    {
        lex_.consume(Token::LPAREN, "New branch should start with LPAREN");

        bool ret = true;
        while (true) {
            ret &= parse_neurite_section(parent_id, token);
            if (lex_.ended() || (lex_.current()->id != +Token::PIPE && lex_.current()->id != +Token::LPAREN)) {
                break;
            }
            lex_.consume();
        }
        lex_.consume(Token::RPAREN, "Branch should end with RPAREN");
        return ret;
    }

    int32_t _create_soma_or_section(Token token, int32_t parent_id,
        std::vector<Point>& points,
        std::vector<float>& diameters)
    {
        lex_.current_section_start_ = lex_.line_num();
        int32_t return_id;
        morphio::Property::PointLevel properties;
        properties._points = points;
        properties._diameters = diameters;
        if (token == Token::CELLBODY) {
            if (nb_.soma()->points().size() != 0)
                throw SomaError(
                    err_.ERROR_SOMA_ALREADY_DEFINED(lex_.line_num()));
            nb_.soma()->properties() = properties;

            return_id = -1;
        } else {
            SectionType section_type = TokenSectionTypeMap.at(token);
            insertLastPointParentSection(parent_id, properties);

            // Condition to remove single point section that duplicate parent
            // point See test_single_point_section_duplicate_parent for an
            // example
            if (parent_id > -1 && properties._points.size() == 1) {
                return_id = parent_id;
            } else {
                std::shared_ptr<morphio::mut::Section> section;
                if (parent_id > -1)
                    section = nb_.section(parent_id)->appendSection(properties,
                        section_type);
                else
                    section = nb_.appendRootSection(properties, section_type);
                return_id = section->id();
                debugInfo_.setLineNumber(return_id,
                    lex_.current_section_start_);
            }
        }
        points.clear();
        diameters.clear();

        return return_id;
    }

    /*
      Add the last point of parent section to the beginning of this section
      if not already present.

     The idea is that these two structures should represent the same morphology:

     (3 -8 0 2)     and          (3 -8 0 2)
     (3 -10 0 2)                 (3 -10 0 2)
     (                           (
       (0 -10 0 2)                 (3 -10 0 2)  <-- duplicate parent point
       (-3 -10 0 2)                (0 -10 0 2)
       |                           (-3 -10 0 2)
       (6 -10 0 2)                 |
       (9 -10 0 2)                 (3 -10 0 2)  <-- duplicate parent point
     )                             (6 -10 0 2)
                                   (9 -10 0 2)
                                 )
     */
    void insertLastPointParentSection(int32_t parentId,
        morphio::Property::PointLevel& properties)
    {
        if (parentId < 0) // Discard root sections
            return;
        auto parent = nb_.section(parentId);
        auto lastParentPoint = parent->points()[parent->points().size() - 1];
        auto lastParentDiameter = parent->diameters()[parent->diameters().size() - 1];

        if (lastParentPoint == properties._points[0])
            return;

        properties._points.insert(properties._points.begin(), lastParentPoint);
        properties._diameters.insert(properties._diameters.begin(),
            lastParentDiameter);
    }

    bool parse_neurite_section(int32_t parent_id, Token token)
    {
        Points points;
        std::vector<float> diameters;
        uint32_t section_id = nb_.sections().size();

        while (true) {
            const Token id = static_cast<Token>(lex_.current()->id);
            const size_t peek_id = lex_.peek()->id;

            if (is_eof(id)) {
                throw RawDataError(err_.ERROR_EOF_IN_NEURITE(lex_.line_num()));
            } else if (is_end_of_section(id)) {
                if (!points.empty())
                    _create_soma_or_section(token, parent_id, points,
                        diameters);
                return true;
            } else if (is_end_of_branch(id)) {
                lex_.consume();
            } else if (id == Token::LSPINE) {
                // skip spines
                while (!lex_.ended() && static_cast<Token>(lex_.current()->id) != Token::RSPINE) {
                    lex_.consume();
                }
                lex_.consume(Token::RSPINE, "Must be end of spine");
            } else if (id == Token::LPAREN) {
                if (skip_sexp(peek_id)) {
                    // skip words/strings/markers
                    lex_.consume_until_balanced_paren();
                } else if (peek_id == +Token::NUMBER) {
                    Point point;
                    float radius;
                    std::tie(point, radius) = parse_point(lex_);
                    points.push_back(point);
                    diameters.push_back(radius);
                } else if (peek_id == +Token::LPAREN) {
                    if (!points.empty()) {
                        section_id = _create_soma_or_section(token, parent_id,
                            points, diameters);
                    }
                    parse_neurite_branch(section_id, token);
                } else {
                    throw RawDataError(
                        err_.ERROR_UNKNOWN_TOKEN(lex_.line_num(),
                            lex_.peek()->str()));
                }
            } else {
                throw RawDataError(
                    err_.ERROR_UNKNOWN_TOKEN(lex_.line_num(),
                        lex_.peek()->str()));
            }
        }
        return false;
    }

    bool parse_block()
    {
        // parse the top level blocks, and if they are a neurite, otherwise skip
        while (!lex_.ended()) {
            const Token peek_id = static_cast<Token>(lex_.peek()->id);
            if (is_neurite_type(peek_id)) {
                lex_.consume(); // Advance to NeuriteType
                const Token current_id = static_cast<Token>(lex_.current()->id);

                lex_.consume();
                lex_.consume(Token::RPAREN, "New Neurite should end in RPAREN");
                parse_neurite_section(-1, current_id);
            }

            if (!lex_.ended())
                lex_.consume();
        }

        return true;
    }

    morphio::mut::Morphology nb_;

    std::string uri_;
    NeurolucidaLexer lex_;

public:
    DebugInfo debugInfo_;

private:
    ErrorMessages err_;
};

Property::Properties load(const URI& uri, unsigned int options)
{
    NeurolucidaParser parser(uri);

    morphio::mut::Morphology& nb_ = parser.parse();
    nb_.sanitize(parser.debugInfo_);
    nb_.applyModifiers(options);

    Property::Properties properties = nb_.buildReadOnly();
    properties._cellLevel._cellFamily = FAMILY_NEURON;
    properties._cellLevel._version = MORPHOLOGY_VERSION_ASC_1;
    return properties;
}

} // namespace asc
} // namespace plugin
} // namespace morphio
