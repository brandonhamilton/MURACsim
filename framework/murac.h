/**
 * MURAC software framework
 * 
 * Author: Brandon Hamilton <brandon.hamilton@gmail.com>
 */

#ifndef MURAC_H
#define MURAC_H


#define MURAC_AA_EXECUTE(NAME) extern "C" int murac_execute(unsigned long int stack)

#define MURAC_AA_INIT(NAME) extern "C" int murac_init(BusInterface* bus)

#define MURAC_SET_PTR(ADDR) asm volatile("mov r0,%[value]" : : [value]"r"(ADDR) : "r0");

class BusInterface {
public:
    virtual int read(unsigned long int addr, unsigned char*data, unsigned int len) = 0;
    virtual int write(unsigned long int addr, unsigned char*data, unsigned int len) = 0;
};

/*
#ifdef __cplusplus
extern "C" {
#endif

extern "C" int murac_simulate(BusInterface* bus, unsigned long int stack);

#ifdef __cplusplus
}
#endif
*/
#endif // MURAC_H
