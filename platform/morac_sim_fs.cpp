/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>

#ifdef USE_SPECMAN
#include "src/snsc_user.h"
#endif


#include "tlm.h"
#include "ovpworld.org/modelSupport/tlmPlatform/1.0/tlm2.0/tlmPlatform.hpp"
#include "ovpworld.org/modelSupport/tlmDecoder/1.0/tlm2.0/tlmDecoder.hpp"
#include "ovpworld.org/memory/ram/1.0/tlm2.0/tlmMemory.hpp"
#include "arm.ovpworld.org/processor/arm/1.0/tlm2.0/processor.igen.hpp"
#include "arm.ovpworld.org/peripheral/CoreModule9x6/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/IntICP/1.01/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/IcpCounterTimer/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/IcpControl/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/DebugLedAndDipSwitch/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/KbPL050/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/RtcPL031/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/UartPL011/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/MmciPL181/1.0/tlm2.0/pse.igen.hpp"
#include "arm.ovpworld.org/peripheral/SmartLoaderArmLinux/1.0/tlm2.0/pse.igen.hpp"

typedef enum bootConfigE { BCONF_UBOOT, BCONF_LINUX, BCONF_BAREMETAL } bootConfig;

////////////////////////////////////////////////////////////////////////////////
//                            ArmIntegratorCP Class                           //
////////////////////////////////////////////////////////////////////////////////

class ArmIntegratorCP : public sc_core::sc_module {

  public:
    ArmIntegratorCP (sc_core::sc_module_name name,
		bootConfig bconf,
		bool connect,
		const char * variant
    );

    icmTLMPlatform        Platform;
    decoder             <3,15> bus1;
    ram                   ram1;
    ram                   ambaDummy;
    arm                   arm1;
    CoreModule9x6         cm;
    IntICP                pic1;
    IntICP                pic2;
    IcpCounterTimer       pit;
    IcpControl            icp;
    DebugLedAndDipSwitch  ld1;
    KbPL050               kb1;
    KbPL050               ms1;
    RtcPL031              rtc;
    UartPL011             uart1;
    UartPL011             uart2;
    MmciPL181             mmci;
    SmartLoaderArmLinux   smartLoader;
    ram                   lcdDummy;

    icmAttrListObject *attrsForarm1(const char * variant) {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("showHiddenRegs", "0");
        userAttrs->addAttr("compatibility", "ISA");
        userAttrs->addAttr("variant", variant);
        userAttrs->addAttr("endian", "little");
        userAttrs->addAttr("mips", 200);
        userAttrs->addAttr("override_debugMask",0);
        return userAttrs;
    }

    icmAttrListObject *attrsForkb1() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("isMouse", 0);
        userAttrs->addAttr("grabDisable", 0);
        return userAttrs;
    }

    icmAttrListObject *attrsForms1() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("isMouse", 1);
        userAttrs->addAttr("grabDisable", 1);
        return userAttrs;
    }

    icmAttrListObject *attrsForuart1(bool connect) {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        if(connect) {
            userAttrs->addAttr("portnum", 9000);
            userAttrs->addAttr("finishOnDisconnect", "on");
        }
        userAttrs->addAttr("outfile", "uart1.log");
        userAttrs->addAttr("variant", "ARM");
        return userAttrs;
    }

    icmAttrListObject *attrsForuart2() {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        userAttrs->addAttr("outfile", "uart2.log");
        userAttrs->addAttr("variant", "ARM");
        return userAttrs;
    }

    icmAttrListObject *attrsForsmartLoader(bootConfig bc) {
        icmAttrListObject *userAttrs = new icmAttrListObject;
        // Local Image
        userAttrs->addAttr("initrd", "fs.img");
        userAttrs->addAttr("kernel", "zImage");
        userAttrs->addAttr("command", "console=ttyAMA0,38400n8");
        if (bc != BCONF_LINUX) {
            userAttrs->addAttr("disable", "True");
        }
        return userAttrs;
    }
}; /* ArmIntegratorCP */

////////////////////////////////////////////////////////////////////////////////
//                         ArmIntegratorCP Constructor                        //
////////////////////////////////////////////////////////////////////////////////

