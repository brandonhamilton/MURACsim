
#include "simple.hpp"
#include "../../../framework/murac.h"

using std::cout;
using std::endl;

MURAC_AA_EXECUTE(example) {
    cout << "[AA] MURAC AA Simulator Example" << endl;
    run_simple_simulation(stack);
    return 1;
}