// EBNF.hpp --- ISO EBNF notation manipulator
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#ifndef EBNF_HPP_
#define EBNF_HPP_   15  // Version 15

/////////////////////////////////////////////////////////////////////////

#include <string>       // for std::string
#include <vector>       // for std::vector
#include <sstream>      // for std::stringstream
#include <cassert>      // for assert macro
#include <cstring>      // for std::strlen, std::strtol, ...

#include "bnf_ast.hpp"  // for bnf_ast::BaseAst, ...

/////////////////////////////////////////////////////////////////////////

#if defined(NDEBUG) || 1
    #define PRINT_FUNCTION()    /*empty*/
#else
    #define PRINT_FUNCTION()    printf("%s\n", __func__);
#endif

/////////////////////////////////////////////////////////////////////////
// ISO EBNF notation manipulator

#define ISO_EBNF     // ISO/IEC 14977 : 1996(E)

namespace EBNF
{
    using namespace bnf_ast;

    typedef std::string         string_type;
    typedef std::stringstream   os_type;

    /////////////////////////////////////////////////////////////////////////
    // character classification

    inline bool is_digit(char ch)
    {
        return '0' <= ch && ch <= '9';
    }
    inline bool is_octal(char ch)
    {
        return '0' <= ch && ch <= '7';
    }
    inline bool is_xdigit(char ch)
    {
        return is_digit(ch) || ('a' <= ch && ch <= 'f') || ('A' <= ch && ch <= 'F');
    }
    inline bool is_lower(char ch)
    {
        return 'a' <= ch && ch <= 'z';
    }
    inline bool is_upper(char ch)
    {
        return 'A' <= ch && ch <= 'Z';
    }
    inline bool is_alpha(char ch)
    {
        return is_lower(ch) || is_upper(ch);
    }
    inline bool is_alnum(char ch)
    {
        return is_alpha(ch) || is_digit(ch);
    }
    inline bool is_csymf(char ch)
    {
        return is_alpha(ch) || ch == '_';
    }
    inline bool is_csym(char ch)
    {
        return is_alnum(ch) || ch == '_';
    }
    inline bool is_space(char ch)
    {
        return strchr(" \t\n\r\f\v", ch) && ch;
    }

    /////////////////////////////////////////////////////////////////////////
    // AuxItem and AuxInfo

    struct AuxItem
    {
        size_t      m_line;
        string_type m_text;
    };

    struct AuxInfo
    {
        std::vector<AuxItem>    m_errors;
        std::vector<AuxItem>    m_warnings;

        void add_error(const string_type& msg, size_t line)
        {
            AuxItem item;
            item.m_line = line;
            item.m_text = msg;
            m_errors.push_back(item);
        }
        void add_warning(const string_type& msg, size_t line)
        {
            AuxItem item;
            item.m_line = line;
            item.m_text = msg;
            m_warnings.push_back(item);
        }
        void clear_errors()
        {
            m_errors.clear();
            m_warnings.clear();
        }

        void err_out(os_type& os) const;
    };

    /////////////////////////////////////////////////////////////////////////
    // TokenType and Token

    enum TokenType
    {
        TOK_IDENT,
        TOK_INTEGER,
        TOK_STRING,
        TOK_SYMBOL,
        TOK_COMMENT,
        TOK_SPECIAL,
        TOK_EOF
    };

    // TOK_IDENT: meta_identifier
    // TOK_INTEGER: integer
    // TOK_STRING: terminal_string
    // TOK_SYMBOL: '=', ';', '|', ',', '-', '*', '[', ']', '{', '}', '(', ')'
    // TOK_COMMENT: "(*" ... "*)"
    // TOK_SPECIAL: "?" ... "?"

    class Token
    {
    public:
        string_type m_str;
        TokenType   m_type;
        int         m_integer;
        size_t      m_line;

        Token(string_type str, TokenType type, size_t line = 0)
            : m_str(str), m_type(type), m_line(line)
        {
            if (type == TOK_INTEGER)
            {
                m_integer = (int)std::strtol(str.c_str(), NULL, 10);
            }
        }
        void to_dbg(os_type& os) const
        {
            os << "[TOKEN: " << m_type << ", '" << m_str << "']";
        }
    };
    typedef std::vector<Token> tokens_type;

    /////////////////////////////////////////////////////////////////////////
    // StringScanner

