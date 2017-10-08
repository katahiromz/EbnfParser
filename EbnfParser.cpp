// EbnfParser.cpp --- EBNF notation parser
// See ReadMe.txt and License.txt.
/////////////////////////////////////////////////////////////////////////

#include "EBNF.hpp"
#include <fstream>
#include <cstdio>       // for std::puts

int parse(const std::string& str)
{
    int ret = 1;
    using namespace EBNF;

    StringScanner scanner(str);

    AuxInfo aux;
    TokenStream stream(scanner, aux);

    os_type os;
    if (stream.scan())
    {
        ret = 2;
        stream.fixup();

        stream.to_dbg(os);

        Parser parser(stream, aux);
        if (parser.parse())
        {
            ret = 0;
            BaseAst *ast = parser.ast();

            os << "\nto_dbg:\n";
            ast->to_dbg(os);
            os << "\n\nto_bnf:\n";
            ast->to_ebnf(os);
        }
        else
        {
            os << "parse error\n";
        }
    }
    else
    {
        os << "scan error\n";
    }
    aux.err_out(os);

    puts(os.str().c_str());

    return ret;
}

void show_help(void)
{
    printf("Usage: EbnfParser [options] file.txt\n");
    printf("Options:\n");
    printf("--version    Show version info\n");
    printf("--help       Show help\n");
}

void show_version(void)
{
    printf("EbnfParser version %d by katahiromz\n", EBNF_HPP_);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        show_help();
        return 1;
    }

    char *file = NULL;
    for (int i = 1; i < argc; ++i)
    {
        char *arg = argv[i];
        if (strcmp(arg, "--help") == 0)
        {
            show_help();
            return 0;
        }
        if (strcmp(arg, "--version") == 0)
        {
            show_version();
            return 0;
        }
        if (file == NULL)
        {
            file = arg;
        }
        else
        {
            printf("ERROR: multiple input files specified\n");
        }
    }

    std::ifstream ifs(file);
    if (ifs.fail())
        return -1;

    int ret;
    std::istreambuf_iterator<char> it(ifs), end;
    std::string str(it, end);
    ret = parse(str);

    assert(EBNF::BaseAst::alive_count() == 0);
    return ret;
}
