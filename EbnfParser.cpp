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

static const TEST_ENTRY g_test_entries[] =
{
    { 1, 1, "list = '';" }, // empty string
    { 2, 1, "list = \"\";" }, // empty string
    { 3, 0, "list = \"a\";" },
    { 4, 2, "list = \"a\"; arg = list | list list;" },  // comma needed
    { 5, 2, "list = \"a\"; arg = list | list, list" },  // semicolon needed
    { 6, 0, "list = \"a\"; arg = list | list, list;" },
    { 7, 2, "list v = \"a\";" },    // invalid syntax
    { 8, 2, "list = v \"a\";" },    // comma needed
    { 9, 2, "'a' \"a\"" },  // invalid syntax
    { 10, 2, "z = 'a' \"a\"" }, // comma needed
    { 11, 0, "z = 'a', \"a\";" },
    { 12, 0, "z = (a | b | c);" },
    { 13, 0, "z = [a , b, c];" },
    { 14, 0, "z = [a | b | c];" },
    { 15, 0, "z = [a | (b | c)];" },
    { 16, 2, "z = [a | (b | c)]; a = test" },   // semicolon needed
    { 17, 0, "z = [a | (b | c)]; a = test;" },
    { 18, 2, "'z' = a; a = test;" },    // invalid syntax
    { 19, 2, "'z';" },    // invalid syntax
    { 20, 2, "z;" },    // invalid syntax
    { 21, 2, "z" },    // invalid syntax
    { 22, 1, "@@" },    // invalid character
    { 23, 1, "!" },    // invalid character
};

// 0:success, 1:scanner failure, 2:parser failure
int just_do_it(const std::string& str)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    int ret = 1;
    os_type os;
    if (stream.scan_tokens())
    {
        ret = 2;
#ifndef NDEBUG
        stream.to_dbg(os);
#endif

        Parser parser(stream);
        if (parser.parse())
        {
            ret = 0;
            BaseAst *ast = parser.ast();

#ifndef NDEBUG
            os << "\nto_dbg:\n";
            ast->to_dbg(os);
#endif
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
    puts(os.str().c_str());
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

    assert(EbnfParser::BaseAst::alive_count() == 0);
    return 0;
}
