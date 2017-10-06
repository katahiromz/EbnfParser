// bnf_ast.hpp --- BNF/EBNF AST
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#ifndef BNF_AST_HPP_
#define BNF_AST_HPP_

#include <string>       // for std::string
#include <vector>       // for std::vector
#include <sstream>      // for std::stringstream
#include <cassert>      // for assert macro
#include <algorithm>    // for std::sort

/////////////////////////////////////////////////////////////////////////

namespace bnf_ast
{
    typedef std::string         string_type;
    typedef std::stringstream   os_type;

    enum AstType
    {
        ATYPE_INTEGER,
        ATYPE_STRING,
        ATYPE_BINARY,
        ATYPE_IDENT,
        ATYPE_UNARY,
        ATYPE_SEQ,
        ATYPE_SPECIAL,
        ATYPE_EMPTY
    };

    struct BaseAst
    {
        AstType m_atype;

#ifndef NDEBUG
        static int& alive_count()
        {
            static int s_count = 0;
            return s_count;
        }
#endif

        BaseAst(AstType atype) : m_atype(atype)
        {
            #ifndef NDEBUG
                ++alive_count();
            #endif
        }
        virtual ~BaseAst()
        {
            #ifndef NDEBUG
                --alive_count();
            #endif
        }

        virtual void to_dbg(os_type& os) const = 0;
        virtual void to_bnf(os_type& os) const = 0;
        virtual void to_ebnf(os_type& os) const = 0;
        virtual BaseAst *clone() const = 0;
        virtual BaseAst *sorted_clone() const = 0;
    private:
        BaseAst();
        BaseAst(const BaseAst&);
        BaseAst& operator=(const BaseAst&);
    };

    // comparison
    bool ast_equal(BaseAst *ast1, BaseAst *ast2);
    bool ast_less_than(BaseAst *ast1, BaseAst *ast2);

    struct IdentAst : public BaseAst
    {
        string_type     m_name;