ArmIntegratorCP::ArmIntegratorCP (
        sc_core::sc_module_name name,
        bootConfig bconf,
        bool connect,
        const char *variant
  )
    : sc_core::sc_module (name)
    , Platform ("icm", ICM_VERBOSE | ICM_STOP_ON_CTRLC| ICM_ENABLE_IMPERAS_INTERCEPTS | ICM_WALLCLOCK)
    , bus1("bus1")
    , ram1 ("ram1", "sp1", 0x8000000)
    , ambaDummy ("ambaDummy", "sp1", 0x1000)
    , arm1 ( "arm1", 0, ICM_ATTR_SIMEX | ICM_ATTR_TRACE_ICOUNT | ICM_ATTR_RELAXED_SCHED, attrsForarm1(variant))
    , cm ("cm")
    , pic1 ("pic1")
    , pic2 ("pic2")
    , pit ("pit")
    , icp ("icp")
    , ld1 ("ld1")
    , kb1 ("kb1", attrsForkb1())
    , ms1 ("ms1", attrsForms1())
    , rtc ("rtc")
    , uart1 ("uart1", attrsForuart1(connect))
    , uart2 ("uart2", attrsForuart2())
    , mmci ("mmci")
    , smartLoader ("smartLoader", attrsForsmartLoader(bconf))
    , lcdDummy("lcdDummy", "sp1", 0x8001000)
{

    // bus1 masters
    arm1.INSTRUCTION.socket(bus1.target_socket[0]);
    arm1.DATA.socket(bus1.target_socket[1]);
    smartLoader.mport.socket(bus1.target_socket[2]);

    // bus1 slaves
    bus1.initiator_socket[0](cm.bport1.socket); // Peripheral
    bus1.setDecode(0, 0x10000000, 0x10000fff);

    bus1.initiator_socket[1](pic1.bport1.socket); // Peripheral
    bus1.setDecode(1, 0x14000000, 0x14000fff);

    bus1.initiator_socket[2](pic2.bport1.socket); // Peripheral
    bus1.setDecode(2, 0xca000000, 0xca000fff);

    bus1.initiator_socket[3](pit.bport1.socket); // Peripheral
    bus1.setDecode(3, 0x13000000, 0x13000fff);

    bus1.initiator_socket[4](icp.bport1.socket); // Peripheral
    bus1.setDecode(4, 0xcb000000, 0xcb00000f);

    bus1.initiator_socket[5](ld1.bport1.socket); // Peripheral
    bus1.setDecode(5, 0x1a000000, 0x1a000fff);

    bus1.initiator_socket[6](kb1.bport1.socket); // Peripheral
    bus1.setDecode(6, 0x18000000, 0x18000fff);

    bus1.initiator_socket[7](ms1.bport1.socket); // Peripheral
    bus1.setDecode(7, 0x19000000, 0x19000fff);

    bus1.initiator_socket[8](rtc.bport1.socket); // Peripheral
    bus1.setDecode(8, 0x15000000, 0x15000fff);

    bus1.initiator_socket[9](uart1.bport1.socket); // Peripheral
    bus1.setDecode(9, 0x16000000, 0x16000fff);

    bus1.initiator_socket[10](uart2.bport1.socket); // Peripheral
    bus1.setDecode(10, 0x17000000, 0x17000fff);

    bus1.initiator_socket[11](mmci.bport1.socket); // Peripheral
    bus1.setDecode(11, 0x1c000000, 0x1c000fff);

    bus1.initiator_socket[12](ram1.sp1); // Memory
    bus1.setDecode(12, 0x0, 0x7ffffff);

    bus1.initiator_socket[13](ambaDummy.sp1); // Memory
    bus1.setDecode(13, 0x1d000000, 0x1d000fff);

    bus1.initiator_socket[14](lcdDummy.sp1); // Memory
    bus1.setDecode(14, 0xC0000000, 0xC8000FFF);

    // Net connections
    pic1.irq(arm1.irq);
    pic1.fiq(arm1.fiq);
    uart1.irq(pic1.ir1);
    uart2.irq(pic1.ir2);
    kb1.irq(pic1.ir3);
    ms1.irq(pic1.ir4);
    pit.irq0(pic1.ir5);
    pit.irq1(pic1.ir6);
    pit.irq2(pic1.ir7);
    rtc.irq(pic1.ir8);
    mmci.irq0(pic1.ir23);
    mmci.irq1(pic1.ir24);
}

int sc_main (int argc, char *argv[] )
{
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);

	sc_set_time_resolution(1,SC_NS);

	bootConfig bc = BCONF_LINUX;

    bool  timeoutSet = false;
    bool  connect = true;
    const char* variant = "ARM926EJ-S";
    sc_time stop(0,SC_MS);


    int i;
    for(i=1; i < argc; i++) {
        if (strcmp(argv[i], "t") == 0) {
            stop = sc_time(atoi(argv[++i]), SC_MS);
            timeoutSet = true;
        } else if (strcmp(argv[i], "n") == 0) {
            connect = false;
        } else if (strcmp(argv[i], "v") == 0) {
            variant = argv[++i];
        } else {
            cout << "Usage: " << argv[0] << " [n] [t <time>] [v <variant>]" << endl;
            cout << "       t = timeout for simulation in milliseconds" << endl;
            cout << "       n = no interactive connection to uart (logging only)" << endl;
            cout << "       v = set variant for the arm processor (default ARM926EJ-S)" << endl;
            exit(0);
        }
    }

    // Ignore some of the Warning messages
    icmIgnoreMessage ("ICM_NPF");

    cout << "Constructing." << endl;
    ArmIntegratorCP top("top", bc, connect, variant);             // instantiate example top module

    top.arm1.setIPS(200000000);
    top.cm.setDiagnosticLevel(3);
    top.uart1.setDiagnosticLevel(1);
    top.uart2.setDiagnosticLevel(1);
    top.smartLoader.setDiagnosticLevel(7);

    cout << "Starting sc_main." << endl;

	cout << "default time resolution = " << sc_get_time_resolution() << endl;

	#ifdef USE_SPECMAN
		specman_init();
		specman_command("load platform/main.e; test");
	#endif

    if (timeoutSet) {
        sc_core::sc_start(stop);                  // start the simulation
    } else {
        sc_core::sc_start();                      // start the simulation
    }

    cout << "Finished sc_main." << endl;
    return 0;                             // return okay status
}

