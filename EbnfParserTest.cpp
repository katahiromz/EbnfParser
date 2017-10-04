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
    size_t num_rules;   // number of rules
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
    { 1, 0, TR_SCAN_FAIL,      "list = '';" }, // empty string
    { 2, 0, TR_SCAN_FAIL,      "list = \"\";" }, // empty string
    { 3, 0, TR_SCAN_FAIL,      "underline_not_allowed" },    // invalid identifier
#endif
    { 4, 1, TR_SUCCESS,        "list = \"a\";" },
    { 5, 0, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list list;" },  // comma needed
    { 6, 0, TR_PARSE_FAIL,     "list = \"a\"; arg = list | list, list" },  // semicolon needed
    { 7, 2, TR_SUCCESS,        "list = \"a\"; arg = list | list, list;" },
    { 8, 0, TR_PARSE_FAIL,     "list v = \"a\";" },    // invalid syntax
    { 9, 0, TR_PARSE_FAIL,     "list = v \"a\";" },    // comma needed
    { 10, 0, TR_PARSE_FAIL,    "'a' \"a\"" },  // invalid syntax
    { 11, 0, TR_PARSE_FAIL,    "z = 'a' \"a\"" }, // comma needed
    { 12, 1, TR_SUCCESS,       "z = 'a', \"a\";" },
    { 13, 1, TR_SUCCESS,       "z = (a | b | c);" },
    { 14, 1, TR_SUCCESS,       "z = [a , b, c];" },
    { 15, 1, TR_SUCCESS,       "z = [a | b | c];" },
    { 16, 1, TR_SUCCESS,       "z = [a | (b | c)];" },
    { 17, 0, TR_PARSE_FAIL,    "z = [a | (b | c)]; a = test" },   // semicolon needed
    { 18, 2, TR_SUCCESS,       "z = [a | (b | c)]; a = test;" },
    { 19, 0, TR_PARSE_FAIL,    "'z' = a; a = test;" },    // invalid syntax
    { 20, 0, TR_PARSE_FAIL,    "'z';" },    // invalid syntax
    { 21, 0, TR_PARSE_FAIL,    "z;" },    // invalid syntax
    { 22, 0, TR_PARSE_FAIL,    "z" },    // invalid syntax
    { 23, 0, TR_SCAN_FAIL,     "\"not-terminated" },    // invalid string
    { 24, 0, TR_SCAN_FAIL,     "\'not-terminated" },    // invalid string
    { 25, 0, TR_SCAN_FAIL,     "?not-terminated" },    // invalid special
    { 26, 0, TR_SCAN_FAIL,     "(*not-terminated" },    // invalid comment // *)
    { 27, 1, TR_SUCCESS,       "xx = \"A\" - xx;" },
    { 28, 1, TR_SUCCESS,       "line = 5 * \" \", (character - (\" \" | \"0\")), 66 * [character];" },
    { 29, 1, TR_SUCCESS,
        "line = character - \"C\", 4 * character, character - (\" \" | \"0\"), 66 * [character];" },
    { 30, 7, TR_SUCCESS,
        "aa = \"A\";\n"
        "bb = 3 * aa, \"B\";\n"
        "cc = 3 * [aa], \"C\";\n"
        "dd = {aa}, \"D\";\n"
        "ee = aa, {aa}, \"E\";\n"
        "ff = 3 * aa, 3 * [aa], \"F\";\n"
        "gg = 3 * {aa}, \"D\";\n" },
    { 31, 1, TR_SUCCESS,
        "letter = 'A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | "
        "'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | "
        "'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z';\n" },
    { 32, 1, TR_SUCCESS, "vowel = 'A' | 'E' | 'I' | 'O' | 'U';" },
    { 33, 1, TR_SUCCESS, "ee = {'A'} - , 'E';" },
    { 34, 0, TR_SCAN_FAIL, "." },
    { 35, 0, TR_SCAN_FAIL, ":" },
    { 36, 0, TR_SCAN_FAIL, "!" },
    { 37, 0, TR_SCAN_FAIL, "+" },
    { 38, 0, TR_SCAN_FAIL, "%" },
    { 39, 0, TR_SCAN_FAIL, "@" },
    { 40, 0, TR_SCAN_FAIL, "&" },
    { 41, 0, TR_SCAN_FAIL, "#" },
    { 42, 0, TR_SCAN_FAIL, "$" },
    { 43, 0, TR_SCAN_FAIL, "<" },
    { 44, 0, TR_SCAN_FAIL, ">" },
    { 45, 0, TR_SCAN_FAIL, "/" },
    { 46, 0, TR_SCAN_FAIL, "\\" },
    { 47, 0, TR_SCAN_FAIL, "^" },
    { 48, 0, TR_SCAN_FAIL, "`" },
    { 49, 0, TR_SCAN_FAIL, "~" },
    { 50, 1, TR_SUCCESS, "(* this is a test of comments *) test = test, 'a'; (* comment *)" },
    { 51, 1, TR_SUCCESS,
        "other = ' ' | ':' | '+' | '_' | '%' | '@' | '&' | '#' | '$' | "
        "'<' | '>' | '\\' | '^' | '`' | '~';" },
    { 52, 1, TR_SUCCESS, "special = ? ISO 6429 character Horizontal Tabulation ?;" },
    { 53, 1, TR_SUCCESS,
        "newline = {? ISO 6429 character Carriage Return ?}, "
        "? ISO 6429 character Line Feed ?, {? ISO 6429 character Carriage Return ?};" },
    { 54, 0, TR_PARSE_FAIL, "test = 'test';;" },   // double semicolon
    { 55, 1, TR_SUCCESS,
        "gapfreesymbol = terminalcharacter - (firstquotesymbol | secondquotesymbol) | terminalstring;" },
    { 56, 1, TR_SUCCESS, "syntax = syntaxrule, {syntaxrule};" },
    { 57, 2, TR_SUCCESS,
        "syntax = syntaxrule, {syntaxrule};\r\n"
        "syntaxrule = metaidentifier, '=', definitionslist, ';';" },
    { 58, 1, TR_SUCCESS,
        "definitionslist = singledefinition, {definitionseparatorsymbol, singledefinition};" },
    { 59, 2, TR_SUCCESS,
        "(*singledefinition *) singledefinition = syntacticterm, {concatenatesymbol, syntacticterm};\n"
        "concatenatesymbol = ',';" },
    { 60, 1, TR_SUCCESS,
        "comment = '(*', {commentsymbol}, '*)' (* A comment is allowed anywhere "
        "outside a <terminal string>, <meta identifier>, <integer> or <special sequence> *);" },
    { 61, 1, TR_SUCCESS, "empty = ;" },
    { 62, 1, TR_SUCCESS, "text = character, { character } | ;" },
    { 63, 1, TR_SUCCESS, "text = | character, { character };" },
    { 64, 1, TR_SUCCESS, "text = { character | };" },
};

TEST_RETURN just_do_it(const std::string& str, size_t& num_rules)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    TEST_RETURN ret = TR_SCAN_FAIL;
    os_type os;
    os << "input: " << str << std::endl;
    num_rules = 0;
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

            if (ast->m_id == ASTID_SEQ)
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

bool do_test_entry(const TEST_ENTRY *entry)
{
    bool failed = false;
    size_t num_rules = 0;
    int ret = just_do_it(entry->input, num_rules);
    if (ret != entry->ret)
    {
        printf("#%d: FAILED: expected %d, got %d\n", entry->entry_number, entry->ret, ret);
        ++g_failed;
        failed = true;
    }
    ++g_executed;
    if (num_rules != entry->num_rules)
    {
        printf("#%d: FAILED: expected %u, got %u\n",
               entry->entry_number, (int)entry->num_rules, (int)num_rules);
        ++g_failed;
        failed = true;
    }
    ++g_executed;
    return !failed;
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
    return g_failed;
}
