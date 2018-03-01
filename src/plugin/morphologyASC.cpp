#include <fstream>

#include <morphio/mut/morphology.h>
#include <morphio/types.h>

#include "morphologyASC.h"

#include "lex.cpp"

namespace morphio
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
    return (type == Token::GENERATED || type == Token::HIGH ||
            type == Token::INCOMPLETE || type == Token::LOW ||
            type == Token::NORMAL || type == Token::MIDPOINT);
}

bool is_neurite_type(Token id)
{
    return (id == Token::AXON || id == Token::APICAL || id == Token::DENDRITE ||
            id == Token::CELLBODY);
}

bool is_end_of_section(Token id)
{
    return (id == Token::RPAREN || id == Token::PIPE);
}

bool skip_sexp(size_t id)
{
    return (id == +Token::WORD || id == +Token::STRING || id == +Token::COLOR ||
            id == +Token::GENERATED || id == +Token::HIGH ||
            id == +Token::INCOMPLETE || id == +Token::LOW ||
            id == +Token::NORMAL);
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
    NeurolucidaParser(){};
    NeurolucidaParser(NeurolucidaParser const&) = delete;
    NeurolucidaParser& operator=(NeurolucidaParser const&) = delete;

    Property::Properties parse(const std::string& input)
    {
        lex_.start_parse(input);

        bool ret = parse_block();

        return nb_.buildReadOnly();
    }

private:
    bool parse_neurite_branch(int32_t parent_id, Token token)
    {
        lex_.consume(Token::LPAREN, "New branch should start with LPAREN");

        bool ret = true;
        while (true)
        {
            ret &= parse_neurite_section(parent_id, token);
            if (lex_.ended() || (lex_.current()->id != +Token::PIPE &&
                                 lex_.current()->id != +Token::LPAREN))
            {
                break;
            }
            lex_.consume();
        }
        lex_.consume(Token::RPAREN, "Branch should end with RPAREN");
        return ret;
    }

    int32_t _create_soma_or_section(Token token, int32_t parent_id,
                                    std::vector<Point> &points, std::vector<float> &diameters)
    {
        int32_t return_id;
        morphio::Property::PointLevel properties;
        properties._points = points;
        properties._diameters = diameters;
        if(token == Token::CELLBODY){
            if(nb_.soma()->points().size() != 0)
                throw SectionBuilderError("A soma is already defined");
            nb_.soma() = std::make_shared<mut::Soma>(mut::Soma(properties));
            return_id = -1;
        } else {
            SectionType section_type = TokenSectionTypeMap.at(token);
            return_id = nb_.appendSection(parent_id,
                                          section_type,
                                          properties);
        }
        points.clear();
        diameters.clear();
        return return_id;
    }

    bool parse_neurite_section(int32_t parent_id, Token token)
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
                    _create_soma_or_section(token, parent_id, points, diameters);
                return true;
            }
            else if (is_end_of_branch(id))
            {
                lex_.consume();
            }
            else if (id == Token::LSPINE)
            {
                // skip spines
                while (!lex_.ended() &&
                       static_cast<Token>(lex_.current()->id) != Token::RSPINE)
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
                        section_id = _create_soma_or_section(token, parent_id, points, diameters);
                    }
                    parse_neurite_branch(section_id, token);
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

                lex_.consume();
                lex_.consume(Token::RPAREN, "New Neurite should end in RPAREN");
                parse_neurite_section(-1, current_id);
            }

            if (!lex_.ended())
                lex_.consume();
        }

        return true;
    }

    NeurolucidaLexer lex_;
    morphio::mut::Morphology nb_;
};

Property::Properties load(const URI& uri)
{
    NeurolucidaParser parser;
    std::ifstream ifs(uri);
    std::string input((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

    Property::Properties properties = parser.parse(input);
    properties._cellLevel._cellFamily = FAMILY_NEURON;
    properties._cellLevel._version = MORPHOLOGY_VERSION_ASC_1;
    return properties;
}

} // namespace asc
} // namespace plugin
} // namespace morphio
