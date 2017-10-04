// EbnfParser.cpp --- ISO EBNF parser
// See ReadMe.txt and License.txt.
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

enum TEST_RETURN
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
    { 3, TR_SCAN_FAIL,      "underline_not_allowed" },    // invalid identifier
#endif
    { 4, TR_SUCCESS,        "list = \"a\";" },
    { 5, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list list;" },  // comma needed
    { 6, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list, list" },  // semicolon needed
    { 7, TR_SUCCESS,        "list = \"a\"; arg = list | list, list;" },
    { 8, TR_PARSE_FAIL,     "list v = \"a\";" },    // invalid syntax
    { 9, TR_PARSE_FAIL,     "list = v \"a\";" },    // comma needed
    { 10, TR_PARSE_FAIL,    "'a' \"a\"" },  // invalid syntax
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
    { 23, TR_SCAN_FAIL,     "\"not-terminated" },    // invalid string
    { 24, TR_SCAN_FAIL,     "\'not-terminated" },    // invalid string
    { 25, TR_SCAN_FAIL,     "?not-terminated" },    // invalid special
    { 26, TR_SCAN_FAIL,     "(*not-terminated" },    // invalid comment // *)
    { 27, TR_SUCCESS,       "xx = \"A\" - xx;" },
    { 28, TR_SUCCESS,       "line = 5 * \" \", (character - (\" \" | \"0\")), 66 * [character];" },
    { 29, TR_SUCCESS,       "line = character - \"C\", 4 * character, character - (\" \" | \"0\"), 66 * [character];" },
    { 30, TR_SUCCESS,
        "aa = \"A\";\n"
        "bb = 3 * aa, \"B\";\n"
        "cc = 3 * [aa], \"C\";\n"
        "dd = {aa}, \"D\";\n"
        "ee = aa, {aa}, \"E\";\n"
        "ff = 3 * aa, 3 * [aa], \"F\";\n"
        "gg = 3 * {aa}, \"D\";\n" },
    { 31, TR_SUCCESS,
        "letter = 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', "
        "'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', "
        "'T', 'U', 'V', 'W', 'X', 'Y', 'Z';\n" },
    { 32, TR_SUCCESS, "vowel = 'A' | 'E' | 'I' | 'O' | 'U';" },
    { 33, TR_SUCCESS, "ee = {'A'}-, 'E';" },
    { 34, TR_SCAN_FAIL, "." },
    { 35, TR_SCAN_FAIL, ":" },
    { 36, TR_SCAN_FAIL, "!" },
    { 37, TR_SCAN_FAIL, "+" },
    { 38, TR_SCAN_FAIL, "%" },
    { 39, TR_SCAN_FAIL, "@" },
    { 40, TR_SCAN_FAIL, "&" },
    { 41, TR_SCAN_FAIL, "#" },
    { 42, TR_SCAN_FAIL, "$" },
    { 43, TR_SCAN_FAIL, "<" },
    { 44, TR_SCAN_FAIL, ">" },
    { 45, TR_SCAN_FAIL, "/" },
    { 46, TR_SCAN_FAIL, "\\" },
    { 47, TR_SCAN_FAIL, "^" },
    { 48, TR_SCAN_FAIL, "`" },
    { 49, TR_SCAN_FAIL, "~" },
    { 50, TR_SUCCESS, "(* this is a test of comments *) test = test, 'a'; (* comment *)" },
    { 51, TR_SUCCESS, "other = ' ' | ':' | '+' | '_' | '%' | '@' | '&' | '#' | '$' | '<' | '>' | '\\' | '^' | '`' | '~';" },
    { 52, TR_SUCCESS, "special = ? ISO 6429 character Horizontal Tabulation ?;" },
    { 53, TR_SUCCESS, "newline = {? ISO 6429 character Carriage Return ?}, ? ISO 6429 character Line Feed ?, {? ISO 6429 character Carriage Return ?};" },
    { 54, TR_PARSE_FAIL, "test = 'test';;" },   // double semicolon
    { 55, TR_SUCCESS, "gapfreesymbol = terminalcharacter - (firstquotesymbol | secondquotesymbol) | terminalstring;" },
    { 56, TR_SUCCESS, "syntax = syntaxrule, {syntaxrule};" },
    { 57, TR_SUCCESS, "syntax = syntaxrule, {syntaxrule}; syntaxrule = metaidentifier, '=', definitionslist, ';';" },
    { 58, TR_SUCCESS, "definitionslist = singledefinition, {definitionseparatorsymbol, singledefinition};" },
    { 59, TR_SUCCESS, "(*singledefinition *) singledefinition = syntacticterm, {concatenatesymbol, syntacticterm}; concatenatesymbol = ',';" },
    { 60, TR_SUCCESS, "comment = '(*', {commentsymbol}, '*)' (* A comment is allowed anywhere outside a <terminal string>, <meta identifier>, <integer> or <special sequence> *);" },
    { 61, TR_SUCCESS, "empty = ;" },
    { 62, TR_SUCCESS, "text = character { character } | ;" },
};

TEST_RETURN just_do_it(const std::string& str)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    TEST_RETURN ret = TR_SCAN_FAIL;
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
