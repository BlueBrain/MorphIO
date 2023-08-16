/* Copyright (c) 2013-2023, EPFL/Blue Brain Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <morphio/errorMessages.h>
#include <morphio/types.h>

#include <lexertl/debug.hpp>
#include <lexertl/generator.hpp>
#include <lexertl/iterator.hpp>
#include <lexertl/lookup.hpp>

namespace morphio {
namespace readers {
namespace asc {
enum class Token {
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
    FONT,
    MARKER,
    RGB,

    // end of branch weirdness
    GENERATED,
    HIGH,
    INCOMPLETE,
    LOW,
    NORMAL,
    MIDPOINT,
    ORIGIN,
};

inline SectionType TokenToSectionType(Token t) {
    if (t == Token::AXON) {
        return SECTION_AXON;
    } else if (t == Token::APICAL) {
        return SECTION_APICAL_DENDRITE;
    } else if (t == Token::DENDRITE) {
        return SECTION_DENDRITE;
    }
    throw RawDataError("Could not convert token to sectin");
}

inline std::string to_string(Token t) {
    switch (t) {
#define Q(x) #x
#define T(TOK)         \
    case Token::TOK:   \
        return Q(TOK); \
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
        T(FONT)
        T(MARKER)
        T(RGB)
        T(GENERATED)
        T(HIGH)
        T(INCOMPLETE)
        T(LOW)
        T(NORMAL)
        T(MIDPOINT)
        T(ORIGIN)
    default:
        return "Unknown";
#undef T
#undef Q
    }
}

inline std::ostream& operator<<(std::ostream& ostr, Token t) {
    return ostr << to_string(t);
}

constexpr std::size_t operator+(Token type) {
    return static_cast<std::size_t>(type);
}

namespace {
class StateMachineBuilder 
{
public:
    static lexertl::state_machine build() {
        lexertl::rules rules_;
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
        rules_.push("Font", +Token::FONT);

        rules_.push("[Aa]xon", +Token::AXON);
        rules_.push("[Aa]pical", +Token::APICAL);
        rules_.push("[Dd]endrite", +Token::DENDRITE);
        rules_.push("[Cc]ell ?[Bb]ody", +Token::CELLBODY);

        // The code snippet used to infer the marker list is available at:
        // https://github.com/BlueBrain/MorphIO/pull/229
        rules_.push("Dot[0-9]*", +Token::MARKER);
        rules_.push("Plus[0-9]*", +Token::MARKER);
        rules_.push("Cross[0-9]*", +Token::MARKER);
        rules_.push("Splat[0-9]*", +Token::MARKER);
        rules_.push("Flower[0-9]*", +Token::MARKER);
        rules_.push("Circle[0-9]*", +Token::MARKER);
        rules_.push("Flower[0-9]*", +Token::MARKER);
        rules_.push("TriStar[0-9]*", +Token::MARKER);
        rules_.push("OpenStar[0-9]*", +Token::MARKER);
        rules_.push("Asterisk[0-9]*", +Token::MARKER);
        rules_.push("SnowFlake[0-9]*", +Token::MARKER);
        rules_.push("OpenCircle[0-9]*", +Token::MARKER);
        rules_.push("ShadedStar[0-9]*", +Token::MARKER);
        rules_.push("FilledStar[0-9]*", +Token::MARKER);
        rules_.push("TexacoStar[0-9]*", +Token::MARKER);
        rules_.push("MoneyGreen[0-9]*", +Token::MARKER);
        rules_.push("DarkYellow[0-9]*", +Token::MARKER);
        rules_.push("OpenSquare[0-9]*", +Token::MARKER);
        rules_.push("OpenDiamond[0-9]*", +Token::MARKER);
        rules_.push("CircleArrow[0-9]*", +Token::MARKER);
        rules_.push("CircleCross[0-9]*", +Token::MARKER);
        rules_.push("OpenQuadStar[0-9]*", +Token::MARKER);
        rules_.push("DoubleCircle[0-9]*", +Token::MARKER);
        rules_.push("FilledSquare[0-9]*", +Token::MARKER);
        rules_.push("MalteseCross[0-9]*", +Token::MARKER);
        rules_.push("FilledCircle[0-9]*", +Token::MARKER);
        rules_.push("FilledDiamond[0-9]*", +Token::MARKER);
        rules_.push("FilledQuadStar[0-9]*", +Token::MARKER);
        rules_.push("OpenUpTriangle[0-9]*", +Token::MARKER);
        rules_.push("FilledUpTriangle[0-9]*", +Token::MARKER);
        rules_.push("OpenDownTriangle[0-9]*", +Token::MARKER);
        rules_.push("FilledDownTriangle[0-9]*", +Token::MARKER);

        rules_.push("Generated", +Token::GENERATED);
        rules_.push("High", +Token::HIGH);
        rules_.push("Incomplete", +Token::INCOMPLETE);
        rules_.push("Low", +Token::LOW);
        rules_.push("Normal", +Token::NORMAL);
        rules_.push("Midpoint", +Token::MIDPOINT);
        rules_.push("Origin", +Token::ORIGIN);

        rules_.push(R"(\"[^"]*\")", +Token::STRING);

        rules_.push(R"([+-]?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?)", +Token::NUMBER);
        rules_.push("[a-zA-Z][0-9a-zA-Z]+", +Token::WORD);

        lexertl::state_machine sm;

        lexertl::generator::build(rules_, sm);
        sm.minimise();

        // useful to debug
        // lexertl::debug::dump(sm, std::cout);

        return sm;
    }
};

const lexertl::state_machine& lexer_singleton() {
    static lexertl::state_machine sm = StateMachineBuilder::build();
    return sm;
}

}  // namespace


class NeurolucidaLexer
{
  private:
    bool debug_;
    ErrorMessages err_;

    lexertl::siterator current_;
    lexertl::siterator next_;

    size_t current_line_num_ = 1;
    size_t next_line_num_ = 1;

  public:
    explicit NeurolucidaLexer(const std::string& path, bool debug = false)
        : debug_(debug)
        , err_(path) {}

    void start_parse(const std::string& input) {
        const auto& sm = lexer_singleton();
        current_ = next_ = lexertl::siterator(input.begin(), input.end(), sm);

        // will set the above, current_ to next_, AND consume whitespace
        size_t n_skipped = skip_whitespace(current_);
        current_line_num_ += n_skipped;
        next_line_num_ += n_skipped;

        consume();
    }

    size_t line_num() const noexcept {
        return current_line_num_;
    }

    const lexertl::siterator& current() const noexcept {
        return current_;
    }

    const lexertl::siterator& peek() const noexcept {
        return next_;
    }

    size_t skip_whitespace(lexertl::siterator& iter) {
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

    bool ended() const {
        const lexertl::siterator end;
        return current() == end;
    }

    lexertl::siterator consume(Token t, const std::string& msg = "") {
        if (!msg.empty()) {
            expect(t, msg.c_str());
        } else {
            expect(t, "Consume");
        }
        return consume();
    }

    lexertl::siterator consume() {
        const lexertl::siterator end;

        if (ended()) {
            throw RawDataError(err_.ERROR_EOF_REACHED(line_num()));
        }

        current_ = lexertl::siterator{next_};

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

    void state() const {
        std::cout << "Id: " << Token(current_->id) << ", Token: '" << current_->str()
                  << "' line: " << current_line_num_ << " Next Id: " << Token(next_->id)
                  << ", Token: '" << next_->str() << "' line: " << next_line_num_ << '\n';
    }

    void expect(Token t, const char* msg) const {
        if (current()->id != +t) {
            throw RawDataError(
                err_.ERROR_UNEXPECTED_TOKEN(line_num(), to_string(t), current()->str(), msg));
        }
    }

    void consume_until(Token endpoint) {
        while (consume()->id != +endpoint) {
        }
    }

    // advance iterator until sexp is consumed, including final paren
    void consume_until_balanced_paren() {
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
                throw RawDataError(err_.ERROR_EOF_UNBALANCED_PARENS(line_num()));
            }
        }
        consume(Token::RPAREN, "consume_until_balanced_paren should end in RPAREN");
    }
};

}  // namespace asc
}  // namespace readers
}  // namespace morphio
// vim: ft=cpp
