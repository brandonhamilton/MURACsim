
#include "simple.hpp"
#include "../../../framework/morac.h"

using std::cout;
using std::endl;

MORAC_AA_EXECUTE(example) {
    cout << "RUNNING MORAC AA SIMULATOR EXAMPLE" << endl;
    run_simple_simulation(stack);
    return 1;
}