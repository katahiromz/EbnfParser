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
        stream.print_tokens();
        printf("scanned\n");

        Parser parser(stream);
        if (parser.parse())
        {
            printf("parsed\n");
            BaseAst *ast = parser.ast();
            ast->print();
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