    class StringScanner
    {
    public:
        StringScanner(const string_type& str) : m_str(str), m_index(0)
        {
        }
        char getch()
        {
            if (m_index < m_str.size())
            {
                return m_str[m_index++];
            }
            return 0;
        }
        void nextch()
        {
            if (m_index < m_str.size())
            {
                ++m_index;
            }
        }
        void ungetch()
        {
            if (m_index > 0)
                --m_index;
        }
        const char *peek() const
        {
            return &m_str.c_str()[m_index];
        }
        char peekch() const
        {
            return *peek();
        }
        bool match_get(const char *psz)
        {
            const size_t len = strlen(psz);
            if (memcmp(peek(), psz, len) == 0)
            {
                skip(len);
                return true;
            }
            return false;
        }
        bool match_get(const char *psz, string_type& str)
        {
            const size_t len = strlen(psz);
            if (memcmp(peek(), psz, len) == 0)
            {
                str = psz;
                skip(len);
                return true;
            }
            return false;
        }
        void skip(size_t count)
        {
            if (m_index + count <= m_str.size())
                m_index += count;
        }
        size_t index() const
        {
            return m_index;
        }
        void index(size_t pos)
        {
            m_index = pos;
        }

        bool scan_special(string_type& ret);
        bool scan_comment(string_type& ret);
        bool scan_meta_identifier(string_type& ret);
        bool scan_integer(string_type& ret);
        bool scan_terminal_string(string_type& ret);

        size_t index_to_line(size_t index) const;
        size_t line_to_index(size_t line) const;

    protected:
        string_type     m_str;
        size_t          m_index;
    };

    /////////////////////////////////////////////////////////////////////////
    // TokenStream

    class TokenStream
    {
    public:
        tokens_type             m_tokens;

        TokenStream(StringScanner& scanner, AuxInfo& aux);
        bool scan();
        void fixup()
        {
            delete_comments();
            join_words();
        }

        void delete_comments();
        void join_words();

              Token& token();
        const Token& token() const;
        void unget(size_t count = 1);
        bool next();

        TokenType type() const;
        string_type str() const;
        int integer() const;

        size_t index() const;
        bool index(size_t pos);

        size_t get_line() const;
        size_t size() const;

        void to_dbg(os_type& os) const;

        void push_back(const Token& t);

        Token& operator[](size_t i)
        {
            return m_tokens[i];
        }
        const Token& operator[](size_t i) const
        {
            return m_tokens[i];
        }

    protected:
        size_t          m_index;
        StringScanner&  m_scanner;
        AuxInfo&        m_aux;

        char getch()
        {
            return m_scanner.getch();
        }
        char peekch() const
        {
            return m_scanner.peekch();
        }
        void ungetch()
        {
            m_scanner.ungetch();
        }
        void nextch()
        {
            m_scanner.nextch();
        }
    };

    /////////////////////////////////////////////////////////////////////////

    class Parser
    {
    public:
        Parser(TokenStream& stream, AuxInfo& aux)
            : m_stream(stream), m_aux(aux), m_ast(NULL), m_line(0)
        {
        }
        virtual ~Parser()
        {
            delete m_ast;
        }

        BaseAst *ast() const
        {
            return m_ast;
        }

        BaseAst *detach()
        {
            BaseAst *ast = m_ast;
            m_ast = NULL;
            return ast;
        }

        bool parse();

        BaseAst *visit_syntax();
        BaseAst *visit_syntax_rule();
        BaseAst *visit_definitions_list();
        BaseAst *visit_single_definition();
        BaseAst *visit_term();
        BaseAst *visit_exception();
        BaseAst *visit_factor();
        BaseAst *visit_primary();
        BaseAst *visit_optional_sequence();
        BaseAst *visit_repeated_sequence();
        BaseAst *visit_grouped_sequence();

    protected:
        TokenStream     m_stream;
        AuxInfo&        m_aux;
        BaseAst        *m_ast;
        size_t          m_line;

        size_t index() const
        {
            return m_stream.index();
        }
        void index(size_t i)
        {
            m_stream.index(i);
        }
        bool next()
        {
            return m_stream.next();
        }
        Token& token()
        {
            return m_stream.token();
        }
        const Token& token() const
        {
            return m_stream.token();
        }
        TokenType type() const
        {
            return m_stream.type();
        }
        string_type str() const
        {
            return m_stream.str();
        }
        int integer() const
        {
            return m_stream.integer();
        }
        size_t get_line() const
        {
            return m_stream.token().m_line;
        }
    };

