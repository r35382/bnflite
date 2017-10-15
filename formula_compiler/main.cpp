
#include "byte_code.h"


int main(int argc, char* argv[])
{
    std::string expression;
    for (int i = 1; i < argc; i++) {
        expression += argv[i];
    }

    std::list<byte_code> bl = bnflite_byte_code(expression);

    std::cout  <<  "Byte-code: ";
    for (std::list<byte_code>::iterator itr = bl.begin(); itr != bl.end(); ++itr)
        std::cout << *itr << ',';
    std::cout <<  std::endl;

    union { int val_i;  float val_f; } res[4] = {0};
    int err = EvaluateBC(bl, res);
    if (err)
        std::cout << "running error: "  << err << std::endl;
    else if (byte_code::toType(bl.back().type) == opInt )
        std::cout << "result = " << res[0].val_i << ", " << res[1].val_i << ", "
            << res[2].val_i << ", " << res[3].val_i <<  std::endl;
    else
        std::cout << "result = " << res[0].val_f << ", " << res[1].val_f << ", "
            << res[2].val_f << ", " << res[3].val_f <<  std::endl;

    return err;
}






