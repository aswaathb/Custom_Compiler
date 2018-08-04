#include "ast.hpp"

#include <string.h>
#include <cstddef>
#include <fstream>


void check_file(FILE *source_file, std::ofstream &out_file, char *argv[]) {

    if (!out_file.is_open())
    {
        std::string message = "out_file could not be opened.\n";
            printf("%s",message.c_str());
        exit(EXIT_FAILURE);
    }
    else if (source_file == NULL)
        {
            std::string message = "source_file could not be opened.\n";
            printf("%s",message.c_str());
            exit(EXIT_FAILURE);
        }
}


void print_assembly(FILE* is, std::ostream &dst, std::string fileName) {
        yyin = is;
        const Node *ast=parseAST();
        Context context = new Context(nullptr);
        dst <<"\t.file\t1 \""<<fileName<<"\"\n"
            <<"\t.section .mdebug.abi32\n\t.previous\n"
            <<"\t.nan\tlegacy\n\t.module fp=xx\n"
            <<"\t.module nooddspreg\n\t.abicalls\n\n"   ;


        ast->generate_assembly(dst, context);
}


int main(int argc, char *argv[])
{
    if (strcmp(argv[1],"-S")==0 && strcmp(argv[3],"-o")==0) {
        std::string fileName = argv[2];
        FILE *source_file =fopen(argv[2], "r");
        std::ofstream out_file(argv[4]);
        check_file(source_file, out_file, argv);

        print_assembly(source_file, out_file, fileName);
    }

    return 0;
}
