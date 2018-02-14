#include "lex.cpp"
#include "neuron_builder.cpp"
#include "point.hpp"

namespace minimorph
{
namespace detail
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

Point parse_point(NeurolucidaLexer& lex)
{
    lex.expect(Token::LPAREN, "Point should start in LPAREN");

    double x = std::stod(lex.consume()->str());
    double y = std::stod(lex.consume()->str());
    double z = std::stod(lex.consume()->str());

    double r = 0;
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

    return {x, y, z};
}

} // namespace detail

class NeurolucidaParser
{
public:
    NeurolucidaParser() {};
    NeurolucidaParser(NeurolucidaParser const&) = delete;
    NeurolucidaParser& operator=(NeurolucidaParser const&) = delete;

    bool parse(std::string& input)
    {
        lex_.start_parse(input);

        bool ret = parse_block();

        std::cout << nb_ << std::endl;
        print_h5(nb_);

        return ret;
    }

private:
    bool parse_neurite_branch(SectionID parent_id, SectionType section_type)
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

    bool parse_neurite_section(SectionID parent_id, SectionType section_type)
    {
        Points points;
        SectionID section_id = nb_.sections_.size();

        while (true)
        {
            const Token id = static_cast<Token>(lex_.current()->id);
            const size_t peek_id = lex_.peek()->id;

            if (detail::is_eof(id))
            {
                throw std::runtime_error(
                    "Hit end of of file while consuming a neurite");
            }
            else if (detail::is_end_of_section(id))
            {
                if (!points.empty())
                {
                    section_id = nb_.add_section(section_id, parent_id,
                                                 section_type, points);
                    points.clear();
                }
                return true;
            }
            else if (detail::is_end_of_branch(id))
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
                if (detail::skip_sexp(peek_id))
                {
                    // skip words/strings/markers
                    lex_.consume_until_balanced_paren();
                }
                else if (peek_id == +Token::NUMBER)
                {
                    Point p = detail::parse_point(lex_);
                    points.push_back(p);
                }
                else if (peek_id == +Token::LPAREN)
                {
                    if (!points.empty())
                    {
                        section_id = nb_.add_section(section_id, parent_id,
                                                     section_type, points);
                        points.clear();
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
            if (detail::is_neurite_type(peek_id))
            {
                lex_.consume(); // Advance to NeuriteType
                std::cout << "Neurite: " << lex_.current()->str() << "\n";

                // TODO: Map token id to neurite type
                SectionType section_type = lex_.current()->id;

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
    NeuronBuilder nb_;
};

} // namespace minimorph
