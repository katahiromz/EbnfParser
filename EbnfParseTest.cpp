// EbnfParseTest.cpp --- tests for ISO EBNF parser
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#include "EBNF.hpp"
#include <cstdio>       // for std::puts

static int g_num_executions = 0;       // number of test executions
static int g_num_failures = 0;         // number of test failures

enum PARSE_TEST_RETURN
{
    TR_SUCCESS = 0,
    TR_SCAN_FAIL,
    TR_PARSE_FAIL,
};

struct PARSE_TEST_ENTRY
{
    int entry_number;       // #
    size_t num_rules;       // number of rules
    PARSE_TEST_RETURN ret;  // return value
    const char *input;
};

static const PARSE_TEST_ENTRY g_test_entries[] =
{
#ifdef ISO_EBNF
    { 1, 0, TR_SCAN_FAIL,      "list = '';" }, // empty string
    { 2, 0, TR_SCAN_FAIL,      "list = \"\";" }, // empty string
#endif
    { 3, 0, TR_SCAN_FAIL,      "underline_not_allowed" },    // invalid identifier
    { 4, 1, TR_SUCCESS,        "list = \"a\";" },
    { 5, 0, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list, list" },  // semicolon needed
    { 6, 2, TR_SUCCESS,        "list = \"a\"; arg = list | list, list;" },
    { 7, 0, TR_PARSE_FAIL,     "list = v \"a\";" },    // comma needed
    { 8, 0, TR_PARSE_FAIL,    "'a' \"a\"" },  // invalid syntax
    { 9, 0, TR_PARSE_FAIL,    "z = 'a' \"a\"" }, // comma needed
    { 10, 1, TR_SUCCESS,       "z = 'a', \"a\";" },
    { 11, 1, TR_SUCCESS,       "z = (a | b | c);" },
    { 12, 1, TR_SUCCESS,       "z = [a , b, c];" },
    { 13, 1, TR_SUCCESS,       "z = [a | b | c];" },
    { 14, 1, TR_SUCCESS,       "z = [a | (b | c)];" },
    { 15, 0, TR_PARSE_FAIL,    "z = [a | (b | c)]; a = test" },   // semicolon needed
    { 16, 2, TR_SUCCESS,       "z = [a | (b | c)]; a = test;" },
    { 17, 0, TR_PARSE_FAIL,    "'z' = a; a = test;" },    // invalid syntax
    { 18, 0, TR_PARSE_FAIL,    "'z';" },    // invalid syntax
    { 19, 0, TR_PARSE_FAIL,    "z;" },    // invalid syntax
    { 20, 0, TR_PARSE_FAIL,    "z" },    // invalid syntax
    { 21, 0, TR_SCAN_FAIL,     "\"not terminated" },    // invalid string
    { 22, 0, TR_SCAN_FAIL,     "\'not terminated" },    // invalid string
    { 23, 0, TR_SCAN_FAIL,     "?not terminated" },    // invalid special
    { 24, 0, TR_SCAN_FAIL,     "(*not terminated" },    // invalid comment // *)
    { 25, 1, TR_SUCCESS,       "xx = \"A\" - xx;" },
    { 26, 1, TR_SUCCESS,       "line = 5 * \" \", (character - (\" \" | \"0\")), 66 * [character];" },
    { 27, 1, TR_SUCCESS,
        "line = character - \"C\", 4 * character, character - (\" \" | \"0\"), 66 * [character];" },
    { 28, 7, TR_SUCCESS,
        "aa = \"A\";\n"
        "bb = 3 * aa, \"B\";\n"
        "cc = 3 * [aa], \"C\";\n"
        "dd = {aa}, \"D\";\n"
        "ee = aa, {aa}, \"E\";\n"
        "ff = 3 * aa, 3 * [aa], \"F\";\n"
        "gg = 3 * {aa}, \"D\";\n" },
    { 19, 1, TR_SUCCESS,
        "letter = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | "
        "'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | "
        "'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z';\n" },
    { 20, 1, TR_SUCCESS, "vowel = 'A' | 'E' | 'I' | 'O' | 'U';" },
    { 21, 1, TR_SUCCESS, "ee = {'A'} - , 'E';" },
    { 22, 0, TR_SCAN_FAIL, "." },
    { 23, 0, TR_SCAN_FAIL, ":" },
    { 24, 0, TR_SCAN_FAIL, "!" },
    { 25, 0, TR_SCAN_FAIL, "+" },
    { 26, 0, TR_SCAN_FAIL, "%" },
    { 27, 0, TR_SCAN_FAIL, "@" },
    { 28, 0, TR_SCAN_FAIL, "&" },
    { 29, 0, TR_SCAN_FAIL, "#" },
    { 30, 0, TR_SCAN_FAIL, "$" },
    { 31, 0, TR_SCAN_FAIL, "<" },
    { 32, 0, TR_SCAN_FAIL, ">" },
    { 33, 0, TR_SCAN_FAIL, "/" },
    { 34, 0, TR_SCAN_FAIL, "\\" },
    { 35, 0, TR_SCAN_FAIL, "^" },
    { 36, 0, TR_SCAN_FAIL, "`" },
    { 37, 0, TR_SCAN_FAIL, "~" },
    { 38, 1, TR_SUCCESS, "(* this is a test of comments *) test = test, 'a'; (* comment *)" },
    { 39, 1, TR_SUCCESS,
        "other = ' ' | ':' | '+' | '_' | '%' | '@' | '&' | '#' | '$' | "
        "'<' | '>' | '\\' | '^' | '`' | '~';" },
    { 40, 1, TR_SUCCESS, "special = ? ISO 6429 character Horizontal Tabulation ?;" },
    { 41, 1, TR_SUCCESS,
        "newline = {? ISO 6429 character Carriage Return ?}, "
        "? ISO 6429 character Line Feed ?, {? ISO 6429 character Carriage Return ?};" },
    { 42, 0, TR_PARSE_FAIL, "test = 'test';;" },   // double semicolon
    { 43, 1, TR_SUCCESS,
        "gap free symbol = terminal character - (first quote symbol | second quote symbol) | terminal string;" },
    { 44, 1, TR_SUCCESS, "syntax = syntax rule, {syntax rule};" },
    { 45, 2, TR_SUCCESS,
        "syntax = syntax rule, {syntax rule};\r\n"
        "syntax rule = meta identifier, '=', definitions list, ';';" },
    { 46, 1, TR_SUCCESS,
        "definitions list = single definition, {definition separator symbol, single definition};" },
    { 47, 2, TR_SUCCESS,
        "(*single definition *) single definition = syntactic term, {concatenate symbol, syntactic term};\n"
        "concatenate symbol = ',';" },
    { 48, 1, TR_SUCCESS,
        "comment = '(*', {comment symbol}, '*)' (* A comment is allowed anywhere "
        "outside a <terminal string>, <meta identifier>, <integer> or <special sequence> *);" },
    { 49, 1, TR_SUCCESS, "empty = ;" },
    { 50, 1, TR_SUCCESS, "text = character, { character } | ;" },
    { 51, 1, TR_SUCCESS, "text = | character, { character };" },
    { 52, 1, TR_SUCCESS, "text = { character | };" },
};

static PARSE_TEST_RETURN
just_do_it(const std::string& str, size_t& num_rules)
{
    using namespace EBNF;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    PARSE_TEST_RETURN ret = TR_SCAN_FAIL;
    os_type os;
    os << "input: " << str << std::endl;
    num_rules = 0;
    if (stream.scan())
    {
        stream.fixup();

        ret = TR_PARSE_FAIL;
        stream.to_dbg(os);

        Parser parser(stream);
        if (parser.parse())
        {
            ret = TR_SUCCESS;
            BaseAst *ast = parser.ast();

            os << "\nto_dbg:\n";
            ast->to_dbg(os);
            os << "\n\nto_ebnf:\n";
            ast->to_ebnf(os);

            if (ast->m_atype == ATYPE_SEQ)
            {
                SeqAst *seq = static_cast<SeqAst *>(ast);
                num_rules = seq->size();
            }
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

static bool do_test_entry(const PARSE_TEST_ENTRY *entry)
{
    bool failed = false;
    size_t num_rules = 0;
    PARSE_TEST_RETURN ret = just_do_it(entry->input, num_rules);
    if (ret != entry->ret)
    {
        printf("#%d: FAILED: ret expected %d, got %d\n", entry->entry_number, entry->ret, ret);
        ++g_num_failures;
        failed = true;
    }
    ++g_num_executions;
    if (num_rules != entry->num_rules)
    {
        printf("#%d: FAILED: num_rules expected %u, got %u\n",
               entry->entry_number, (int)entry->num_rules, (int)num_rules);
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
