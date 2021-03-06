// bnf_ast.hpp --- BNF/EBNF notation AST
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#ifndef BNF_AST_HPP_
#define BNF_AST_HPP_    26  // Version 26

#include <string>           // for std::string
#include <vector>           // for std::vector
#include <sstream>          // for std::stringstream
#include <cassert>          // for assert macro
#include <algorithm>        // for std::sort

/////////////////////////////////////////////////////////////////////////

namespace bnf_ast
{
    typedef std::string         string_type;
    typedef std::stringstream   os_type;
    typedef std::vector<string_type>  names_type;

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

    struct BaseAst;
        struct IntegerAst;
        struct StringAst;
        struct BinaryAst;
        struct IdentAst;
        struct UnaryAst;
        struct SeqAst;
        struct SpecialAst;
        struct EmptyAst;

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

        virtual bool empty() const = 0;
        virtual void to_dbg(os_type& os) const = 0;
        virtual void to_bnf(os_type& os) const = 0;
        virtual void to_ebnf(os_type& os) const = 0;
        virtual BaseAst *clone() const = 0;
        virtual BaseAst *sorted_clone() const = 0;

        template <typename T, AstType atype>
        T *get_ast();
        template <typename T, AstType atype>
        const T *get_ast() const;

        IntegerAst *get_int_ast();
        StringAst *get_str_ast();
        BinaryAst *get_bin_ast();
        IdentAst *get_ident_ast();
        UnaryAst *get_unary_ast();
        SeqAst *get_seq_ast();
        SpecialAst *get_special_ast();

        const IntegerAst *get_int_ast() const;
        const StringAst *get_str_ast() const;
        const BinaryAst *get_bin_ast() const;
        const IdentAst *get_ident_ast() const;
        const UnaryAst *get_unary_ast() const;
        const SeqAst *get_seq_ast() const;
        const SpecialAst *get_special_ast() const;

