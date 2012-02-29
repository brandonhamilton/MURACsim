
#include "mem_access.hpp"
#include "../../../framework/murac.h"

using std::cout;
using std::endl;

MURAC_AA_EXECUTE(example) {
    cout << "[AA] Starting memory access simulation" << endl;
    run_mem_access_simulation(stack);
    return 1;
}