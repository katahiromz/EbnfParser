// EbnfJoinTest.cpp --- joining tests for ISO EBNF notation
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#include "EBNF.hpp"
#include <cstdio>       // for std::puts

static int g_num_executions = 0;       // number of test executions
static int g_num_failures = 0;         // number of test failures

enum COMPARE_TEST_RETURN
{
    TR_EQUAL = 0,
    TR_LESS_THAN = -1,
    TR_GREATER_THAN = 1,
    TR_OTHER_ERROR = 2
};

struct COMPARE_TEST_ENTRY
{
    int entry_number;   // #
    COMPARE_TEST_RETURN ret;
    const char *input1;
    const char *input2;
};

static const COMPARE_TEST_ENTRY g_test_entries[] =
{
    { 1, TR_EQUAL, "a = a; a = b;", "a = a | b;" },
    { 2, TR_EQUAL, "a = 'a'; b = 'b'; a = 'c';", "a = 'a' | 'c'; b = 'b';" },
    { 3, TR_EQUAL, "a = 'a' | 'b'; b = 'b'; a = 'c';", "a = 'a' | 'b' | 'c'; b = 'b';" },
};

static EBNF::SeqAst *do_parse(const std::string& str)
{
    using namespace EBNF;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    if (stream.scan())
    {
        stream.fixup();

        Parser parser(stream);
        if (parser.parse())
        {
            BaseAst *ast = parser.detach();
            if (ast && ast->m_atype == ATYPE_SEQ)
            {
                SeqAst *seq = static_cast<SeqAst *>(ast);
                return seq;
            }
            delete ast;
        }
    }
    return NULL;
}

static COMPARE_TEST_RETURN just_do_it(const COMPARE_TEST_ENTRY *entry)
{
    using namespace EBNF;

    SeqAst *seq1 = do_parse(entry->input1);
    SeqAst *seq2 = do_parse(entry->input2);

    if (seq1 == NULL || seq2 == NULL)
    {
        delete seq1;
        delete seq2;
        return TR_OTHER_ERROR;
    }

#ifndef NDEBUG
    os_type os;
    os << "seq1: ";
    seq1->to_dbg(os);
    os << "\n";
    os << "seq2: ";
    seq2->to_dbg(os);
    os << "\n";
    BaseAst *s1 = seq1->sorted_clone();
    BaseAst *s2 = seq2->sorted_clone();
    os << "s1: ";
    s1->to_dbg(os);
    os << "\n";
    os << "s2: ";
    s2->to_dbg(os);
    os << "\n";
    delete s1;
    delete s2;
    puts(os.str().c_str());
#endif

    ast_join_joinable_rules(seq1);
    ast_join_joinable_rules(seq2);

    COMPARE_TEST_RETURN ret;
    if (ast_equal(seq1, seq2))
        ret = TR_EQUAL;
    else if (ast_less_than(seq1, seq2))
        ret = TR_LESS_THAN;
    else
        ret = TR_GREATER_THAN;

    delete seq1;
    delete seq2;
    return ret;
}

static bool do_test_entry(const COMPARE_TEST_ENTRY *entry)
{
    bool failed = false;
    COMPARE_TEST_RETURN ret = just_do_it(entry);

    if (ret != entry->ret)
    {
        printf("#%d: FAILED: ret expected %d, got %d\n", entry->entry_number, entry->ret, ret);
        ++g_num_failures;
        failed = true;
    }

    ++g_num_executions;
    return !failed;
}

int main(void)
{
    size_t count = sizeof(g_test_entries) / sizeof(g_test_entries[0]);
    for (size_t i = 0; i < count; ++i)
    {
        do_test_entry(&g_test_entries[i]);
    }

    printf("executions %d, failures %d\n", g_num_executions, g_num_failures);
    if (g_num_failures == 0)
        printf("SUCCESS!\n");

    assert(EBNF::BaseAst::alive_count() == 0);
    return g_num_failures;
}
