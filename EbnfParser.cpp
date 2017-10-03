// EbnfParser.cpp --- EBNF parser
/////////////////////////////////////////////////////////////////////////

#include "EbnfParser.hpp"

bool just_do_it(const std::string& str)
{
    using namespace EbnfParser;

    StringScanner scanner(str);

    TokenStream stream(scanner);

    bool ret = false;
    os_type os;
    if (stream.scan_tokens())
    {
        stream.to_dbg(os);
        os << "scanned\n";

        Parser parser(stream);
        if (parser.parse())
        {
            os << "parsed\n";
            BaseAst *ast = parser.ast();
            ret = true;

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
    puts(os.str().c_str());
    return ret;
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