    /////////////////////////////////////////////////////////////////////////
    // AuxInfo inlines

    inline void AuxInfo::err_out(os_type& os) const
    {
        for (size_t i = 0; i < m_errors.size(); ++i)
        {
            const AuxItem& item = m_errors[i];
            os << "ERROR: " << item.m_text << ", at line " << item.m_line << std::endl;
        }
        for (size_t i = 0; i < m_warnings.size(); ++i)
        {
            const AuxItem& item = m_warnings[i];
            os << "WARNING: " << item.m_text << ", at line " << item.m_line << std::endl;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    // TokenStream inlines

    inline void TokenStream::to_dbg(os_type& os) const
    {
        if (m_tokens.size())
        {
            m_tokens[0].to_dbg(os);
            for (size_t i = 1; i < m_tokens.size(); ++i)
            {
                os << ", ";
                m_tokens[i].to_dbg(os);
            }
        }
        os << "\n";
    }

    inline TokenStream::TokenStream(StringScanner& scanner, AuxInfo& aux)
        : m_index(0), m_scanner(scanner), m_aux(aux)
    {
    }

    inline void TokenStream::unget(size_t count/* = 1*/)
    {
        if (count <= m_index)
            m_index -= count;
        else
            m_index = 0;
    }

    inline bool TokenStream::next()
    {
        if (m_index + 1 < size())
        {
            ++m_index;
            return true;
        }
        return false;
    }

    inline void TokenStream::push_back(const Token& t)
    {
        m_tokens.push_back(t);
    }

    inline Token& TokenStream::token()
    {
        assert(m_index <= size());
        return m_tokens[m_index];
    }

    inline const Token& TokenStream::token() const
    {
        assert(m_index <= size());
        return m_tokens[m_index];
    }

    inline TokenType TokenStream::type() const
    {
        return token().m_type;
    }

    inline string_type TokenStream::str() const
    {
        return token().m_str;
    }

    inline int TokenStream::integer() const
    {
        return token().m_integer;
    }

    inline size_t TokenStream::index() const
    {
        return m_index;
    }

    inline size_t TokenStream::size() const
    {
        return m_tokens.size();
    }

    inline bool TokenStream::index(size_t pos)
    {
        if (pos <= size())
        {
            m_index = pos;
            return true;
        }
        return false;
    }

    size_t TokenStream::get_line() const
    {
        return m_scanner.index_to_line(m_scanner.index());
    }

    inline bool TokenStream::scan()
    {
        m_tokens.clear();

        char ch;
        string_type str;

        for (;;)
        {
            for (;;)
            {
                ch = peekch();
                if (!is_space(ch))
                    break;
                // space
                nextch();
            }

            if (is_digit(ch))
            {
                // integer
                size_t line = get_line();
                m_scanner.scan_integer(str);
                Token token(str, TOK_INTEGER, line);
                m_tokens.push_back(token);
                continue;
            }

            if (ch == '"' || ch == '\'')
            {
                // terminal_string
                size_t line = get_line();
                if (m_scanner.scan_terminal_string(str))
                {
                    Token token(str, TOK_STRING, line);
                    m_tokens.push_back(token);
                    continue;
                }
                m_aux.add_error("terminal string is invalid", line);
                return false;
            }

            if (is_alpha(ch))
            {
                // meta_identifier
                size_t line = get_line();
                m_scanner.scan_meta_identifier(str);
                Token token(str, TOK_IDENT, line);
                m_tokens.push_back(token);
                continue;
            }

            if (m_scanner.match_get("(*"))  // )
            {
                // comment
                size_t line = get_line();
                if (m_scanner.scan_comment(str))
                {
                    Token token(str, TOK_COMMENT, line);
                    m_tokens.push_back(token);
                    continue;
                }
                m_aux.add_error("no end of comment", line);
                return false;
            }

            if (ch == '?')
            {
                // special
                nextch();
                size_t line = get_line();
                if (m_scanner.scan_special(str))
                {
                    Token token(str, TOK_SPECIAL, line);
                    m_tokens.push_back(token);
                    continue;
                }
                // no end of special
                m_aux.add_error("no end of special", line);
                return false;
            }

            if (ch == 0)
            {
                // end of file
                size_t line = get_line();
                Token token("", TOK_EOF, line);
                m_tokens.push_back(token);
                break;
            }
            else if (strchr("=;|,-*[]{}()", ch) != NULL)
            {
                // symbol
                nextch();
                size_t line = get_line();
                str.clear();
                str += ch;
                Token token(str, TOK_SYMBOL, line);
                m_tokens.push_back(token);
                continue;
            }

            // invalid character
            string_type msg = "invalid character: '";
            msg += ch;
            msg += "'";
            m_aux.add_error(msg, get_line());
            break;
        }

        return m_aux.m_errors.empty();
    }

    inline void TokenStream::join_words()
    {
        for (size_t i = 0; i < m_tokens.size() - 1; ++i)
        {
            if (m_tokens[i].m_type == TOK_IDENT &&
                m_tokens[i + 1].m_type == TOK_IDENT)
            {
                m_tokens[i].m_str += "-";
                m_tokens[i].m_str += m_tokens[i + 1].m_str;
                m_tokens.erase(m_tokens.begin() + (i + 1));
                --i;
            }
        }
    }

    inline void TokenStream::delete_comments()
    {
        for (size_t i = m_tokens.size(); i > 0; )
        {
            --i;
            if (m_tokens[i].m_type == TOK_COMMENT)
            {
                m_tokens.erase(m_tokens.begin() + i);
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////
    // StringScanner inlines

    inline bool StringScanner::scan_comment(string_type& ret)
    {
        ret.clear();

        for (;;)
        {
            // (*
            if (match_get("*)"))
                return true;

            char ch = getch();
            if (ch == 0)
                break;

            ret += ch;
        }

        return false;
    }

    inline bool StringScanner::scan_special(string_type& ret)
    {
        ret.clear();

        for (;;)
        {
            char ch = getch();
            if (ch == 0)
                break;
            if (ch == '?')
                return true;
            ret += ch;
        }
        return false;
    }


    // meta_identifier = letter, { letter | decimal_digit };
    inline bool StringScanner::scan_meta_identifier(string_type& ret)
    {
        ret.clear();

        char ch = *peek();
        if (!is_alpha(ch))
            return false;

        ch = getch();
        ret += ch;
        for (;;)
        {
            ch = getch();
            if (ch == 0)
                break;
            if (!is_alnum(ch) && ch != '-')
            {
                ungetch();
                break;
            }
            ret += ch;
        }

        return true;
    }

    // integer = decimal_digit, { decimal digit };
    inline bool StringScanner::scan_integer(string_type& ret)
    {
        ret.clear();

        char ch = *peek();
        if (!is_digit(ch))
            return false;

        ch = getch();
        ret += ch;
        for (;;)
        {
            ch = getch();
            if (ch == 0)
                break;
            if (!is_digit(ch))
            {
                ungetch();
                break;
            }
            ret += ch;
        }

        return true;
    }

    // terminal string = "'", character - "'", {character - "'"}, "'"
    //                 | '"', character - '"', {character - '"'}, '"';
    inline bool StringScanner::scan_terminal_string(string_type& ret)
    {
        ret.clear();

        char ch = getch();
        if (ch == 0)
            return false;

        if (ch != '"' && ch != '\'')
        {
            ungetch();
            return false;
        }
        const char first_ch = ch;

#ifdef ISO_EBNF
        ch = getch();
        if (ch == first_ch)
        {
            // an empty string is not acceptable
            ungetch();
            ungetch();
            return false;
        }
        ungetch();
#endif

        for (;;)
        {
            ch = getch();
            if (ch == 0)
            {
                // no end of string
                return false;
            }
            if (ch == first_ch)
            {
                break;
            }
            ret += ch;
        }

        return true;
    }

    inline size_t StringScanner::index_to_line(size_t index) const
    {
        size_t line_count = 1;
        for (size_t i = 0; i < m_str.size(); ++i)
        {
            if (i == index)
                return line_count;

            if (m_str[i] == '\n')
            {
                ++line_count;
            }
        }
        return line_count;
    }

    inline size_t StringScanner::line_to_index(size_t line) const
    {
        if (line <= 1)
            return 0;

        size_t index, line_count = 1;
        for (index = 0; index < m_str.size(); ++index)
        {
            if (m_str[index] == '\n')
            {
                ++line_count;
                if (line_count == line)
                    return index + 1;
            }
        }
        return index;
    }

    /////////////////////////////////////////////////////////////////////////
    // Parser inlines

    inline bool Parser::parse()
    {
        if (m_stream.size() == 0)
            return false;

        delete m_ast;
        m_ast = visit_syntax();
        if (m_ast != NULL && type() == TOK_EOF)
            return true;

        delete m_ast;
        m_ast = NULL;
        return false;
    }

    // syntax = syntax_rule, {syntax_rule};
    // syntax is SeqAst("rules").
    inline BaseAst *Parser::visit_syntax()
    {
        PRINT_FUNCTION();

        BaseAst *rule = visit_syntax_rule();
        if (rule == NULL)
        {
            return NULL;
        }

        SeqAst *seq = new SeqAst("rules");
        for (;;)
        {
            seq->push_back(rule);

            if (type() == TOK_EOF)
                break;

            rule = visit_syntax_rule();
            if (rule == NULL)
            {
                delete seq;
                return NULL;
            }
        }
        return seq;
    }

    // syntax_rule = meta_identifier, '=', definitions_list, ';';
    // syntax_rule is BinaryAst(IdentAst, SeqAst("expr")).
    inline BaseAst *Parser::visit_syntax_rule()
    {
        PRINT_FUNCTION();

        if (type() != TOK_IDENT)
        {
            m_aux.add_error("expected TOK_IDENT", get_line());
            return NULL;
        }
        IdentAst *id = new IdentAst(str());
        next();
        if (type() != TOK_SYMBOL || str() != "=")
        {
            m_aux.add_error("expected '='", get_line());
            delete id;
            return NULL;
        }
        next();
        BaseAst *def_list = visit_definitions_list();
        if (def_list == NULL)
        {
            delete id;
            return NULL;
        }
        if (type() != TOK_SYMBOL || str() != ";")
        {
            m_aux.add_error("expected ';' or ','", get_line());
            delete id;
            delete def_list;
            return NULL;
        }
        next();

        BinaryAst *bin = new BinaryAst("rule", id, def_list);
        return bin;
    }

    // definitions_list = single_definition, {'|', single_definition};
    // definitions_list is SeqAst("expr").
    inline BaseAst *Parser::visit_definitions_list()
    {
        PRINT_FUNCTION();

        BaseAst *ast = visit_single_definition();
        if (ast == NULL)
            return NULL;

        SeqAst *seq = new SeqAst("expr");
        for (;;)
        {
            seq->push_back(ast);

            if (type() == TOK_SYMBOL && str() == "|")
            {
                next();
            }
            else
            {
                break;
            }

            ast = visit_single_definition();
            if (ast == NULL)
            {
                delete seq;
                return NULL;
            }
        }
        return seq;
    }

    // single_definition = term, {',', term};
    // single_definition is SeqAst("terms").
    inline BaseAst *Parser::visit_single_definition()
    {
        PRINT_FUNCTION();

        BaseAst *term = visit_term();
        if (term == NULL)
            return NULL;

        SeqAst *seq = new SeqAst("terms");
        for (;;)
        {
            seq->push_back(term);

            if (type() == TOK_SYMBOL && str() == ",")
                next();
            else
                break;

            term = visit_term();
            if (term == NULL)
            {
                delete seq;
                return NULL;
            }
        }
        return seq;
    }

    // term = factor, ['-', exception];
    // term is factor or BinaryAst("-", factor, exception).
    inline BaseAst *Parser::visit_term()
    {
        PRINT_FUNCTION();

        BaseAst *fact = visit_factor();
        if (fact == NULL)
            return NULL;

        if (type() == TOK_SYMBOL && str() == "-")
        {
            next();
            BaseAst *ex = visit_exception();
            if (ex)
            {
                BaseAst *ret = new BinaryAst("-", fact, ex);
                return ret;
            }
            delete fact;
            return NULL;
        }
        return fact;
    }

    // exception = factor;
    // exception is factor.
    inline BaseAst *Parser::visit_exception()
    {
        PRINT_FUNCTION();

        BaseAst *ast = visit_factor();
        return ast;
    }

    // factor = [integer, '*'], primary;
    // factor is primary or BinaryAst("*", IntegerAst, primary).
    inline BaseAst *Parser::visit_factor()
    {
        PRINT_FUNCTION();

        if (type() == TOK_INTEGER)
        {
            int inte = integer();
            next();
            if (type() == TOK_SYMBOL && str() == "*")
            {
                next();
                IntegerAst *i_ast = new IntegerAst(inte);
                BaseAst *prim = visit_primary();
                if (prim)
                {
                    return new BinaryAst("*", i_ast, prim);
                }
                delete i_ast;
            }
            m_aux.add_error("expected '*'", get_line());
            return NULL;
        }

        BaseAst *ret = visit_primary();
        return ret;
    }

    // primary = optional_sequence | repeated_sequence |
    //           special_sequence | grouped_sequence |
    //           meta_identifier | terminal_string | empty;
    // primary is StringAst, IdentAst, SpecialAst, SeqAst, or EmptyAst*/.
    inline BaseAst *Parser::visit_primary()
    {
        PRINT_FUNCTION();

        BaseAst *ret;
        switch (type())
        {
        case TOK_STRING:
            ret = new StringAst(str());
            next();
            break;
        case TOK_IDENT:
            ret = new IdentAst(str());
            next();
            break;
        case TOK_SPECIAL:
            ret = new SpecialAst(str());
            next();
            break;
        case TOK_SYMBOL:
            if (str() == "[")   // ]
            {
                ret = visit_optional_sequence();
                break;
            }
            if (str() == "{")   // }
            {
                ret = visit_repeated_sequence();
                break;
            }
            if (str() == "(")   // )
            {
                ret = visit_grouped_sequence();
                break;
            }
            if (str() == ";" || str() == "|" || str() == "," ||
                str() == ")" || str() == "}" || str() == "]")
            {
                ret = new EmptyAst();
                break;
            }
            ret = NULL;
            break;
        default:
            ret = NULL;
            break;
        }
        return ret;
    }

    // optional_sequence = '[', definitions_list, ']';
    // optional_sequence is UnaryAst.
    inline BaseAst *Parser::visit_optional_sequence()
    {
        PRINT_FUNCTION();
        BaseAst *ret;
        if (type() != TOK_SYMBOL || str() != "[")
        {
            m_aux.add_error("expected '['", get_line());
            return NULL;
        }
        next();
        ret = visit_definitions_list();
        if (ret == NULL)
        {
            delete ret;
            return NULL;
        }
        if (type() != TOK_SYMBOL || str() != "]")
        {
            m_aux.add_error("']' unmatched", get_line());
            delete ret;
            return NULL;
        }
        next();
        ret = new UnaryAst("optional", ret);
        return ret;
    }

    // repeated_sequence = '{', definitions_list, '}';
    // repeated_sequence is UnaryAst.
    inline BaseAst *Parser::visit_repeated_sequence()
    {
        PRINT_FUNCTION();
        BaseAst *ret;
        if (type() != TOK_SYMBOL || str() != "{")
        {
            m_aux.add_error("expected '{'", get_line());    // }
            return NULL;
        }
        next();
        ret = visit_definitions_list();
        if (ret == NULL)
        {
            delete ret;
            return NULL;
        }
        if (type() != TOK_SYMBOL || str() != "}")
        {
            m_aux.add_error("'}' unmatched", get_line());
            delete ret;
            return NULL;
        }
        next();
        ret = new UnaryAst("repeated", ret);
        return ret;
    }

    // grouped_sequence = '(', definitions_list, ')';
    // grouped_sequence is UnaryAst.
    inline BaseAst *Parser::visit_grouped_sequence()
    {
        PRINT_FUNCTION();

        BaseAst *ret;
        if (type() != TOK_SYMBOL || str() != "(")
        {
            m_aux.add_error("expected '('", get_line());
            return NULL;
        }
        next();
        ret = visit_definitions_list();
        if (ret == NULL)
        {
            delete ret;
            return NULL;
        }
        if (type() != TOK_SYMBOL || str() != ")")
        {
            m_aux.add_error("')' unmatched", get_line());
            delete ret;
            return NULL;
        }
        next();
        ret = new UnaryAst("group", ret);
        return ret;
    }
} // namespace EBNF

/////////////////////////////////////////////////////////////////////////

#endif  // ndef EBNF_HPP_
