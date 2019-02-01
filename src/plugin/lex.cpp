#include <morphio/errorMessages.h>
#include <morphio/types.h>

#include <lexertl/debug.hpp>
#include <lexertl/generator.hpp>
#include <lexertl/iterator.hpp>
#include <lexertl/lookup.hpp>

namespace morphio {
namespace plugin {
namespace asc {
enum class Token
{
    EOF_,
    WS = 1,
    NEWLINE,
    COMMENT,
    LPAREN,
    RPAREN,
    LSPINE,
    RSPINE,
    COMMA,
    PIPE,
    WORD,
    STRING,
    NUMBER,

    // neurite types
    AXON,
    APICAL,
    DENDRITE,
    CELLBODY,

    // Special WORDS
    COLOR = 101,
    RGB,

    // end of branch weirdness
    GENERATED,
    HIGH,
    INCOMPLETE,
    LOW,
    NORMAL,
    MIDPOINT,
};

const std::map<Token, SectionType> TokenSectionTypeMap{
    {Token::AXON, SECTION_AXON},
    {Token::APICAL, SECTION_APICAL_DENDRITE},
    {Token::DENDRITE, SECTION_DENDRITE}};

constexpr bool operator==(int lhs, Token type)
{
    return lhs == static_cast<int>(type);
}

inline std::ostream& operator<<(std::ostream& s, const Token& t)
{
    switch (t) {
#define Q(x) #x
#define T(TOK)       \
    case Token::TOK: \
        s << Q(TOK); \
        break;
        T(EOF_)
        T(WS)
        T(NEWLINE)
        T(COMMENT)
        T(LPAREN)
        T(RPAREN)
        T(LSPINE)
        T(RSPINE)
        T(COMMA)
        T(PIPE)
        T(WORD)
        T(STRING)
        T(NUMBER)
        T(AXON)
        T(APICAL)
        T(DENDRITE)
        T(CELLBODY)
        T(COLOR)
        T(RGB)
        T(GENERATED)
        T(HIGH)
        T(INCOMPLETE)
        T(LOW)
        T(NORMAL)
        T(MIDPOINT)
    default:
        s << "Unknown";
#undef T
#undef Q
    }

    return s;
}

constexpr std::size_t operator+(Token type)
{
    return static_cast<std::size_t>(type);
}

class NeurolucidaLexer
{
private:
    std::string uri_;
    bool debug_;
    ErrorMessages err_;

    lexertl::state_machine sm_;

    lexertl::siterator current_;
    lexertl::siterator next_;

    mutable size_t current_line_num_ = 1;
    mutable size_t next_line_num_ = 1;

public:
    size_t current_section_start_ = 0;

    NeurolucidaLexer(const std::string& uri, bool debug = false)
        : uri_(uri)
        , debug_(debug)
        , err_(uri)
    {
        lexertl::rules rules;
        build_lexer(rules, sm_);
    }

    void start_parse(const std::string& input)
    {
        current_ = next_ = lexertl::siterator(input.begin(), input.end(), sm_);
        // will set the above, current_ to next_, AND consume whitespace
        size_t n_skipped = skip_whitespace(current_);
        current_line_num_ += n_skipped;
        next_line_num_ += n_skipped;

        consume();
    }

    void build_lexer(lexertl::rules& rules_, lexertl::state_machine& sm_)
    {
        rules_.push("\n", +Token::NEWLINE);
        rules_.push("[ \t\r]+", +Token::WS);
        rules_.push(";[^\n]*", +Token::COMMENT);

        rules_.push("\\(", +Token::LPAREN);
        rules_.push("\\)", +Token::RPAREN);

        rules_.push("<[ \t\r]*\\(", +Token::LSPINE);
        rules_.push("\\)>", +Token::RSPINE);

        rules_.push(",", +Token::COMMA);
        rules_.push("\\|", +Token::PIPE);

        rules_.push("Color", +Token::COLOR);

        rules_.push("Axon", +Token::AXON);
        rules_.push("Apical", +Token::APICAL);
        rules_.push("Dendrite", +Token::DENDRITE);
        // rules_.push("\\\"CellBody\\\"", +Token::CELLBODY);
        rules_.push("CellBody", +Token::CELLBODY);

        rules_.push("Generated", +Token::GENERATED);
        rules_.push("High", +Token::HIGH);
        rules_.push("Incomplete", +Token::INCOMPLETE);
        rules_.push("Low", +Token::LOW);
        rules_.push("Normal", +Token::NORMAL);
        rules_.push("Midpoint", +Token::MIDPOINT);

        rules_.push("\\\"[^\"]*\\\"", +Token::STRING);

        rules_.push("-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?", +Token::NUMBER);
        rules_.push("[a-zA-Z][0-9a-zA-Z]+", +Token::WORD);

        lexertl::generator::build(rules_, sm_);
        sm_.minimise();
        if (debug_) {
            lexertl::debug::dump(sm_, std::cout);
        }
    }

    size_t line_num() const { return current_line_num_; }
    const lexertl::siterator current() const { return current_; }
    const lexertl::siterator peek() const { return next_; }
    static size_t skip_whitespace(lexertl::siterator& iter)
    {
        const lexertl::siterator end;
        size_t endlines = 0;
        while (iter != end) {
            if (iter->id == +Token::NEWLINE) {
                ++endlines;
                ++iter;
            } else if (iter->id == +Token::WS || iter->id == +Token::COMMENT) {
                ++iter;
            } else {
                break;
            }
        }
        return endlines;
    }

    bool ended() const
    {
        const lexertl::siterator end;
        return current() == end;
    }

    const lexertl::siterator consume(Token t, std::string msg = "")
    {
        if (!msg.empty()) {
            expect(t, msg.c_str());
        } else {
            expect(t, "Consume");
        }
        return consume();
    }

    const lexertl::siterator consume()
    {
        const lexertl::siterator end;
        if (ended()) {
            throw RawDataError(err_.ERROR_EOF_REACHED(line_num()));
        }

        lexertl::siterator temp(next_);
        current_ = next_;
        next_ = temp;

        current_line_num_ = next_line_num_;

        if (next_ != end) {
            ++next_;
            next_line_num_ += skip_whitespace(next_);
        }

        if (debug_) {
            state();
        }

        return current_;
    }

    void state() const
    {
        std::cout << "Id: " << Token(current_->id) << ", Token: '"
                  << current_->str() << "' line: " << current_line_num_
                  << " Next Id: " << Token(next_->id) << ", Token: '"
                  << next_->str() << "' line: " << next_line_num_ << std::endl;
    }

    void expect(Token t, const char* msg) const
    {
        if (current()->id != +t) {
            std::stringstream expected;
            expected << t;

            std::stringstream got;
            got << current()->str();

            throw RawDataError(err_.ERROR_UNEXPECTED_TOKEN(line_num(),
                expected.str(),
                got.str(), msg));
        }
    }

    // advance iterator until sexp is consumed, including final paren
    void consume_until_balanced_paren()
    {
        expect(Token::LPAREN,
            "consume_until_balanced_paren should start in LPAREN");
        size_t opening_count = 1;
        while (opening_count != 0) {
            size_t id = consume()->id;
            switch (id) {
            case +Token::RPAREN:
                --opening_count;
                break;
            case +Token::LPAREN:
                ++opening_count;
                break;
            default:
                break;
            }
            if (ended()) {
                throw RawDataError(
                    err_.ERROR_EOF_UNBALANCED_PARENS(line_num()));
            }
        }
        consume(Token::RPAREN,
            "consume_until_balanced_paren should end in RPAREN");
    }
};

} // namespace asc
} // namespace plugin
} // namespace morphio