        IdentAst(const string_type& name) : BaseAst(ATYPE_IDENT), m_name(name)
        {
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[IDENT: " << m_name << "]";
        }
        virtual void to_bnf(os_type& os) const
        {
            os << "<" << m_name << ">";
        }
        virtual void to_ebnf(os_type& os) const
        {
            os << m_name;
        }
        virtual BaseAst *clone() const
        {
            return new IdentAst(m_name);
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct IntegerAst : public BaseAst
    {
        int m_integer;

        IntegerAst(int integer) : BaseAst(ATYPE_INTEGER), m_integer(integer)
        {
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[INTEGER: " << m_integer << "]";
        }
        virtual void to_bnf(os_type& os) const
        {
            os << m_integer;
        }
        virtual void to_ebnf(os_type& os) const
        {
            os << m_integer;
        }
        virtual BaseAst *clone() const
        {
            return new IntegerAst(m_integer);
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct StringAst : public BaseAst
    {
        string_type m_str;  // unquoted string

        StringAst(const string_type& str) : BaseAst(ATYPE_STRING), m_str(str)
        {
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[STRING: " << m_str << "]";
        }
        virtual void to_bnf(os_type& os) const
        {
            if (m_str.find('"') == string_type::npos)
                os << '"' << m_str << '"';
            else
                os << "'" << m_str << "'";
        }
        virtual void to_ebnf(os_type& os) const
        {
            if (m_str.find('"') == string_type::npos)
                os << '"' << m_str << '"';
            else
                os << "'" << m_str << "'";
        }
        virtual BaseAst *clone() const
        {
            return new StringAst(m_str);
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct SpecialAst : public BaseAst
    {
        string_type m_str;

        SpecialAst(const string_type& str) : BaseAst(ATYPE_SPECIAL), m_str(str)
        {
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[SPECIAL: " << m_str << "]";
        }
        virtual void to_bnf(os_type& os) const
        {
            os << "..." << m_str << "...";
        }
        virtual void to_ebnf(os_type& os) const
        {
            os << '?' << m_str << '?';
        }
        virtual BaseAst *clone() const
        {
            return new SpecialAst(m_str);
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct UnaryAst : public BaseAst
    {
        string_type m_str;  // "+", "*", "?", "optional", "repeated", or "group"
        BaseAst *m_arg;

        UnaryAst(const string_type& str, BaseAst *arg = NULL)
            : BaseAst(ATYPE_UNARY), m_str(str), m_arg(arg)
        {
        }
        ~UnaryAst()
        {
            delete m_arg;
        }
        virtual void to_dbg(os_type& os) const;
        virtual void to_bnf(os_type& os) const;
        virtual void to_ebnf(os_type& os) const;
        virtual BaseAst *clone() const
        {
            if (m_arg)
            {
                return new UnaryAst(m_str, m_arg->clone());
            }
            return new UnaryAst(m_str);
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct BinaryAst : public BaseAst
    {
        string_type m_str;  // "rule", "-", or "*"
        BaseAst *m_left;
        BaseAst *m_right;

        BinaryAst(const string_type& str, BaseAst *left, BaseAst *right)
            : BaseAst(ATYPE_BINARY), m_str(str), m_left(left), m_right(right)
        {
            assert(m_left);
            assert(m_right);
        }
        ~BinaryAst()
        {
            delete m_left;
            delete m_right;
        }
        virtual void to_dbg(os_type& os) const;
        virtual void to_bnf(os_type& os) const;
        virtual void to_ebnf(os_type& os) const;
        virtual BaseAst *clone() const
        {
            return new BinaryAst(m_str, m_left->clone(), m_right->clone());
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    struct SeqAst : public BaseAst
    {
        string_type m_str;  // "rules", "expr", or "terms"
        std::vector<BaseAst *> m_vec;

        SeqAst(const string_type& str) : BaseAst(ATYPE_SEQ), m_str(str)
        {
        }
        SeqAst(const string_type& str, BaseAst *ast)
            : BaseAst(ATYPE_SEQ), m_str(str)
        {
            assert(ast);
            m_vec.push_back(ast);
        }
        ~SeqAst()
        {
            for (size_t i = 0; i < m_vec.size(); ++i)
            {
                delete m_vec[i];
            }
        }
        void push_back(BaseAst *ast)
        {
            assert(ast);
            m_vec.push_back(ast);
        }
        size_t size() const
        {
            return m_vec.size();
        }
        bool empty() const
        {
            return size() == 0;
        }
        virtual BaseAst *clone() const;
        virtual BaseAst *sorted_clone() const;
        virtual void to_dbg(os_type& os) const;
        virtual void to_bnf(os_type& os) const;
        virtual void to_ebnf(os_type& os) const;
    };

    struct EmptyAst : public BaseAst
    {
        EmptyAst() : BaseAst(ATYPE_EMPTY)
        {
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[EMPTY]";
        }
        virtual void to_bnf(os_type& os) const
        {
            os << "\"\"";
        }
        virtual void to_ebnf(os_type& os) const
        {
        }
        virtual BaseAst *clone() const
        {
            return new EmptyAst();
        }
        virtual BaseAst *sorted_clone() const
        {
            return clone();
        }
    };

    /////////////////////////////////////////////////////////////////////////
    // comparison

    inline bool ast_equal(BaseAst *ast1, BaseAst *ast2)
    {
        assert(ast1);
        assert(ast2);

        if (ast1->m_atype != ast2->m_atype)
            return false;

        switch (ast1->m_atype)
        {
        case ATYPE_INTEGER:
            {
                IntegerAst *i1 = static_cast<IntegerAst *>(ast1);
                IntegerAst *i2 = static_cast<IntegerAst *>(ast2);
                return i1->m_integer == i2->m_integer;
            }
        case ATYPE_STRING:
            {
                StringAst *s1 = static_cast<StringAst *>(ast1);
                StringAst *s2 = static_cast<StringAst *>(ast2);
                return s1->m_str == s2->m_str;
            }
        case ATYPE_BINARY:
            {
                BinaryAst *b1 = static_cast<BinaryAst *>(ast1);
                BinaryAst *b2 = static_cast<BinaryAst *>(ast2);
                if (b1->m_str != b2->m_str)
                    return false;
                return ast_equal(b1->m_left, b2->m_left) &&
                       ast_equal(b1->m_right, b2->m_right);
            }
        case ATYPE_IDENT:
            {
                IdentAst *i1 = static_cast<IdentAst *>(ast1);
                IdentAst *i2 = static_cast<IdentAst *>(ast2);
                return i1->m_name == i2->m_name;
            }
        case ATYPE_UNARY:
            {
                UnaryAst *u1 = static_cast<UnaryAst *>(ast1);
                UnaryAst *u2 = static_cast<UnaryAst *>(ast2);
                if (u1->m_str != u2->m_str)
                    return false;
                if (!u1->m_arg != !u2->m_arg)
                    return false;
                return ast_equal(u1->m_arg, u2->m_arg);
            }
        case ATYPE_SEQ:
            {
                SeqAst *s1 = static_cast<SeqAst *>(ast1);
                SeqAst *s2 = static_cast<SeqAst *>(ast2);
                if (s1->m_str != s2->m_str)
                    return false;
                if (s1->size() != s2->size())
                    return false;
                s1 = static_cast<SeqAst *>(s1->sorted_clone());
                s2 = static_cast<SeqAst *>(s2->sorted_clone());
                for (size_t i = 0; i < s1->size(); ++i)
                {
                    if (!ast_equal(s1->m_vec[i], s2->m_vec[i]))
                    {
                        delete s1;
                        delete s2;
                        return false;
                    }
                }
                delete s1;
                delete s2;
                return true;
            }
        case ATYPE_SPECIAL:
            {
                SpecialAst *spe1 = static_cast<SpecialAst *>(ast1);
                SpecialAst *spe2 = static_cast<SpecialAst *>(ast2);
                return spe1->m_str == spe2->m_str;
            }
        case ATYPE_EMPTY:
            return true;
        }
    }

    inline bool ast_less_than(BaseAst *ast1, BaseAst *ast2)
    {
        assert(ast1);
        assert(ast2);

        if (ast1->m_atype < ast2->m_atype)
            return true;
        if (ast1->m_atype > ast2->m_atype)
            return false;

        switch (ast1->m_atype)
        {
        case ATYPE_INTEGER:
            {
                IntegerAst *i1 = static_cast<IntegerAst *>(ast1);
                IntegerAst *i2 = static_cast<IntegerAst *>(ast2);
                return i1->m_integer < i2->m_integer;
            }
        case ATYPE_STRING:
            {
                StringAst *s1 = static_cast<StringAst *>(ast1);
                StringAst *s2 = static_cast<StringAst *>(ast2);
                return s1->m_str < s2->m_str;
            }
        case ATYPE_BINARY:
            {
                BinaryAst *b1 = static_cast<BinaryAst *>(ast1);
                BinaryAst *b2 = static_cast<BinaryAst *>(ast2);
                if (b1->m_str < b2->m_str)
                    return true;
                if (b1->m_str > b2->m_str)
                    return false;
                if (ast_less_than(b1->m_left, b2->m_left))
                    return true;
                if (!ast_equal(b1->m_left, b2->m_left))
                    return false;
                return ast_less_than(b1->m_right, b2->m_right);
            }
        case ATYPE_IDENT:
            {
                IdentAst *i1 = static_cast<IdentAst *>(ast1);
                IdentAst *i2 = static_cast<IdentAst *>(ast2);
                return i1->m_name < i2->m_name;
            }
        case ATYPE_UNARY:
            {
                UnaryAst *u1 = static_cast<UnaryAst *>(ast1);
                UnaryAst *u2 = static_cast<UnaryAst *>(ast2);
                if (u1->m_str < u2->m_str)
                    return true;
                if (u1->m_str > u2->m_str)
                    return false;
                if (!!u1->m_arg < !!u2->m_arg)
                    return true;
                if (!!u1->m_arg > !!u2->m_arg)
                    return false;
                return ast_less_than(u1->m_arg, u2->m_arg);
            }
        case ATYPE_SEQ:
            {
                SeqAst *s1 = static_cast<SeqAst *>(ast1);
                SeqAst *s2 = static_cast<SeqAst *>(ast2);
                if (s1->m_str < s2->m_str)
                    return true;
                if (s1->m_str > s2->m_str)
                    return false;

                size_t count;
                if (s1->size() < s2->size())
                    count = s1->size();
                else
                    count = s2->size();

                s1 = static_cast<SeqAst *>(s1->sorted_clone());
                s2 = static_cast<SeqAst *>(s2->sorted_clone());

                for (size_t i = 0; i < count; ++i)
                {
                    if (ast_equal(s1->m_vec[i], s2->m_vec[i]))
                    {
                        continue;
                    }
                    else 
                    {
                        bool ret = ast_less_than(s1->m_vec[i], s2->m_vec[i]);
                        delete s1;
                        delete s2;
                        return ret;
                    }
                }
                bool less_than = s1->size() < s2->size();
                delete s1;
                delete s2;
                return less_than;
            }
        case ATYPE_SPECIAL:
            {
                SpecialAst *spe1 = static_cast<SpecialAst *>(ast1);
                SpecialAst *spe2 = static_cast<SpecialAst *>(ast2);
                return spe1->m_str < spe2->m_str;
            }
        case ATYPE_EMPTY:
            return false;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    // AST inlines

    inline void UnaryAst::to_dbg(os_type& os) const
    {
        os << "[UNARY " << m_str << ": ";
        if (m_arg)
        {
            m_arg->to_dbg(os);
        }
        os << "]";
    }

    inline void UnaryAst::to_bnf(os_type& os) const
    {
        if (m_str == "optional")
        {
            os << '[';
            m_arg->to_bnf(os);
            os << ']';
            return;
        }
        if (m_str == "repeated")
        {
            os << '{';
            m_arg->to_bnf(os);
            os << '}';
            return;
        }
        if (m_str == "group")
        {
            os << '(';
            m_arg->to_bnf(os);
            os << ')';
            return;
        }
        if (m_str == "+" || m_str == "*" || m_str == "?")
        {
            m_arg->to_bnf(os);
            os << m_str;
            return;
        }
        assert(0);
    }

    inline void UnaryAst::to_ebnf(os_type& os) const
    {
        if (m_str == "optional")
        {
            os << '[';
            m_arg->to_ebnf(os);
            os << ']';
            return;
        }
        if (m_str == "repeated")
        {
            os << '{';
            m_arg->to_ebnf(os);
            os << '}';
            return;
        }
        if (m_str == "group")
        {
            os << '(';
            m_arg->to_ebnf(os);
            os << ')';
            return;
        }
        if (m_str == "+" || m_str == "*" || m_str == "?")
        {
            assert(0);
            m_arg->to_bnf(os);
            os << m_str;
            return;
        }
        assert(0);
    }

    inline void BinaryAst::to_dbg(os_type& os) const
    {
        os << "[BINARY " << m_str << ": ";
        m_left->to_dbg(os);
        os << ", ";
        m_right->to_dbg(os);
        os << "]";
    }

    inline void BinaryAst::to_bnf(os_type& os) const
    {
        if (m_str == "rule")
        {
            m_left->to_bnf(os);
            os << " ::= ";
            m_right->to_bnf(os);
            os << "\n";
            return;
        }
        if (m_str == "-")
        {
            m_left->to_bnf(os);
            os << " - ";
            m_right->to_bnf(os);
            return;
        }
        if (m_str == "*")
        {
            m_left->to_bnf(os);
            os << " * ";
            m_right->to_bnf(os);
            return;
        }
        assert(0);
    }

    inline void BinaryAst::to_ebnf(os_type& os) const
    {
        if (m_str == "rule")
        {
            m_left->to_ebnf(os);
            os << " = ";
            m_right->to_ebnf(os);
            os << ";\n";
            return;
        }
        if (m_str == "-")
        {
            m_left->to_ebnf(os);
            os << " - ";
            m_right->to_ebnf(os);
            return;
        }
        if (m_str == "*")
        {
            m_left->to_ebnf(os);
            os << " * ";
            m_right->to_ebnf(os);
            return;
        }
        assert(0);
    }

    inline BaseAst *SeqAst::clone() const
    {
        SeqAst *ast = new SeqAst(m_str);
        for (size_t i = 0; i < m_vec.size(); ++i)
        {
            ast->push_back(m_vec[i]->clone());
        }
        return ast;
    }

    inline BaseAst *SeqAst::sorted_clone() const
    {
        SeqAst *ast = new SeqAst(m_str);
        for (size_t i = 0; i < m_vec.size(); ++i)
        {
            BaseAst *cloned = m_vec[i]->sorted_clone();
            ast->push_back(cloned);
        }
        std::sort(ast->m_vec.begin(), ast->m_vec.end(), ast_less_than);
        return ast;
    }

    inline void SeqAst::to_dbg(os_type& os) const
    {
        os << "[SEQ " << m_str << ": ";
        if (m_vec.size())
        {
            m_vec[0]->to_dbg(os);
            for (size_t i = 1; i < m_vec.size(); ++i)
            {
                os << ", ";
                m_vec[i]->to_dbg(os);
            }
        }
        os << "]";
    }

    inline void SeqAst::to_bnf(os_type& os) const
    {
        if (m_str == "rules")
        {
            for (size_t i = 0; i < m_vec.size(); ++i)
            {
                m_vec[i]->to_bnf(os);
            }
            return;
        }
        if (m_str == "expr")
        {
            if (m_vec.size())
            {
                m_vec[0]->to_bnf(os);
                for (size_t i = 1; i < m_vec.size(); ++i)
                {
                    os << " | ";
                    m_vec[i]->to_bnf(os);
                }
            }
            return;
        }
        if (m_str == "terms")
        {
            if (m_vec.size())
            {
                m_vec[0]->to_bnf(os);
                for (size_t i = 1; i < m_vec.size(); ++i)
                {
                    os << " ";
                    m_vec[i]->to_bnf(os);
                }
            }
            return;
        }
        assert(0);
    }

    inline void SeqAst::to_ebnf(os_type& os) const
    {
        if (m_str == "rules")
        {
            for (size_t i = 0; i < m_vec.size(); ++i)
            {
                m_vec[i]->to_ebnf(os);
            }
            return;
        }
        if (m_str == "expr")
        {
            if (m_vec.size())
            {
                m_vec[0]->to_ebnf(os);
                for (size_t i = 1; i < m_vec.size(); ++i)
                {
                    os << " | ";
                    m_vec[i]->to_ebnf(os);
                }
            }
            return;
        }
        if (m_str == "terms")
        {
            if (m_vec.size())
            {
                m_vec[0]->to_ebnf(os);
                for (size_t i = 1; i < m_vec.size(); ++i)
                {
                    os << ", ";
                    m_vec[i]->to_ebnf(os);
                }
            }
            return;
        }
        assert(0);
    }
} // namespace bnf_ast

/////////////////////////////////////////////////////////////////////////

#endif  // ndef BNF_AST_HPP_