        SeqAst *get_expr();
        SeqAst *get_terms();
        UnaryAst *get_group();
        UnaryAst *get_repeated();
        UnaryAst *get_optional();
        const SeqAst *get_expr() const;
        const SeqAst *get_terms() const;
        const UnaryAst *get_group() const;
        const UnaryAst *get_repeated() const;
        const UnaryAst *get_optional() const;
    private:
        BaseAst();
        BaseAst(const BaseAst&);
        BaseAst& operator=(const BaseAst&);
    };

    struct IdentAst : public BaseAst
    {
        // NOTE: Every '-' and ' ' will be converted to '_'.
        string_type     m_name;     // generic name

        IdentAst(const string_type& name);
        string_type bnf_name() const;
        string_type ebnf_name() const;

        virtual bool empty() const
        {
            return false;
        }
        virtual void to_dbg(os_type& os) const
        {
            os << "[IDENT: " << m_name << "]";
        }
        virtual void to_bnf(os_type& os) const
        {
            os << "<" << bnf_name() << ">";
        }
        virtual void to_ebnf(os_type& os) const
        {
            os << ebnf_name();
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
        virtual bool empty() const
        {
            return false;
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
        virtual bool empty() const
        {
            return m_str.empty();
        }
        size_t size() const
        {
            return m_str.size();
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
        virtual BaseAst *sorted_clone() const;
    };

    struct SpecialAst : public BaseAst
    {
        string_type m_str;

        SpecialAst(const string_type& str) : BaseAst(ATYPE_SPECIAL), m_str(str)
        {
        }
        virtual bool empty() const
        {
            return false;
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
        virtual bool empty() const
        {
            return false;
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
            if (m_arg)
            {
                return new UnaryAst(m_str, m_arg->sorted_clone());
            }
            return new UnaryAst(m_str);
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
        virtual bool empty() const
        {
            return false;
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
            BaseAst *left = m_left->sorted_clone();
            BaseAst *right = m_right->sorted_clone();
            return new BinaryAst(m_str, left, right);
        }
    };
    typedef std::vector<BinaryAst *> rules_vector;

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
            for (size_t i = 0; i < size(); ++i)
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
        virtual bool empty() const;
        void unique();
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
        virtual bool empty() const
        {
            return true;
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
    // AST functions

    bool ast_equal(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted = false);
    bool ast_less_than(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted = false);
    bool ast_greater_than(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted = false);

    inline bool ast_equal_sorted(const BaseAst *ast1, const BaseAst *ast2)
    {
        return ast_equal(ast1, ast2, true);
    }

    inline bool ast_less_than_sorted(const BaseAst *ast1, const BaseAst *ast2)
    {
        return ast_less_than(ast1, ast2, true);
    }

          rules_vector *ast_get_rules_vector(      BaseAst *rules);
    const rules_vector *ast_get_rules_vector(const BaseAst *rules);

    string_type ast_get_first_rule_name(const BaseAst *rules);
    string_type ast_get_rule_name(const BaseAst *rule);
    void ast_get_defined_rule_names(names_type& names, const BaseAst *rules);

          BaseAst *ast_get_rule_body(      BaseAst *rules, const string_type& rule_name);
    const BaseAst *ast_get_rule_body(const BaseAst *rules, const string_type& rule_name);

    bool ast_join_joinable_rules(BaseAst *rules);

    void name_increment(string_type& name);

    void ast_add_rule(BaseAst *rules, string_type& name, const BaseAst *rule_expr);

    /////////////////////////////////////////////////////////////////////////
    // AST function inlines

    inline bool ast_equal(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted)
    {
        assert(ast1);
        assert(ast2);

        if (ast1->m_atype != ast2->m_atype)
            return false;

        switch (ast1->m_atype)
        {
        case ATYPE_INTEGER:
            {
                const IntegerAst *i1 = ast1->get_int_ast();
                const IntegerAst *i2 = ast2->get_int_ast();
                return i1->m_integer == i2->m_integer;
            }
        case ATYPE_STRING:
            {
                const StringAst *s1 = ast1->get_str_ast();
                const StringAst *s2 = ast2->get_str_ast();
                return s1->m_str == s2->m_str;
            }
        case ATYPE_BINARY:
            {
                const BinaryAst *b1 = ast1->get_bin_ast();
                const BinaryAst *b2 = ast2->get_bin_ast();
                if (b1->m_str != b2->m_str)
                    return false;
                return ast_equal(b1->m_left, b2->m_left, already_sorted) &&
                       ast_equal(b1->m_right, b2->m_right, already_sorted);
            }
        case ATYPE_IDENT:
            {
                const IdentAst *i1 = ast1->get_ident_ast();
                const IdentAst *i2 = ast2->get_ident_ast();
                return i1->m_name == i2->m_name;
            }
        case ATYPE_UNARY:
            {
                const UnaryAst *u1 = ast1->get_unary_ast();
                const UnaryAst *u2 = ast1->get_unary_ast();
                if (u1->m_str != u2->m_str)
                    return false;
                if (!u1->m_arg != !u2->m_arg)
                    return false;
                return ast_equal(u1->m_arg, u2->m_arg, already_sorted);
            }
        case ATYPE_SEQ:
            {
                const SeqAst *s1 = ast1->get_seq_ast();
                const SeqAst *s2 = ast2->get_seq_ast();
                if (s1->m_str != s2->m_str)
                    return false;
                if (s1->size() != s2->size())
                    return false;
                if (already_sorted)
                {
                    for (size_t i = 0; i < s1->size(); ++i)
                    {
                        if (!ast_equal(s1->m_vec[i], s2->m_vec[i], true))
                        {
                            return false;
                        }
                    }
                    return true;
                }

                SeqAst *seq1 = s1->sorted_clone()->get_seq_ast();
                SeqAst *seq2 = s2->sorted_clone()->get_seq_ast();
                for (size_t i = 0; i < s1->size(); ++i)
                {
                    if (!ast_equal(seq1->m_vec[i], seq2->m_vec[i], true))
                    {
                        delete seq1;
                        delete seq2;
                        return false;
                    }
                }
                delete seq1;
                delete seq2;
                return true;
            }
        case ATYPE_SPECIAL:
            {
                const SpecialAst *spe1 = ast1->get_special_ast();
                const SpecialAst *spe2 = ast2->get_special_ast();
                return spe1->m_str == spe2->m_str;
            }
        case ATYPE_EMPTY:
            return true;
        }
    }

    inline bool ast_less_than(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted)
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
                const IntegerAst *i1 = ast1->get_int_ast();
                const IntegerAst *i2 = ast2->get_int_ast();
                return i1->m_integer < i2->m_integer;
            }
        case ATYPE_STRING:
            {
                const StringAst *s1 = ast1->get_str_ast();
                const StringAst *s2 = ast2->get_str_ast();
                return s1->m_str < s2->m_str;
            }
        case ATYPE_BINARY:
            {
                const BinaryAst *b1 = ast1->get_bin_ast();
                const BinaryAst *b2 = ast2->get_bin_ast();
                if (b1->m_str < b2->m_str)
                    return true;
                if (b1->m_str > b2->m_str)
                    return false;
                if (ast_less_than(b1->m_left, b2->m_left, already_sorted))
                    return true;
                if (!ast_equal(b1->m_left, b2->m_left, already_sorted))
                    return false;
                return ast_less_than(b1->m_right, b2->m_right, already_sorted);
            }
        case ATYPE_IDENT:
            {
                const IdentAst *i1 = ast1->get_ident_ast();
                const IdentAst *i2 = ast2->get_ident_ast();
                return i1->m_name < i2->m_name;
            }
        case ATYPE_UNARY:
            {
                const UnaryAst *u1 = ast1->get_unary_ast();
                const UnaryAst *u2 = ast2->get_unary_ast();
                if (u1->m_str < u2->m_str)
                    return true;
                if (u1->m_str > u2->m_str)
                    return false;
                if (!!u1->m_arg < !!u2->m_arg)
                    return true;
                if (!!u1->m_arg > !!u2->m_arg)
                    return false;
                return ast_less_than(u1->m_arg, u2->m_arg, already_sorted);
            }
        case ATYPE_SEQ:
            {
                const SeqAst *s1 = ast1->get_seq_ast();
                const SeqAst *s2 = ast2->get_seq_ast();
                if (s1->m_str < s2->m_str)
                    return true;
                if (s1->m_str > s2->m_str)
                    return false;

                size_t count;
                if (s1->size() < s2->size())
                    count = s1->size();
                else
                    count = s2->size();

                if (already_sorted)
                {
                    for (size_t i = 0; i < count; ++i)
                    {
                        if (ast_equal(s1->m_vec[i], s2->m_vec[i], true))
                        {
                            continue;
                        }
                        return ast_less_than(s1->m_vec[i], s2->m_vec[i], true);
                    }
                    return s1->size() < s2->size();
                }

                SeqAst *seq1 = s1->sorted_clone()->get_seq_ast();
                SeqAst *seq2 = s2->sorted_clone()->get_seq_ast();

                for (size_t i = 0; i < count; ++i)
                {
                    if (ast_equal(seq1->m_vec[i], seq2->m_vec[i], true))
                    {
                        continue;
                    }

                    bool ret = ast_less_than(seq1->m_vec[i], seq2->m_vec[i], true);
                    delete seq1;
                    delete seq2;
                    return ret;
                }

                bool less_than = seq1->size() < seq2->size();
                delete seq1;
                delete seq2;
                return less_than;
            }
        case ATYPE_SPECIAL:
            {
                const SpecialAst *spe1 = ast1->get_special_ast();
                const SpecialAst *spe2 = ast2->get_special_ast();
                return spe1->m_str < spe2->m_str;
            }
        case ATYPE_EMPTY:
            return false;
        }
    }

    inline bool ast_greater_than(const BaseAst *ast1, const BaseAst *ast2, bool already_sorted)
    {
        BaseAst *cloned1 = ast1->sorted_clone();
        BaseAst *cloned2 = ast2->sorted_clone();
        bool ret = !ast_equal(ast1, ast2, true) && !ast_less_than(ast1, ast2, true);
        delete cloned1;
        delete cloned2;
        return ret;
    }

    inline const rules_vector *ast_get_rules_vector(const BaseAst *rules)
    {
        assert(rules->m_atype == ATYPE_SEQ);
        const SeqAst *seq = rules->get_seq_ast();
        assert(seq && seq->m_str == "rules");
        return reinterpret_cast<const rules_vector *>(&seq->m_vec);
    }

    inline rules_vector *ast_get_rules_vector(BaseAst *rules)
    {
        assert(rules->m_atype == ATYPE_SEQ);
        SeqAst *seq = rules->get_seq_ast();
        assert(seq && seq->m_str == "rules");
        return reinterpret_cast<rules_vector *>(&seq->m_vec);
    }

    inline string_type ast_get_first_rule_name(const BaseAst *rules)
    {
        const rules_vector *pvec = ast_get_rules_vector(rules);
        assert(pvec);
        if (pvec->size() == 0)
            return string_type();
        return ast_get_rule_name((*pvec)[0]);
    }

    inline string_type ast_get_rule_name(const BaseAst *rule)
    {
        const BinaryAst *bin = rule->get_bin_ast();
        assert(bin && bin->m_str == "rule");

        const IdentAst *ident = bin->m_left->get_ident_ast();
        assert(ident);
        return ident->m_name;
    }

    inline void ast_get_defined_rule_names(names_type& names, const BaseAst *rules)
    {
        names.clear();
        const rules_vector *pvec = ast_get_rules_vector(rules);
        for (size_t i = 0; i < (*pvec).size(); ++i)
        {
            BinaryAst *bin = (*pvec)[i];
            names.push_back(ast_get_rule_name(bin));
        }
    }

    inline const BaseAst *
    ast_get_rule_body(const BaseAst *rules, const string_type& rule_name)
    {
        const rules_vector *pvec = ast_get_rules_vector(rules);
        for (size_t i = 0; i < (*pvec).size(); ++i)
        {
            const BinaryAst *bin = (*pvec)[i];
            if (ast_get_rule_name(bin) == rule_name)
                return bin->m_right;
        }
        return NULL;
    }

    inline BaseAst *ast_get_rule_body(BaseAst *rules, const string_type& rule_name)
    {
        rules_vector *pvec = ast_get_rules_vector(rules);
        for (size_t i = 0; i < (*pvec).size(); ++i)
        {
            BinaryAst *bin = (*pvec)[i];
            if (ast_get_rule_name(bin) == rule_name)
                return bin->m_right;
        }
        return NULL;
    }

    inline bool ast_join_joinable_rules(BaseAst *rules)
    {
        bool ret = false;
        assert(rules->m_atype == ATYPE_SEQ);
        rules_vector *pvec = ast_get_rules_vector(rules);
        assert(pvec);
        if (pvec->size() == 0)
            return false;

        for (size_t i = 0; i < (*pvec).size() - 1; ++i)
        {
            BinaryAst *bin1 = (*pvec)[i];
            assert(bin1->m_atype == ATYPE_BINARY);
            string_type name1 = ast_get_rule_name(bin1);

            for (size_t k = i + 1; k < (*pvec).size(); ++k)
            {
                BinaryAst *bin2 = (*pvec)[k];
                assert(bin2->m_atype == ATYPE_BINARY);
                string_type name2 = ast_get_rule_name(bin2);

                if (name1 != name2)
                    continue;

                SeqAst *seq1 = bin1->m_right->get_seq_ast();
                SeqAst *seq2 = bin2->m_right->get_seq_ast();

                assert(seq1 && seq1->m_str == "expr");
                assert(seq2 && seq2->m_str == "expr");

                seq1->m_vec.insert(seq1->m_vec.end(), seq2->m_vec.begin(), seq2->m_vec.end());
                seq2->m_vec.clear();

                delete bin2;
                (*pvec)[k] = NULL;
                pvec->erase(pvec->begin() + k);
                --k;
                ret = true;
            }
        }

        return ret;
    }

    inline void name_increment(string_type& name)
    {
        size_t i = name.find_last_not_of("0123456789");
        if (i == string_type::npos || i == name.size() - 1)
        {
            name += "_02";
            return;
        }
        ++i;
        string_type str = name.substr(0, i);
        int n = atoi(name.substr(i).c_str());
        name = str;
        char buf[32];
        std::sprintf(buf, "%02u", n + 1);
        name += buf;
    }

    inline void ast_add_rule(BaseAst *rules, string_type& name, const BaseAst *rule_expr)
    {
        assert(!ast_join_joinable_rules(rules));

        rules_vector *pvec = ast_get_rules_vector(rules);
        assert(pvec);

        assert(rule_expr->get_expr());
        for (size_t i = 0; i < pvec->size(); ++i)
        {
            BinaryAst *bin = (*pvec)[i];
            if (ast_equal(bin->m_right, rule_expr))
            {
                IdentAst *ident = bin->m_left->get_ident_ast();
                name = ident->m_name;
                return;
            }
        }

        for (;;)
        {
            bool flag = false;
            for (size_t i = 0; i < pvec->size(); ++i)
            {
                BinaryAst *bin = (*pvec)[i];
                IdentAst *ident = bin->m_left->get_ident_ast();
                if (name == ident->m_name)
                {
                    flag = true;
                    break;
                }
            }
            if (!flag)
                break;

            name_increment(name);
        }

        IdentAst *ident = new IdentAst(name);
        SeqAst *expr = rule_expr->sorted_clone()->get_expr();
        assert(expr);
        BinaryAst *bin = new BinaryAst("rule", ident, expr);
        pvec->push_back(bin);
    }

    /////////////////////////////////////////////////////////////////////////
    // AST method inlines

    inline IdentAst::IdentAst(const string_type& name)
        : BaseAst(ATYPE_IDENT), m_name(name)
    {
        for (size_t i = 0; i < m_name.size(); ++i)
        {
            if (m_name[i] == '-' || m_name[i] == ' ')
            {
                m_name[i] = '_';
            }
        }
    }
    inline string_type IdentAst::bnf_name() const
    {
        string_type ret = m_name;
        for (size_t i = 0; i < ret.size(); ++i)
        {
            if (ret[i] == '_' || ret[i] == ' ')
                ret[i] = '-';
        }
        return ret;
    }
    inline string_type IdentAst::ebnf_name() const
    {
        string_type ret = m_name;
        for (size_t i = 0; i < ret.size(); ++i)
        {
            if (ret[i] == '_' || ret[i] == ' ')
                ret[i] = '-';
        }
        return ret;
    }

    template <typename T, AstType atype>
    inline T *BaseAst::get_ast()
    {
        if (m_atype == atype)
            return reinterpret_cast<T *>(this);
        return NULL;
    }
    template <typename T, AstType atype>
    inline const T *BaseAst::get_ast() const
    {
        if (m_atype == atype)
            return reinterpret_cast<const T *>(this);
        return NULL;
    }

    inline IntegerAst *BaseAst::get_int_ast()
    {
        return get_ast<IntegerAst, ATYPE_INTEGER>();
    }
    inline StringAst *BaseAst::get_str_ast()
    {
        return get_ast<StringAst, ATYPE_STRING>();
    }
    inline BinaryAst *BaseAst::get_bin_ast()
    {
        return get_ast<BinaryAst, ATYPE_BINARY>();
    }
    inline IdentAst *BaseAst::get_ident_ast()
    {
        return get_ast<IdentAst, ATYPE_IDENT>();
    }
    inline UnaryAst *BaseAst::get_unary_ast()
    {
        return get_ast<UnaryAst, ATYPE_UNARY>();
    }
    inline SeqAst *BaseAst::get_seq_ast()
    {
        return get_ast<SeqAst, ATYPE_SEQ>();
    }
    inline SpecialAst *BaseAst::get_special_ast()
    {
        return get_ast<SpecialAst, ATYPE_SPECIAL>();
    }

    inline const IntegerAst *BaseAst::get_int_ast() const
    {
        return get_ast<IntegerAst, ATYPE_INTEGER>();
    }
    inline const StringAst *BaseAst::get_str_ast() const
    {
        return get_ast<StringAst, ATYPE_STRING>();
    }
    inline const BinaryAst *BaseAst::get_bin_ast() const
    {
        return get_ast<BinaryAst, ATYPE_BINARY>();
    }
    inline const IdentAst *BaseAst::get_ident_ast() const
    {
        return get_ast<IdentAst, ATYPE_IDENT>();
    }
    inline const UnaryAst *BaseAst::get_unary_ast() const
    {
        return get_ast<UnaryAst, ATYPE_UNARY>();
    }
    inline const SeqAst *BaseAst::get_seq_ast() const
    {
        return get_ast<SeqAst, ATYPE_SEQ>();
    }
    inline const SpecialAst *BaseAst::get_special_ast() const
    {
        return get_ast<SpecialAst, ATYPE_SPECIAL>();
    }

    inline SeqAst *BaseAst::get_expr()
    {
        SeqAst *ret = get_seq_ast();
        if (ret && ret->m_str == "expr")
            return ret;
        return NULL;
    }
    inline SeqAst *BaseAst::get_terms()
    {
        SeqAst *ret = get_seq_ast();
        if (ret && ret->m_str == "terms")
            return ret;
        return NULL;
    }
    inline UnaryAst *BaseAst::get_group()
    {
        UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "group")
            return ret;
        return NULL;
    }
    inline UnaryAst *BaseAst::get_repeated()
    {
        UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "repeated")
            return ret;
        return NULL;
    }
    inline UnaryAst *BaseAst::get_optional()
    {
        UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "optional")
            return ret;
        return NULL;
    }

    inline const SeqAst *BaseAst::get_expr() const
    {
        const SeqAst *ret = get_seq_ast();
        if (ret && ret->m_str == "expr")
            return ret;
        return NULL;
    }
    inline const SeqAst *BaseAst::get_terms() const
    {
        const SeqAst *ret = get_seq_ast();
        if (ret && ret->m_str == "terms")
            return ret;
        return NULL;
    }
    inline const UnaryAst *BaseAst::get_group() const
    {
        const UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "group")
            return ret;
        return NULL;
    }
    inline const UnaryAst *BaseAst::get_repeated() const
    {
        const UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "repeated")
            return ret;
        return NULL;
    }
    inline const UnaryAst *BaseAst::get_optional() const
    {
        const UnaryAst *ret = get_unary_ast();
        if (ret && ret->m_str == "optional")
            return ret;
        return NULL;
    }

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
        if (m_str == "optional" || m_str == "?")
        {
            os << '[';
            m_arg->to_ebnf(os);
            os << ']';
            return;
        }
        if (m_str == "repeated" || m_str == "*")
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
        if (m_str == "+")
        {
            os << '(';
            m_arg->to_ebnf(os);
            os << "), {";
            m_arg->to_ebnf(os);
            os << '}';
            return;
        }
        assert(0);
    }

    inline BaseAst *StringAst::sorted_clone() const
    {
        if (m_str.empty())
            return new EmptyAst();

        return new StringAst(m_str);
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
            const IntegerAst *integer = m_left->get_int_ast();
            if (const int n = integer->m_integer)
            {
                m_right->to_bnf(os);
                for (int i = 1; i < n; ++i)
                {
                    os << " ";
                    m_right->to_bnf(os);
                }
            }
            else
            {
                os << "\"\"";
            }
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
            assert(m_left->get_int_ast());
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
        for (size_t i = 0; i < size(); ++i)
        {
            ast->push_back(m_vec[i]->clone());
        }
        return ast;
    }

    inline BaseAst *SeqAst::sorted_clone() const
    {
        SeqAst *ast = new SeqAst(m_str);
        if (m_str == "terms")
        {
            for (size_t i = 0; i < size(); ++i)
            {
                if (m_vec[i]->empty())
                    continue;

                if (const UnaryAst *unary = m_vec[i]->get_group())
                {
                    const SeqAst *expr = unary->m_arg->get_expr();
                    if (expr->size() == 1)
                    {
                        const SeqAst *terms = expr->m_vec[0]->get_terms();
                        if (terms->empty())
                            continue;

                        SeqAst *cloned_terms = terms->sorted_clone()->get_terms();
                        ast->m_vec.insert(ast->m_vec.end(),
                                          cloned_terms->m_vec.begin(),
                                          cloned_terms->m_vec.end());
                        cloned_terms->m_vec.clear();
                        delete cloned_terms;
                        continue;
                    }
                }

                BaseAst *cloned = m_vec[i]->sorted_clone();
                ast->push_back(cloned);
            }
        }
        else if (m_str == "expr")
        {
            for (size_t i = 0; i < size(); ++i)
            {
                const SeqAst *terms = m_vec[i]->get_terms();
                if (terms && terms->size() == 1)
                {
                    if (const UnaryAst *unary = terms->m_vec[0]->get_group())
                    {
                        const SeqAst *expr = unary->m_arg->get_expr();
                        SeqAst *cloned_expr = expr->sorted_clone()->get_expr();
                        ast->m_vec.insert(ast->m_vec.end(),
                                          cloned_expr->m_vec.begin(),
                                          cloned_expr->m_vec.end());
                        cloned_expr->m_vec.clear();
                        delete cloned_expr;
                        continue;
                    }
                }

                BaseAst *cloned = m_vec[i]->sorted_clone();
                ast->push_back(cloned);
            }
            std::sort(ast->m_vec.begin(), ast->m_vec.end(), ast_less_than_sorted);
            ast->unique();
        }
        else if (m_str == "rules")
        {
            for (size_t i = 0; i < size(); ++i)
            {
                BaseAst *cloned = m_vec[i]->sorted_clone();
                ast->push_back(cloned);
            }
        }
        return ast;
    }

    inline bool SeqAst::empty() const
    {
        if (m_str == "rules")
            return false;

        for (size_t i = 0; i < size(); ++i)
        {
            if (!m_vec[i]->empty())
                return false;
        }
        return true;
    }

    inline void SeqAst::unique()
    {
        for (size_t i = 0; i < size() - 1; )
        {
            if (ast_equal(m_vec[i], m_vec[i + 1], true))
            {
                delete m_vec[i];
                m_vec.erase(m_vec.begin() + i);
            }
            else
            {
                ++i;
            }
        }
    }

    inline void SeqAst::to_dbg(os_type& os) const
    {
        os << "[SEQ " << m_str << ": ";
        if (size())
        {
            m_vec[0]->to_dbg(os);
            for (size_t i = 1; i < size(); ++i)
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
            for (size_t i = 0; i < size(); ++i)
            {
                m_vec[i]->to_bnf(os);
            }
            return;
        }
        if (m_str == "expr")
        {
            if (empty())
            {
                os << "\"\"";
            }
            else
            {
                m_vec[0]->to_bnf(os);
                for (size_t i = 1; i < size(); ++i)
                {
                    os << " | ";
                    m_vec[i]->to_bnf(os);
                }
            }
            return;
        }
        if (m_str == "terms")
        {
            if (empty())
            {
                os << "\"\"";
            }
            else
            {
                m_vec[0]->to_bnf(os);
                for (size_t i = 1; i < size(); ++i)
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
            for (size_t i = 0; i < size(); ++i)
            {
                m_vec[i]->to_ebnf(os);
            }
            return;
        }
        if (m_str == "expr")
        {
            if (!empty())
            {
                m_vec[0]->to_ebnf(os);
                for (size_t i = 1; i < size(); ++i)
                {
                    os << " | ";
                    m_vec[i]->to_ebnf(os);
                }
            }
            return;
        }
        if (m_str == "terms")
        {
            if (!empty())
            {
                m_vec[0]->to_ebnf(os);
                for (size_t i = 1; i < size(); ++i)
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
