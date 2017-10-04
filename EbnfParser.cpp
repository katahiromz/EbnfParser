// EbnfParser.cpp --- ISO EBNF parser
/////////////////////////////////////////////////////////////////////////

#include "EbnfParser.hpp"
#include <cstdio>       // for puts

int g_executed = 0;
int g_failed = 0;

struct TEST_ENTRY
{
    int entry_number;   // #
    int ret;            // return value
    const char *input;
};

enum TestReturn
{
    TR_SUCCESS = 0,
    TR_SCAN_FAIL,
    TR_PARSE_FAIL,
};

static const TEST_ENTRY g_test_entries[] =
{
#ifdef ISO_EBNF
    { 1, TR_SCAN_FAIL,      "list = '';" }, // empty string
    { 2, TR_SCAN_FAIL,      "list = \"\";" }, // empty string
    { 3, TR_SCAN_FAIL,     "underline_not_acceptable" },    // invalid identifier
#endif
    { 4, TR_SUCCESS,        "list = \"a\";" },
    { 5, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list list;" },  // comma needed
    { 6, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list, list" },  // semicolon needed
    { 7, TR_SUCCESS,        "list = \"a\"; arg = list | list, list;" },
    { 8, TR_PARSE_FAIL,     "list v = \"a\";" },    // invalid syntax
    { 9, TR_PARSE_FAIL,     "list = v \"a\";" },    // comma needed
    { 10, TR_PARSE_FAIL,     "'a' \"a\"" },  // invalid syntax
    { 11, TR_PARSE_FAIL,    "z = 'a' \"a\"" }, // comma needed
    { 12, TR_SUCCESS,       "z = 'a', \"a\";" },
    { 13, TR_SUCCESS,       "z = (a | b | c);" },
    { 14, TR_SUCCESS,       "z = [a , b, c];" },
    { 15, TR_SUCCESS,       "z = [a | b | c];" },
    { 16, TR_SUCCESS,       "z = [a | (b | c)];" },
    { 17, TR_PARSE_FAIL,    "z = [a | (b | c)]; a = test" },   // semicolon needed
    { 18, TR_SUCCESS,       "z = [a | (b | c)]; a = test;" },
    { 19, TR_PARSE_FAIL,    "'z' = a; a = test;" },    // invalid syntax
    { 20, TR_PARSE_FAIL,    "'z';" },    // invalid syntax
    { 21, TR_PARSE_FAIL,    "z;" },    // invalid syntax
    { 22, TR_PARSE_FAIL,    "z" },    // invalid syntax
    { 23, TR_SCAN_FAIL,     "@@" },    // invalid character
    { 24, TR_SCAN_FAIL,     "!" },    // invalid character
    { 25, TR_SCAN_FAIL,     "\"not-terminated" },    // invalid string
    { 26, TR_SCAN_FAIL,     "\'not-terminated" },    // invalid string
};

TestReturn just_do_it(const std::string& str)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    TestReturn ret = TR_SCAN_FAIL;
    os_type os;
    if (stream.scan_tokens())
    {
        ret = TR_PARSE_FAIL;
        stream.to_dbg(os);

        Parser parser(stream);
        if (parser.parse())
        {
            ret = TR_SUCCESS;
            BaseAst *ast = parser.ast();

            os << "\nto_dbg:\n";
            ast->to_dbg(os);
            os << "\n\nto_out:\n";
            ast->to_out(os);
        }
        else
        {
            parser.err_out(os);
        }
    }
    else
    {
        stream.err_out(os);
    }
#ifndef NDEBUG
    puts(os.str().c_str());
#endif
    return ret;
}

bool do_test_entry(const TEST_ENTRY *entry)
{
    int ret = just_do_it(entry->input);
    if (ret != entry->ret)
    {
        printf("#%d: FAILED: expected %d, got %d\n", entry->entry_number, entry->ret, ret);
        ++g_failed;
        ++g_executed;
        return false;
    }
    ++g_executed;
    return true;
}

int main(void)
{
    size_t count = sizeof(g_test_entries) / sizeof(g_test_entries[0]);
    for (size_t i = 0; i < count; ++i)
    {
        do_test_entry(&g_test_entries[i]);
    }

    printf("g_executed: %d, g_failed: %d\n", g_executed, g_failed);
    if (g_failed == 0)
        printf("SUCCESS!\n");

    assert(EbnfParser::BaseAst::alive_count() == 0);
    return 0;
}
