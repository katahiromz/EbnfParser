// EbnfParser.cpp --- EBNF parser
/////////////////////////////////////////////////////////////////////////

#include "EbnfParser.hpp"

bool just_do_it(const std::string& str)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    if (stream.scan_tokens())
    {
        os_type os;
        stream.to_dbg(os);
        puts(os.str().c_str());

        printf("scanned\n");

        Parser parser(stream);
        if (parser.parse())
        {
            printf("parsed\n");
            BaseAst *ast = parser.ast();

            os.clear();
            ast->to_dbg(os);
            puts(os.str().c_str());
            return true;
        }
        parser.print_errors();
    }
    return false;
}

int main(void)
{
    just_do_it(
        "list = list, 'test' | dummy;\n"
        "dummy = 'dummy';\n"
    );

    assert(EbnfParser::BaseAst::alive_count() == 0);
    return 0;
}
