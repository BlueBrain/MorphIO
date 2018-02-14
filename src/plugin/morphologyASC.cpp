#include <fstream>

#include <minimorph/types.h>
#include <minimorph/sectionBuilder.h>

#include "morphologyASC.h"

#include "lex.cpp"

namespace minimorph
{

namespace plugin
{

namespace asc
{

bool is_eof(Token type)
{
    return type == Token::EOF_;
}

bool is_end_of_branch(Token type)
{
    return (type == Token::GENERATED || type == Token::HIGH
            || type == Token::INCOMPLETE || type == Token::LOW
            || type == Token::NORMAL || type == Token::MIDPOINT);
}

bool is_neurite_type(Token id)
{
    return (id == Token::AXON || id == Token::APICAL || id == Token::DENDRITE
            || id == Token::CELLBODY);
}

bool is_end_of_section(Token id)
{
    return (id == Token::RPAREN || id == Token::PIPE);
}

bool skip_sexp(size_t id)
{
    return (id == +Token::WORD || id == +Token::STRING || id == +Token::COLOR
            || id == +Token::GENERATED || id == +Token::HIGH
            || id == +Token::INCOMPLETE || id == +Token::LOW
            || id == +Token::NORMAL);
}

std::tuple<Point, float> parse_point(NeurolucidaLexer& lex)
{
    lex.expect(Token::LPAREN, "Point should start in LPAREN");

    float x = std::stod(lex.consume()->str());
    float y = std::stod(lex.consume()->str());
    float z = std::stod(lex.consume()->str());
    float r = 0;

    if (lex.peek()->id == +Token::NUMBER)
    {
        r = std::stod(lex.consume()->str());
    }

    lex.consume();

    if (lex.current()->id == +Token::WORD)
    {
        lex.consume(Token::WORD);
    }

    lex.consume(Token::RPAREN, "Point should end in RPAREN");

    return std::tuple<Point, float>{{x, y, z}, r};
}

class NeurolucidaParser
{
public:
    NeurolucidaParser() {};
    NeurolucidaParser(NeurolucidaParser const&) = delete;
    NeurolucidaParser& operator=(NeurolucidaParser const&) = delete;

    Property::Properties parse(const std::string& input)
    {
        lex_.start_parse(input);

        bool ret = parse_block();

        return nb_.buildReadOnly();
    }

private:
    bool parse_neurite_branch(int32_t parent_id, SectionType section_type)
    {
        lex_.consume(Token::LPAREN, "New branch should start with LPAREN");

        bool ret = true;
        while (true)
        {
            ret &= parse_neurite_section(parent_id, section_type);
            if (lex_.ended() || (lex_.current()->id != +Token::PIPE
                                 && lex_.current()->id != +Token::LPAREN))
            {
                break;
            }
            lex_.consume();
        }
        lex_.consume(Token::RPAREN, "Branch should end with RPAREN");
        return ret;
    }

    bool parse_neurite_section(int32_t parent_id, SectionType section_type)
    {
        Points points;
        std::vector<float> diameters;
        uint32_t section_id = nb_.sections().size();

        while (true)
        {
            const Token id = static_cast<Token>(lex_.current()->id);
            const size_t peek_id = lex_.peek()->id;

            if (is_eof(id))
            {
                throw std::runtime_error(
                    "Hit end of of file while consuming a neurite");
            }
            else if (is_end_of_section(id))
            {
                if (!points.empty())
                {
                    minimorph::Property::PointLevel properties;
                    properties._points = points;
                    properties._diameters = diameters;
                    std::cout << "section_type: " << section_type << std::endl;
                    if(section_type == SECTION_SOMA){
                        assert(nb_.soma().type() == SECTION_UNDEFINED &&
                               "found two soma sections");
                        nb_.soma() = builder::Soma(properties, SECTION_SOMA);
                    } else {
                        if(parent_id == -1)
                            section_id = nb_.createNeurite(section_type,
                                                           properties);
                        else
                            section_id = nb_.appendSection(nb_.sections()[parent_id],
                                                           section_type,
                                                           properties);
                    }
                    points.clear();
                    diameters.clear();
                }
                return true;
            }
            else if (is_end_of_branch(id))
            {
                lex_.consume();
            }
            else if (id == Token::LSPINE)
            {
                // skip spines
                while (!lex_.ended() && static_cast<Token>(lex_.current()->id) != Token::RSPINE)
                {
                    lex_.consume();
                }
                lex_.consume(Token::RSPINE, "Must be end of spine");
            }
            else if (id == Token::LPAREN)
            {
                if (skip_sexp(peek_id))
                {
                    // skip words/strings/markers
                    lex_.consume_until_balanced_paren();
                }
                else if (peek_id == +Token::NUMBER)
                {
                    Point point;
                    float radius;
                    std::tie(point, radius) = parse_point(lex_);
                    points.push_back(point);
                    diameters.push_back(radius);
                }
                else if (peek_id == +Token::LPAREN)
                {
                    if (!points.empty())
                    {
                        minimorph::Property::PointLevel properties;
                        properties._points = points;
                        properties._diameters = diameters;
                        if(section_type == SECTION_SOMA){
                            assert(nb_.soma().type() == SECTION_UNDEFINED &&
                                   "found two soma sections");
                            nb_.soma() = builder::Soma(properties, SECTION_SOMA);
                        } else {
                            if(parent_id == -1)
                                section_id = nb_.createNeurite(section_type,
                                                               properties);
                            else
                                section_id = nb_.appendSection(nb_.sections()[parent_id],
                                                               section_type,
                                                               properties);
                        }
                        points.clear();
                        diameters.clear();
                    }
                    parse_neurite_branch(section_id, section_type);
                }
                else
                {
                    throw std::runtime_error(
                        "Unknown token after LPAREN in neurite parse");
                }
            }
            else
            {
                std::cout << "Default: Id: " << id << ", Token: '"
                          << lex_.current()->str() << "'\n";
                throw std::runtime_error("Unknown token in neurite parse");
            }
        }
        return false;
    }

    bool parse_block()
    {
        // parse the top level blocks, and if they are a neurite, otherwise skip
        while (!lex_.ended())
        {
            const Token peek_id = static_cast<Token>(lex_.peek()->id);
            if (is_neurite_type(peek_id))
            {
                lex_.consume(); // Advance to NeuriteType
                std::cout << "Neurite: " << lex_.current()->str() << "\n";
                const Token current_id = static_cast<Token>(lex_.current()->id);
                SectionType section_type = TokenSectionTypeMap.at(current_id);

                lex_.consume();
                lex_.consume(Token::RPAREN, "New Neurite should end in RPAREN");
                parse_neurite_section(-1, section_type);
            }

            if (!lex_.ended())
                lex_.consume();
        }

        return true;
    }

    NeurolucidaLexer lex_;
    minimorph::builder::Morphology nb_;
};


Property::Properties load(const URI& uri) {
    NeurolucidaParser parser;
    std::ifstream ifs(uri);
    std::string input((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

    Property::Properties _properties = parser.parse(input);
    _properties._cellLevel._cellFamily = FAMILY_NEURON;
    return _properties;
}

} // namespace asc

} // namespace plugin

} // namespace minimorph
