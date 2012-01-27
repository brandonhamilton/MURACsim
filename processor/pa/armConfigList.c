/*
 * Copyright (c) 2005-2011 Imperas Software Ltd., www.imperas.com
 *
 * YOUR ACCESS TO THE INFORMATION IN THIS MODEL IS CONDITIONAL
 * UPON YOUR ACCEPTANCE THAT YOU WILL NOT USE OR PERMIT OTHERS
 * TO USE THE INFORMATION FOR THE PURPOSES OF DETERMINING WHETHER
 * IMPLEMENTATIONS OF THE ARM ARCHITECTURE INFRINGE ANY THIRD
 * PARTY PATENTS.
 *
 * THE LICENSE BELOW EXTENDS ONLY TO USE OF THE SOFTWARE FOR
 * MODELING PURPOSES AND SHALL NOT BE CONSTRUED AS GRANTING
 * A LICENSE TO CREATE A HARDWARE IMPLEMENTATION OF THE
 * FUNCTIONALITY OF THE SOFTWARE LICENSED HEREUNDER.
 * YOU MAY USE THE SOFTWARE SUBJECT TO THE LICENSE TERMS BELOW
 * PROVIDED THAT YOU ENSURE THAT THIS NOTICE IS REPLICATED UNMODIFIED
 * AND IN ITS ENTIRETY IN ALL DISTRIBUTIONS OF THE SOFTWARE,
 * MODIFIED OR UNMODIFIED, IN SOURCE CODE OR IN BINARY FORM.
 *
 * Licensed under an Imperas Modfied Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.ovpworld.org/licenses/OVP_MODIFIED_1.0_APACHE_OPEN_SOURCE_LICENSE_2.0.pdf
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

// model header files
#include "armConfig.h"
#include "armVariant.h"

const struct armConfigS armConfigTable[] = {

    ////////////////////////////////////////////////////////////////////////////
    // ISA CONFIGURATIONS
    ////////////////////////////////////////////////////////////////////////////

    {.name = "ARMv4T",    .arch = ARM_V4T,    .rotateUnaligned = True},
    {.name = "ARMv4xM",   .arch = ARM_V4XM,   .rotateUnaligned = True},
    {.name = "ARMv4",     .arch = ARM_V4,     .rotateUnaligned = True},
    {.name = "ARMv4TxM",  .arch = ARM_V4TXM,  .rotateUnaligned = True},
    {.name = "ARMv5xM",   .arch = ARM_V5XM,   .rotateUnaligned = True},
    {.name = "ARMv5",     .arch = ARM_V5,     .rotateUnaligned = True},
    {.name = "ARMv5TxM",  .arch = ARM_V5TXM,  .rotateUnaligned = True},
    {.name = "ARMv5T",    .arch = ARM_V5T,    .rotateUnaligned = True},
    {.name = "ARMv5TExP", .arch = ARM_V5TEXP, .rotateUnaligned = True},
    {.name = "ARMv5TE",   .arch = ARM_V5TE,   .rotateUnaligned = True},
    {.name = "ARMv5TEJ",  .arch = ARM_V5TEJ,  .rotateUnaligned = True},
    {.name = "ARMv6",     .arch = ARM_V6,     .rotateUnaligned = True},
    {.name = "ARMv6K",    .arch = ARM_V6K,    .rotateUnaligned = True},
    {.name = "ARMv6T2",   .arch = ARM_V6T2,   .rotateUnaligned = True},
    {.name = "ARMv7",     .arch = ARM_V7,     .rotateUnaligned = True},

    ////////////////////////////////////////////////////////////////////////////
    // PROCESSOR MODEL CONFIGURATIONS
    ////////////////////////////////////////////////////////////////////////////

    {
        .name            = "ARM7TDMI",
        .arch            = ARM_V4T,
        .fcsePresent     = False,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .ignoreBadCp15   = True,

        .cpRegDefaults = {
            .MIDR = {
                .arm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x700,
                    .Variant      = 0x0,
                    .A            = 1,
                    .Implementor  = AI_ARM
                },
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled (not present)
                .A  = 0,            // alignment checking disabled (not present)
                .C  = 0,            // cache disabled (not present)
                .W  = 0,            // write buffer disabled (not present)
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit (not used)
                .R  = 0,            // ROM protection bit (not used)
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disable (ignored)
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        }
    },

    {
        .name            = "ARM7EJ-S",
        .arch            = ARM_V5TEJ,
        .fcsePresent     = False,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .ignoreBadCp15   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x700,
                    .Architecture = 0x6,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled (not present)
                .A  = 0,            // alignment checking disabled (not present)
                .C  = 0,            // cache disabled (not present)
                .W  = 0,            // write buffer disabled (not present)
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit (not used)
                .R  = 0,            // ROM protection bit (not used)
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disable (ignored)
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        }
    },

    {
        .name            = "ARM720T",
        .arch            = ARM_V4T,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .ignoreBadCp15   = True,

        .cpRegDefaults = {
            .MIDR = {
                .arm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x720,
                    .Variant      = 0x0,
                    .A            = 1,
                    .Implementor  = AI_ARM
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM920T",
        .arch            = ARM_V4T,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x920,
                    .Architecture = 0x2,
                    .Variant      = 0x1,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 6,   // 64 way
                    .Isize_size  = 5,   // 16k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 6,   // 64 way
                    .Dsize_size  = 5,   // 16k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 6    // write-back, format A
                }
            },
            .TLBTR = {
                .nU     = 1,        // not unified TLB
                .DLsize = 0,        // number of unified/data lockable entries
                .ILsize = 0         // number of instruction lockable entries
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM922T",
        .arch            = ARM_V4T,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x922,
                    .Architecture = 0x2,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 6,   // 64 way
                    .Isize_size  = 4,   // 8k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 6,   // 64 way
                    .Dsize_size  = 4,   // 8k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 6    // write-back, format A
                }
            },
            .TLBTR = {
                .nU     = 1,        // not unified TLB
                .DLsize = 0,        // number of unified/data lockable entries
                .ILsize = 0         // number of instruction lockable entries
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM926EJ-S",
        .arch            = ARM_V5TEJ,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 5,
                    .PartNum      = 0x926,
                    .Architecture = 0x6,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 2,   // 4 way
                    .Isize_size  = 5,   // 16k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 2,   // 4 way
                    .Dsize_size  = 4,   // 8k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 14   // write-back, format C
                }
            },
            .TLBTR = {
                .nU     = 0,        // unified TLB
                .DLsize = 8,        // number of unified/data lockable entries
                .ILsize = 8         // number of instruction lockable entries
            },
            .TCMTR = {
                .DTCM = 1,          // single data TCM present
                .ITCM = 1           // single instruction TCM present
            },
            .DTCMRR = {
                .Size = 6           // 32kb data TCM
            },
            .ITCMRR = {
                .Size = 6           // 32kb instruction TCM
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM940T",
        .arch            = ARM_V4T,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0x940,
                    .Architecture = 0x2,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 1,   // 16 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 6,   // 64 way
                    .Isize_size  = 3,   // 4k cache
                    .Dsize_len   = 1,   // 16 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 6,   // 64 way
                    .Dsize_size  = 3,   // 8k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 7    // write-back, format B
                }
            },
            .MPUIR = {
                .nU      = 1,       // separate MPU
                .DRegion = 8,       // number of data memory regions
                .IRegion = 8        // number of instruction memory regions
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 0,    // system protection DISABLED
                    .R  = 0,    // ROM protection DISABLED
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM946E",
        .arch            = ARM_V5TE,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .mpuV5ExtAP      = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 1,
                    .PartNum      = 0x946,
                    .Architecture = 0x5,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 1,   // 2 way (implementation-specific)
                    .Isize_size  = 7,   // 64k cache (implementation-specific)
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 1,   // 2 way (implementation-specific)
                    .Dsize_size  = 7,   // 64k cache (implementation-specific)
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 7    // write-back, format B
                }
            },
            .MPUIR = {
                .nU      = 0,       // unified MPU
                .DRegion = 8,       // number of data/unified memory regions
                .IRegion = 0        // number of instruction memory regions
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 0,    // system protection DISABLED
                    .R  = 0,    // ROM protection DISABLED
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .L4 = 1,    // backwards-compatible load enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM966E",
        .arch            = ARM_V5TE,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 1,
                    .PartNum      = 0x966,
                    .Architecture = 0x5,
                    .Variant      = 0x2,
                    .Implementor  = AI_ARM
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled (not present)
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 1,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .L4 = 1,    // backwards-compatible load enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM968E-S",
        .arch            = ARM_V5TE,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 1,
                    .PartNum      = 0x968,
                    .Architecture = 0x5,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM1020E",
        .arch            = ARM_V5TE,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0xa20,
                    .Architecture = 0x5,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 1,   // 2 way
                    .Isize_size  = 6,   // 32k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 1,   // 2 way
                    .Dsize_size  = 6,   // 32k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 0    // write-through
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                    .L4 = 1,    // backwards-compatible load enable
                }
            }
        }
    },

    {
        .name            = "ARM1022E",
        .arch            = ARM_V5TE,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0xa22,
                    .Architecture = 0x5,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 1,   // 2 way
                    .Isize_size  = 5,   // 16k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 1,   // 2 way
                    .Dsize_size  = 5,   // 16k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 0    // write-through
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enableRegression_Tests/OVPsim/Demo/OVPsim_linux_ArmIntegratorCP_tlm2.0
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                    .L4 = 1,    // backwards-compatible load enable
                }
            }
        }
    },

    {
        .name            = "ARM1026EJ-S",
        .arch            = ARM_V5TEJ,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 2,
                    .PartNum      = 0xa26,
                    .Architecture = 0x6,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 1,   // 2 way
                    .Isize_size  = 4,   // 8k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 1,   // 2 way
                    .Dsize_size  = 4,   // 8k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 0    // write-through
                }
            },
            .SCTLR = {
                .M  = 0,            // MMU disabled
                .A  = 0,            // alignment checking disabled
                .C  = 0,            // data cache disabled
                .W  = 1,            // write buffer enabled
                .P  = 1,            // 32-bit exception handlers
                .D  = 1,            // 26-bit address exception checking disabled
                .L  = 1,            // late abort model selected
                .B  = 0,            // endianness
                .S  = 0,            // system protection bit
                .R  = 0,            // ROM protection bit
                .SW = 0,            // (implementation defined)
                .Z  = 0,            // branch prediction (ignored)
                .I  = 0,            // instruction cache disabled
                .V  = 0,            // high vectors
                .RR = 0,            // replacement strategy (ignored)
                .L4 = 0,            // backwards-compatible load
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M  = 1,    // MMU enable
                    .A  = 1,    // alignment checking enable
                    .C  = 1,    // data cache enable
                    .W  = 1,    // write buffer enable
                    .S  = 1,    // system protection enable
                    .R  = 1,    // ROM protection enable
                    .I  = 1,    // instruction cache enable
                    .V  = 1,    // high vector enable
                    .RR = 1,    // replacement strategy enable
                }
            }
        }
    },

    {
        .name            = "ARM1136J-S",
        .arch            = ARM_V6K,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = False,
        .STRoffsetPC12   = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 5,
                    .PartNum      = 0xb36,
                    .Architecture = 0x7,
                    .Variant      = 0x1,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 2,   // 4 way
                    .Isize_size  = 5,   // 16k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 2,   // 4 way
                    .Dsize_size  = 5,   // 16k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 14   // write-back, format C lockdown
                }
            },
            .TCMTR = {
                .DTCM = 1,          // single data TCM present
                .ITCM = 1           // single instruction TCM present
            },
            .DTCMRR = {
                .Size = 5           // 16kb data TCM
            },
            .ITCMRR = {
                .Size = 5           // 16kb instruction TCM
            },
            .TLBTR = {
                .nU     = 0,        // unified TLB
                .DLsize = 8,        // number of unified/data lockable entries
                .ILsize = 8         // number of instruction lockable entries
            },
            .ID_PFR0 = {
                .State0 = 1,        // 32-bit ARM instruction set support
                .State1 = 1,        // Thumb encoding support
                .State2 = 1,        // Jazelle support
                .State3 = 0         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 1, // ARM programmer's model
                .SecurityExtension     = 0, // Security Extensions support
                .MicroProgrammersModel = 0  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 2, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 0, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA               = 3,    // VMSA support
                .PMSA               = 0,    // PMSA support
                .OuterShareability  = 0,    // outer shareability
                .ShareabilityLevels = 0,    // shareability levels
                .TCMSupport         = 3,    // TCM support
                .AuxillaryRegisters = 1,    // ARMv6 Auxillary Control register
                .FCSESupport        = 1,    // FCSE support
                .InnerShareability  = 0     // inner shareability
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 2,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 3,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 3,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 1    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 1,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 1,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 2,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 2,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 2,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 1,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 0, // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 0, // Hierarchical cache maintainence, MVA
                .BPMaint       = 0, // Branch predictor maintainence
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 1,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 0,  // BitField instructions
                .CmpBranch_instrs = 0,  // CmpBranch instructions
                .Coproc_instrs    = 4,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 0,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 1,  // Endian instructions
                .Except_instrs    = 1,  // Exception instructions
                .Except_AR_instrs = 1,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 0,  // IfThen instructions
                .Immediate_instrs = 0,  // Immediate instructions
                .Interwork_instrs = 2,  // Interwork instructions
                .Jazelle_instrs   = 1,  // Jazelle instructions
                .Morac_instrs     = 1   // MORAC instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 1, // MemoryHint instructions
                .MultiAccessInt_instrs = 1, // Multi-access interruptible instructions
                .Mult_instrs           = 1, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 1  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 2,   // SynchPrim instructions
                .TabBranch_instrs    = 0,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 0    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 1, // Unprivileged instructions
                .WithShifts_instrs     = 4, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 0, // SMI instructions
                .Barrier_instrs        = 0, // Barrier instructions
                .SynchPrim_instrs_frac = 0, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 0, // M profile PSR instructions
                .SWP_frac              = 0  // Swap instructions
            },
            .DMAPresent = {
                .CH0 = 1,   // DMA channel 1 present
                .CH1 = 1,   // DMA channel 2 present
            },
            .SCTLR = {
                .M   = 0,           // MMU disabled
                .A   = 0,           // alignment checking disabled
                .C   = 0,           // data cache disabled
                .W   = 1,           // write buffer enabled
                .P   = 1,           // 32-bit exception handlers
                .D   = 1,           // 26-bit address exception checking disabled
                .L   = 1,           // late abort model selected
                .B   = 0,           // endianness
                .S   = 0,           // system protection bit
                .R   = 0,           // ROM protection bit
                .SW  = 0,           // SWP/SWPB enable bit
                .Z   = 0,           // branch prediction (ignored)
                .I   = 0,           // instruction cache disabled
                .V   = 0,           // high vectors
                .RR  = 0,           // replacement strategy (ignored)
                .L4  = 0,           // backwards-compatible load
                .DT  = 1,           // global data TCM enable/disable (redundant)
                .IT  = 1,           // global instruction TCM enable/disable (redundant)
                .FI  = 0,           // low latency features for fast interrupts
                .U   = 0,           // unaligned data access operations
                .XP  = 0,           // subpage AP bits disabled
                .VE  = 0,           // configure vectored interrupt
                .EE  = 0,           // CPSR E bit on taking an exception
                .NMI = 0,           // non-maskable bit (set by pin FIQISNMI)
                .TRE = 0,           // TEX Remap enable
                .AFE = 0,           // access flag enable
                .TE  = 0,           // enter Thumb mode on exception
            },
            .ACTLR = {
                .preV7 = {
                    .RS = 1,        // return stack enable
                    .DB = 1,        // dynamic branch prediction enable
                    .SB = 1,        // static branch predition enable
                    .TR = 0,        // MicroTLB random replacement
                    .RA = 0,        // disable clean entire data cache
                    .RV = 0,        // disable block transfer cache
                    .CZ = 0         // restrict cache size
                }
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M   = 1,   // MMU enable
                    .A   = 1,   // alignment checking enable
                    .C   = 1,   // data cache enable
                    .S   = 1,   // system protection bit
                    .R   = 1,   // ROM protection bit
                    .Z   = 1,   // branch prediction (ignored)
                    .I   = 1,   // instruction cache enable
                    .V   = 1,   // high vector enable
                    .RR  = 1,   // replacement strategy enable
                    .L4  = 1,   // backwards-compatible load
                    .FI  = 1,   // low latency features for fast interrupts
                    .U   = 1,   // unaligned data access operations
                    .XP  = 1,   // subpage AP bits disabled
                    .VE  = 1,   // configure vectored interrupt
                    .EE  = 1,   // CPSR E bit on taking an exception
                    .TRE = 1,   // TEX Remap enable
                    .AFE = 1,   // access flag enable
                }
            }
        }
    },

    {
        .name            = "ARM1156T2-S",
        .arch            = ARM_V6T2,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = False,
        .STRoffsetPC12   = True,
        .IFARPresent     = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 4,
                    .PartNum      = 0xb56,
                    .Architecture = 0xf,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .preV7 = {
                    .Isize_len   = 2,   // 32 byte line
                    .Isize_M     = 0,
                    .Isize_assoc = 2,   // 4 way
                    .Isize_size  = 5,   // 16k cache
                    .Dsize_len   = 2,   // 32 byte line
                    .Dsize_M     = 0,
                    .Dsize_assoc = 2,   // 4 way
                    .Dsize_size  = 5,   // 16k cache
                    .S           = 1,   // separate instruction/data caches
                    .ctype       = 14   // write-back, format C lockdown
                }
            },
            .TCMTR = {
                .DTCM = 1,          // single data TCM present
                .ITCM = 1           // single instruction TCM present
            },
            .DTCMRR = {
                .Size = 6           // 32kb data TCM
            },
            .ITCMRR = {
                .Size = 6           // 32kb instruction TCM
            },
            .MPUIR = {
                .nU      = 0,       // unified MPU
                .DRegion = 16,      // number of data/unified memory regions
                .IRegion = 0        // number of instruction memory regions
            },
            .ID_PFR0 = {
                .State0 = 1,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 1,        // Jazelle support
                .State3 = 0         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 1, // ARM programmer's model
                .SecurityExtension     = 0, // Security Extensions support
                .MicroProgrammersModel = 0  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 2, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 0, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA               = 0,    // VMSA support
                .PMSA               = 2,    // PMSA support
                .OuterShareability  = 0,    // outer shareability
                .ShareabilityLevels = 0,    // shareability levels
                .TCMSupport         = 2,    // TCM support
                .AuxillaryRegisters = 1,    // ARMv6 Auxillary Control register
                .FCSESupport        = 0,    // FCSE support
                .InnerShareability  = 0     // inner shareability
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 2,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 3,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 2,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 0    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 1,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 0,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 2,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 1,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 0, // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 0, // Hierarchical cache maintainence, MVA
                .BPMaint       = 0, // Branch predictor maintainence
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 1,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 4,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 1,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 1,  // Endian instructions
                .Except_instrs    = 1,  // Exception instructions
                .Except_AR_instrs = 1,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 2,  // Interwork instructions
                .Jazelle_instrs   = 1,  // Jazelle instructions
                .Morac_instrs     = 1   // MORAC instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 1, // MemoryHint instructions
                .MultiAccessInt_instrs = 1, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 1,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 0    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 4, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 0, // SMI instructions
                .Barrier_instrs        = 0, // Barrier instructions
                .SynchPrim_instrs_frac = 0, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 0, // M profile PSR instructions
                .SWP_frac              = 0  // Swap instructions
            },
            .SCTLR = {
                .M   = 0,           // MMU disabled
                .A   = 0,           // alignment checking disabled
                .C   = 0,           // data cache disabled
                .W   = 1,           // write buffer enabled
                .P   = 1,           // 32-bit exception handlers
                .D   = 1,           // 26-bit address exception checking disabled
                .L   = 1,           // late abort model selected
                .B   = 0,           // endianness
                .S   = 0,           // system protection bit
                .R   = 0,           // ROM protection bit
                .SW  = 0,           // SWP/SWPB enable bit
                .Z   = 0,           // branch prediction (ignored)
                .I   = 0,           // instruction cache disabled
                .V   = 0,           // high vectors
                .RR  = 0,           // replacement strategy (ignored)
                .L4  = 0,           // backwards-compatible load
                .DT  = 1,           // global data TCM enable/disable (redundant)
                .IT  = 1,           // global instruction TCM enable/disable (redundant)
                .FI  = 0,           // low latency features for fast interrupts
                .U   = 0,           // unaligned data access operations
                .XP  = 0,           // subpage AP bits disabled
                .VE  = 0,           // configure vectored interrupt
                .EE  = 0,           // CPSR E bit on taking an exception
                .NMI = 0,           // non-maskable bit (set by pin FIQISNMI)
                .TRE = 0,           // TEX Remap enable
                .AFE = 0,           // access flag enable
                .TE  = 0,           // enter Thumb mode on exception
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M   = 1,   // MMU enable
                    .A   = 1,   // alignment checking enable
                    .C   = 1,   // data cache enable
                    .Z   = 1,   // branch prediction (ignored)
                    .I   = 1,   // instruction cache enable
                    .V   = 1,   // high vector enable
                    .RR  = 1,   // replacement strategy enable
                    .L4  = 1,   // backwards-compatible load
                    .FI  = 1,   // low latency features for fast interrupts
                    .U   = 1,   // unaligned data access operations
                    .VE  = 1,   // configure vectored interrupt
                    .EE  = 1,   // CPSR E bit on taking an exception
                    .TE  = 1,   // enter Thumb mode on exception
                }
            }
        }
    },

    {
        .name            = "Cortex-A8",
        .arch            = ARM_V7,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .IFARPresent     = True,
        .advSIMDPresent  = True,
        .vfpPresent      = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 2,
                    .PartNum      = 0xc08,
                    .Architecture = 0xf,
                    .Variant      = 0x3,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .postV7 = {
                    .IMinLine  = 4,         // 32 byte line
                    .L1Ipolicy = 2,         // VIPT support
                    .DMinLine  = 4,         // 32 byte line
                    .CWG       = 2,         // 4 byte writeback granule
                    .format    = CT_POST_V7 // post-armV7
                }
            },
            .TLBTR = {
                .nU     = 1,        // not unified TLB
                .DLsize = 32,       // number of unified/data lockable entries
                .ILsize = 32        // number of instruction lockable entries
            },
            .ID_PFR0 = {
                .State0 = 1,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 1,        // Jazelle support
                .State3 = 1         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 1, // ARM programmer's model
                .SecurityExtension     = 1, // Security Extensions support
                .MicroProgrammersModel = 0  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 0, // Core debug model
                .SecureDebug   = 0, // Secure debug model
                .EmbeddedDebug = 4, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 0, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA               = 3,    // VMSA support
                .PMSA               = 0,    // PMSA support
                .OuterShareability  = 0,    // outer shareability
                .ShareabilityLevels = 0,    // shareability levels
                .TCMSupport         = 0,    // TCM support
                .AuxillaryRegisters = 1,    // ARMv6 Auxillary Control register
                .FCSESupport        = 1,    // FCSE support
                .InnerShareability  = 0     // inner shareability
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 2    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 2,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 0,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 2,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 1,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 1,     // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 1,     // Hierarchical cache maintainence, MVA
                .BPMaint       = 2,     // Branch predictor maintainence
                .Supersection  = 0,     // Supersections supported
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 1,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 0,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 0,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 1,  // Endian instructions
                .Except_instrs    = 1,  // Exception instructions
                .Except_AR_instrs = 1,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 3,  // Interwork instructions
                .Jazelle_instrs   = 1,  // Jazelle instructions
                .Morac_instrs     = 1   // MORAC instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 3, // MemoryHint instructions
                .MultiAccessInt_instrs = 0, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 2,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 1    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 4, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 1, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 0, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 0, // M profile PSR instructions
                .SWP_frac              = 0  // Swap instructions
            },
            .CCSIDR = {
                // DATA CACHE ENTRIES
                [0] = {
                    [0] = {                     // L1 data cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 1,
                        .RA            = 1,
                        .WB            = 1,
                        .WT            = 0
                    },
                },
                // INSTRUCTION CACHE ENTRIES
                [1] = {
                    [0] = {                     // L1 instruction cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 0,
                        .RA            = 1,
                        .WB            = 0,
                        .WT            = 0
                    },
                },
            },
            .CLIDR = {
                .Ctype1 = 3, // cache type field, L1 cache
                .Ctype2 = 0, // cache type field, L2 cache
                .Ctype3 = 0, // cache type field, L3 cache
                .Ctype4 = 0, // cache type field, L4 cache
                .Ctype5 = 0, // cache type field, L5 cache
                .Ctype6 = 0, // cache type field, L6 cache
                .Ctype7 = 0, // cache type field, L7 cache
                .LoUIS  = 0, // level of unification inner shareable
                .LoC    = 1, // level of coherency
                .LoUU   = 1  // level of unification uniprocessor
            },
            .AIDR = {
                .value = 0
            },
            .SCTLR = {
                .M   = 0,           // MMU disabled
                .A   = 0,           // alignment checking disabled
                .C   = 0,           // data cache disabled
                .W   = 1,           // write buffer enabled
                .P   = 1,           // 32-bit exception handlers
                .D   = 1,           // 26-bit address exception checking disabled
                .L   = 1,           // late abort model selected
                .B   = 0,           // endianness
                .S   = 0,           // system protection bit
                .R   = 0,           // ROM protection bit
                .SW  = 0,           // SWP/SWPB enable bit
                .Z   = 0,           // branch prediction (ignored)
                .I   = 0,           // instruction cache disabled
                .V   = 0,           // high vectors
                .RR  = 0,           // replacement strategy (ignored)
                .L4  = 0,           // backwards-compatible load
                .DT  = 1,           // global data TCM enable/disable (redundant)
                .IT  = 1,           // global instruction TCM enable/disable (redundant)
                .FI  = 0,           // low latency features for fast interrupts
                .U   = 1,           // unaligned data access operations
                .XP  = 1,           // subpage AP bits disabled
                .VE  = 0,           // configure vectored interrupt
                .EE  = 0,           // CPSR E bit on taking an exception
                .NMI = 0,           // non-maskable bit (set by pin FIQISNMI)
                .TRE = 0,           // TEX Remap enable
                .AFE = 0,           // access flag enable
                .TE  = 0,           // enter Thumb mode on exception
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M   = 1,   // MMU enable
                    .A   = 1,   // alignment checking enable
                    .C   = 1,   // data cache enable
                    .Z   = 1,   // branch prediction (ignored)
                    .I   = 1,   // instruction cache enable
                    .V   = 1,   // high vector enable
                    .EE  = 1,   // CPSR E bit on taking an exception
                    .TRE = 1,   // TEX Remap enable
                    .AFE = 1,   // access flag enable
                    .TE  = 1,   // enter Thumb mode on exception
                }
            },
        },

        .sdfpRegDefaults = {

            .FPSID = {
                .Revision        = 0x3,     // revision
                .Variant         = 0xc,     // variant
                .PartNumber      = 0x30,    // part number
                .Subarchitecture = 0x3,     // Null VFP sub-architecture
                .SW              = 0,       // hardware implementation
                .Implementor     = AI_ARM   // implementor is ARM
            },

            .MVFR0 = {
                .A_SIMD_Registers  = 2, // 32x64-bit media register bank
                .SinglePrecision   = 2, // single precision supported
                .DoublePrecision   = 2, // double precision supported
                .VFP_ExceptionTrap = 0, // trapped exceptions not supported
                .Divide            = 1, // VFP hardware divide supported
                .SquareRoot        = 1, // VFP hardware square root supported
                .ShortVectors      = 1, // VFP short vector supported
                .VFP_RoundingModes = 1  // all VFP rounding modes supported
            },

            .MVFR1 = {
                .FlushToZeroMode        = 1,    // VFP denormal arithmetic supported
                .DefaultNaNMode         = 1,    // VFP NaN propagation supported
                .A_SIMD_LoadStore       = 1,    // SIMD load/store instructions supported
                .A_SIMD_Integer         = 1,    // SIMD integer instructions supported
                .A_SIMD_SinglePrecision = 1,    // SIMD single-precision supported
                .A_SIMD_HalfPrecision   = 0,    // SIMD half-precision not supported
                .VFP_HalfPrecision      = 0     // VFP half-precision not supported
            },
        },
    },

    {
        .name            = "Cortex-A9UP",
        .arch            = ARM_V7,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .IFARPresent     = True,
        .PCRPresent      = True,
        .CBARPresent     = True,
        .NEONBPresent    = True,
        .TLBLDPresent    = True,
        .advSIMDPresent  = True,
        .vfpPresent      = True,
        .fpexcDexPresent = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 0,
                    .PartNum      = 0xc09,
                    .Architecture = 0xf,
                    .Variant      = 0x1,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .postV7 = {
                    .IMinLine  = 3,         // 16 byte line
                    .L1Ipolicy = 2,         // VIPT support
                    .DMinLine  = 3,         // 16 byte line
                    .CWG       = 3,         // 8 byte writeback granule
                    .format    = CT_POST_V7 // post-armV7
                }
            },
            .TLBTR = {
                .nU     = 0,        // not unified TLB
                .DLsize = 4,        // number of unified/data lockable entries
                .ILsize = 0         // number of instruction lockable entries
            },
            .ID_PFR0 = {
                .State0 = 1,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 2,        // Jazelle support, clear JOSCR.CV on exception
                .State3 = 1         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 1, // ARM programmer's model
                .SecurityExtension     = 1, // Security Extensions support
                .MicroProgrammersModel = 0  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 4, // Core debug model
                .SecureDebug   = 4, // Secure debug model
                .EmbeddedDebug = 4, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 1, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
            },
            .ID_MMFR0 = {
                .VMSA               = 3,    // VMSA support
                .PMSA               = 0,    // PMSA support
                .OuterShareability  = 1,    // outer shareability
                .ShareabilityLevels = 0,    // shareability levels
                .TCMSupport         = 0,    // TCM support
                .AuxillaryRegisters = 1,    // ARMv6 Auxillary Control register
                .FCSESupport        = 0,    // FCSE support
                .InnerShareability  = 0     // inner shareability
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 2    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 3,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 2,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 1,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 1,     // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 1,     // Hierarchical cache maintainence, MVA
                .BPMaint       = 1,     // Branch predictor maintainence
                .MaintBCast    = 2,     // Maitainence broadcast
                .CoherentWalk  = 1,     // Clean to unification point on TLB update
                .Supersection  = 0,     // Supersections supported
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 1,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 0,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 0,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 1,  // Endian instructions
                .Except_instrs    = 1,  // Exception instructions
                .Except_AR_instrs = 1,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 3,  // Interwork instructions
                .Jazelle_instrs   = 1,  // Jazelle instructions
                .Morac_instrs     = 1   // MORAC instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 4, // MemoryHint instructions
                .MultiAccessInt_instrs = 0, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 2,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 1    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 4, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 1, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 0, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 0, // M profile PSR instructions
                .SWP_frac              = 0  // Swap instructions
            },
            .CCSIDR = {
                // DATA CACHE ENTRIES
                [0] = {
                    [0] = {                     // L1 data cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 1,
                        .RA            = 1,
                        .WB            = 1,
                        .WT            = 0
                    },
                },
                // INSTRUCTION CACHE ENTRIES
                [1] = {
                    [0] = {                     // L1 instruction cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 0,
                        .RA            = 1,
                        .WB            = 0,
                        .WT            = 0
                    },
                },
            },
            .CLIDR = {
                .Ctype1 = 3, // cache type field, L1 cache
                .Ctype2 = 0, // cache type field, L2 cache
                .Ctype3 = 0, // cache type field, L3 cache
                .Ctype4 = 0, // cache type field, L4 cache
                .Ctype5 = 0, // cache type field, L5 cache
                .Ctype6 = 0, // cache type field, L6 cache
                .Ctype7 = 0, // cache type field, L7 cache
                .LoUIS  = 0, // level of unification inner shareable
                .LoC    = 1, // level of coherency
                .LoUU   = 1  // level of unification uniprocessor
            },
            .AIDR = {
                .value = 0
            },
            .SCTLR = {
                .M   = 0,           // MMU disabled
                .A   = 0,           // alignment checking disabled
                .C   = 0,           // data cache disabled
                .W   = 1,           // write buffer enabled
                .P   = 1,           // 32-bit exception handlers
                .D   = 1,           // 26-bit address exception checking disabled
                .L   = 1,           // late abort model selected
                .B   = 0,           // endianness
                .S   = 0,           // system protection bit
                .R   = 0,           // ROM protection bit
                .SW  = 0,           // SWP/SWPB enable bit
                .Z   = 0,           // branch prediction (ignored)
                .I   = 0,           // instruction cache disabled
                .V   = 0,           // high vectors
                .RR  = 0,           // replacement strategy (ignored)
                .L4  = 0,           // backwards-compatible load
                .DT  = 1,           // global data TCM enable/disable (redundant)
                .IT  = 1,           // global instruction TCM enable/disable (redundant)
                .FI  = 0,           // low latency features for fast interrupts
                .U   = 1,           // unaligned data access operations
                .XP  = 1,           // subpage AP bits disabled
                .VE  = 0,           // configure vectored interrupt
                .EE  = 0,           // CPSR E bit on taking an exception
                .NMI = 0,           // non-maskable bit (set by pin FIQISNMI)
                .TRE = 0,           // TEX Remap enable
                .AFE = 0,           // access flag enable
                .TE  = 0,           // enter Thumb mode on exception
            },
            .PCR = {
                .Latency = 2        // maximum clock latency
            }
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M   = 1,   // MMU enable
                    .A   = 1,   // alignment checking enable
                    .C   = 1,   // data cache enable
                    .SW  = 1,   // SWP/SWPB enable bit
                    .Z   = 1,   // branch prediction (ignored)
                    .I   = 1,   // instruction cache enable
                    .V   = 1,   // high vector enable
                    .EE  = 1,   // CPSR E bit on taking an exception
                    .TRE = 1,   // TEX Remap enable
                    .AFE = 1,   // access flag enable
                    .TE  = 1,   // enter Thumb mode on exception
                }
            },
        },

        .sdfpRegDefaults = {

            .FPSID = {
                .Revision        = 0x2,     // revision
                .Variant         = 0x9,     // variant
                .PartNumber      = 0x30,    // part number
                .Subarchitecture = 0x3,     // Null VFP sub-architecture
                .SW              = 0,       // hardware implementation
                .Implementor     = AI_ARM   // implementor is ARM
            },

            .MVFR0 = {
                .A_SIMD_Registers  = 2, // 32x64-bit media register bank
                .SinglePrecision   = 2, // single precision supported
                .DoublePrecision   = 2, // double precision supported
                .VFP_ExceptionTrap = 0, // trapped exceptions not supported
                .Divide            = 1, // VFP hardware divide supported
                .SquareRoot        = 1, // VFP hardware square root supported
                .ShortVectors      = 0, // VFP short vector supported
                .VFP_RoundingModes = 1  // all VFP rounding modes supported
            },

            .MVFR1 = {
                .FlushToZeroMode        = 1,    // VFP denormal arithmetic supported
                .DefaultNaNMode         = 1,    // VFP NaN propagation supported
                .A_SIMD_LoadStore       = 1,    // SIMD load/store instructions not supported
                .A_SIMD_Integer         = 1,    // SIMD integer instructions not supported
                .A_SIMD_SinglePrecision = 1,    // SIMD single-precision not supported
                .A_SIMD_HalfPrecision   = 1,    // SIMD half-precision not supported
                .VFP_HalfPrecision      = 1     // VFP half-precision supported
            },
        },
    },

    // this macro is used to define Cortex-A9MP variants
    #define CORTEX_A9_MPxN(_NAME, _NUM) {                                       \
                                                                                \
        .name            = _NAME,                                               \
        .arch            = ARM_V7,                                              \
        .ICCPMRBits      = 5,                                                   \
        .minICCBPR       = 2,                                                   \
        .fcsePresent     = True,                                                \
        .fcseRequiresMMU = True,                                                \
        .rotateUnaligned = True,                                                \
        .align64as32     = True,                                                \
        .STRoffsetPC12   = True,                                                \
        .IFARPresent     = True,                                                \
        .PCRPresent      = True,                                                \
        .CBARPresent     = True,                                                \
        .NEONBPresent    = True,                                                \
        .TLBLDPresent    = True,                                                \
        .advSIMDPresent  = True,                                                \
        .vfpPresent      = True,                                                \
        .fpexcDexPresent = True,                                                \
        .numCPUs         = _NUM,                                                \
                                                                                \
        .cpRegDefaults = {                                                      \
            .MIDR = {                                                           \
                .postArm7 = {                                                   \
                    .Revision     = 0,                                          \
                    .PartNum      = 0xc09,                                      \
                    .Architecture = 0xf,                                        \
                    .Variant      = 0x1,                                        \
                    .Implementor  = AI_ARM                                      \
                }                                                               \
            },                                                                  \
            .CTR = {                                                            \
                .postV7 = {                                                     \
                    .IMinLine  = 3,         /* 16 byte line */                  \
                    .L1Ipolicy = 2,         /* VIPT support */                  \
                    .DMinLine  = 3,         /* 16 byte line */                  \
                    .ERG       = 3,         /* 8 byte reservation granule */    \
                    .CWG       = 3,         /* 8 byte writeback granule */      \
                    .format    = CT_POST_V7 /* post-armV7 */                    \
                }                                                               \
            },                                                                  \
            .TLBTR = {                                                          \
                .nU     = 0,        /* not unified TLB */                       \
                .DLsize = 4,        /* number of unified/data lockable entries */\
                .ILsize = 0         /* number of instruction lockable entries */\
            },                                                                  \
            .ID_PFR0 = {                                                        \
                .State0 = 1,        /* 32-bit ARM instruction set support */    \
                .State1 = 3,        /* Thumb encoding support */                \
                .State2 = 2,        /* Jazelle support, clear JOSCR.CV on exception */\
                .State3 = 1         /* ThumbEE support */                       \
            },                                                                  \
            .ID_PFR1 = {                                                        \
                .ProgrammersModel      = 1, /* ARM programmer's model */        \
                .SecurityExtension     = 1, /* Security Extensions support */   \
                .MicroProgrammersModel = 0  /* Microcontroller programmer's model */\
            },                                                                  \
            .ID_DFR0 = {                                                        \
                .CoreDebug     = 4, /* Core debug model */                      \
                .SecureDebug   = 4, /* Secure debug model */                    \
                .EmbeddedDebug = 4, /* Embedded debug model */                  \
                .TraceDebugCP  = 0, /* Trace debug model, coprocessor-based */  \
                .TraceDebugMM  = 1, /* Trace debug model, memory mapped */      \
                .MicroDebug    = 0, /* Microcontroller debug model */           \
            },                                                                  \
            .ID_MMFR0 = {                                                       \
                .VMSA               = 3,    /* VMSA support */                  \
                .PMSA               = 0,    /* PMSA support */                  \
                .OuterShareability  = 1,    /* outer shareability */            \
                .ShareabilityLevels = 0,    /* shareability levels */           \
                .TCMSupport         = 0,    /* TCM support */                   \
                .AuxillaryRegisters = 1,    /* ARMv6 Auxillary Control register */\
                .FCSESupport        = 0,    /* FCSE support */                  \
                .InnerShareability  = 0     /* inner shareability */            \
            },                                                                  \
            .ID_MMFR1 = {                                                       \
                .L1VAHarvard = 0,   /* L1 maintainence by VA, Harvard */        \
                .L1VAUnified = 0,   /* L1 maintainence by VA, unified */        \
                .L1SWHarvard = 0,   /* L1 maintainence by Set/Way, Harvard */   \
                .L1SWUnified = 0,   /* L1 maintainence by Set/Way, unified */   \
                .L1Harvard   = 0,   /* L1 maintainence, Harvard */              \
                .L1Unified   = 0,   /* L1 maintainence, unified */              \
                .L1TestClean = 0,   /* L1 test and clean */                     \
                .BTB         = 2    /* Branch target buffer */                  \
            },                                                                  \
            .ID_MMFR2 = {                                                       \
                .L1FgndPrefetchHarvard  = 0,    /* L1 F/ground cache p/fetch range, Harvard */\
                .L1BgndPrefetchHarvard  = 0,    /* L1 B/ground cache p/fetch range, Harvard */\
                .L1MaintRangeHarvard    = 0,    /* L1 maintanence range, Harvard */\
                .TLBMaintHarvard        = 0,    /* TLB maintanence, Harvard */  \
                .TLBMaintUnified        = 3,    /* TLB maintanence, Unified */  \
                .MemoryBarrierCP15      = 2,    /* Memory Barrier, CP15 based */\
                .WaitForInterruptStall  = 1,    /* Wait-for-interrupt stalling */\
                .HWAccessFlag           = 0     /* hardware access flag support */\
            },                                                                  \
            .ID_MMFR3 = {                                                       \
                .HierMaintSW   = 1,     /* Hierarchical cache maintainence, set/way */\
                .HierMaintMVA  = 1,     /* Hierarchical cache maintainence, MVA */\
                .BPMaint       = 1,     /* Branch predictor maintainence */     \
                .MaintBCast    = 2,     /* Maitainence broadcast */             \
                .CoherentWalk  = 1,     /* Clean to unification point on TLB update */\
                .Supersection  = 0,     /* Supersections supported */           \
            },                                                                  \
            .ID_ISAR0 = {                                                       \
                .Swap_instrs      = 1,  /* Atomic instructions */               \
                .BitCount_instrs  = 1,  /* BitCount instructions */             \
                .BitField_instrs  = 1,  /* BitField instructions */             \
                .CmpBranch_instrs = 1,  /* CmpBranch instructions */            \
                .Coproc_instrs    = 0,  /* Coprocessor instructions */          \
                .Debug_instrs     = 1,  /* Debug instructions */                \
                .Divide_instrs    = 0,  /* Divide instructions */               \
            },                                                                  \
            .ID_ISAR1 = {                                                       \
                .Endian_instrs    = 1,  /* Endian instructions */               \
                .Except_instrs    = 1,  /* Exception instructions */            \
                .Except_AR_instrs = 1,  /* A/R profile exception instructions */\
                .Extend_instrs    = 2,  /* Extend instructions */               \
                .IfThen_instrs    = 1,  /* IfThen instructions */               \
                .Immediate_instrs = 1,  /* Immediate instructions */            \
                .Interwork_instrs = 3,  /* Interwork instructions */            \
                .Jazelle_instrs   = 1,  /* Jazelle instructions */              \
                .Morac_instrs     = 1   /* MORAC Instructions */                \
            },                                                                  \
            .ID_ISAR2 = {                                                       \
                .LoadStore_instrs      = 1, /* LoadStore instructions */        \
                .MemHint_instrs        = 4, /* MemoryHint instructions */       \
                .MultiAccessInt_instrs = 0, /* Multi-access interruptible instructions */\
                .Mult_instrs           = 2, /* Multiply instructions */         \
                .MultS_instrs          = 3, /* Multiply instructions, advanced signed */\
                .MultU_instrs          = 2, /* Multiply instructions, advanced unsigned */\
                .PSR_AR_instrs         = 1, /* A/R profile PSR instructions */  \
                .Reversal_instrs       = 2  /* Reversal instructions */         \
            },                                                                  \
            .ID_ISAR3 = {                                                       \
                .Saturate_instrs     = 1,   /* Saturate instructions */         \
                .SIMD_instrs         = 3,   /* SIMD instructions */             \
                .SVC_instrs          = 1,   /* SVC instructions */              \
                .SynchPrim_instrs    = 2,   /* SynchPrim instructions */        \
                .TabBranch_instrs    = 1,   /* TableBranch instructions */      \
                .ThumbCopy_instrs    = 1,   /* ThumbCopy instructions */        \
                .TrueNOP_instrs      = 1,   /* TrueNOP instructions */          \
                .T2ExeEnvExtn_instrs = 1    /* Thumb-2 Execution env extensions */\
            },                                                                  \
            .ID_ISAR4 = {                                                       \
                .Unpriv_instrs         = 2, /* Unprivileged instructions */     \
                .WithShifts_instrs     = 4, /* Shift instructions */            \
                .Writeback_instrs      = 1, /* Writeback instructions */        \
                .SMI_instrs            = 1, /* SMI instructions */              \
                .Barrier_instrs        = 1, /* Barrier instructions */          \
                .SynchPrim_instrs_frac = 0, /* Fractional support for sync primitive instructions */\
                .PSR_M_instrs          = 0, /* M profile PSR instructions */    \
                .SWP_frac              = 0  /* Swap instructions */             \
            },                                                                  \
            .CCSIDR = {                                                         \
                /* DATA CACHE ENTRIES */                                        \
                [0] = {                                                         \
                    [0] = {                     /* L1 data cache */             \
                        .LineSize      = 1,                                     \
                        .Associativity = 3,                                     \
                        .NumSets       = 255,                                   \
                        .WA            = 1,                                     \
                        .RA            = 1,                                     \
                        .WB            = 1,                                     \
                        .WT            = 0                                      \
                    },                                                          \
                },                                                              \
                /* INSTRUCTION CACHE ENTRIES */                                 \
                [1] = {                                                         \
                    [0] = {                     /* L1 instruction cache */      \
                        .LineSize      = 1,                                     \
                        .Associativity = 3,                                     \
                        .NumSets       = 255,                                   \
                        .WA            = 0,                                     \
                        .RA            = 1,                                     \
                        .WB            = 0,                                     \
                        .WT            = 0                                      \
                    },                                                          \
                },                                                              \
            },                                                                  \
            .CLIDR = {                                                          \
                .Ctype1 = 3, /* cache type field, L1 cache */                   \
                .Ctype2 = 0, /* cache type field, L2 cache */                   \
                .Ctype3 = 0, /* cache type field, L3 cache */                   \
                .Ctype4 = 0, /* cache type field, L4 cache */                   \
                .Ctype5 = 0, /* cache type field, L5 cache */                   \
                .Ctype6 = 0, /* cache type field, L6 cache */                   \
                .Ctype7 = 0, /* cache type field, L7 cache */                   \
                .LoUIS  = 1, /* level of unification inner shareable */         \
                .LoC    = 1, /* level of coherency */                           \
                .LoUU   = 1  /* level of unification uniprocessor */            \
            },                                                                  \
            .AIDR = {                                                           \
                .value = 0                                                      \
            },                                                                  \
            .SCTLR = {                                                          \
                .M   = 0,           /* MMU disabled */                          \
                .A   = 0,           /* alignment checking disabled */           \
                .C   = 0,           /* data cache disabled */                   \
                .W   = 1,           /* write buffer enabled */                  \
                .P   = 1,           /* 32-bit exception handlers */             \
                .D   = 1,           /* 26-bit address exception checking disabled */\
                .L   = 1,           /* late abort model selected */             \
                .B   = 0,           /* endianness */                            \
                .S   = 0,           /* system protection bit */                 \
                .R   = 0,           /* ROM protection bit */                    \
                .SW  = 0,           /* SWP/SWPB enable bit */                   \
                .Z   = 0,           /* branch prediction (ignored) */           \
                .I   = 0,           /* instruction cache disabled */            \
                .V   = 0,           /* high vectors */                          \
                .RR  = 0,           /* replacement strategy (ignored) */        \
                .L4  = 0,           /* backwards-compatible load */             \
                .DT  = 1,           /* global data TCM enable/disable (redundant) */\
                .IT  = 1,           /* global instruction TCM enable/disable (redundant) */\
                .FI  = 0,           /* low latency features for fast interrupts */\
                .U   = 1,           /* unaligned data access operations */      \
                .XP  = 1,           /* subpage AP bits disabled */              \
                .VE  = 0,           /* configure vectored interrupt */          \
                .EE  = 0,           /* CPSR E bit on taking an exception */     \
                .NMI = 0,           /* non-maskable bit (set by pin FIQISNMI) */\
                .TRE = 0,           /* TEX Remap enable */                      \
                .AFE = 0,           /* access flag enable */                    \
                .TE  = 0,           /* enter Thumb mode on exception */         \
            },                                                                  \
            .PCR = {                                                            \
                .Latency = 2        /* maximum clock latency */                 \
            },                                                                  \
            .CBAR = {                                                           \
                .value = 0x13080000 /* base address */                          \
            }                                                                   \
        },                                                                      \
                                                                                \
        .cpRegMasks = {                                                         \
            .SCTLR = {                                                          \
                .fields = {                                                     \
                    .M   = 1,   /* MMU enable */                                \
                    .A   = 1,   /* alignment checking enable */                 \
                    .C   = 1,   /* data cache enable */                         \
                    .SW  = 1,   /* SWP/SWPB enable bit */                       \
                    .Z   = 1,   /* branch prediction (ignored) */               \
                    .I   = 1,   /* instruction cache enable */                  \
                    .V   = 1,   /* high vector enable */                        \
                    .EE  = 1,   /* CPSR E bit on taking an exception */         \
                    .TRE = 1,   /* TEX Remap enable */                          \
                    .AFE = 1,   /* access flag enable */                        \
                    .TE  = 1,   /* enter Thumb mode on exception */             \
                }                                                               \
            },                                                                  \
        },                                                                      \
                                                                                \
        .sdfpRegDefaults = {                                                    \
                                                                                \
            .FPSID = {                                                          \
                .Revision        = 0x2,     /* revision */                      \
                .Variant         = 0x9,     /* variant */                       \
                .PartNumber      = 0x30,    /* part number */                   \
                .Subarchitecture = 0x3,     /* Null VFP sub-architecture */     \
                .SW              = 0,       /* hardware implementation */       \
                .Implementor     = AI_ARM   /* implementor is ARM */            \
            },                                                                  \
                                                                                \
            .MVFR0 = {                                                          \
                .A_SIMD_Registers  = 2, /* 32x64-bit media register bank */     \
                .SinglePrecision   = 2, /* single precision supported */        \
                .DoublePrecision   = 2, /* double precision supported */        \
                .VFP_ExceptionTrap = 0, /* trapped exceptions not supported */  \
                .Divide            = 1, /* VFP hardware divide supported */     \
                .SquareRoot        = 1, /* VFP hardware square root supported */\
                .ShortVectors      = 0, /* VFP short vector supported */        \
                .VFP_RoundingModes = 1  /* all VFP rounding modes supported */  \
            },                                                                  \
                                                                                \
            .MVFR1 = {                                                          \
                .FlushToZeroMode        = 1,    /* VFP denormal arithmetic supported */\
                .DefaultNaNMode         = 1,    /* VFP NaN propagation supported */\
                .A_SIMD_LoadStore       = 1,    /* SIMD load/store instructions not supported */\
                .A_SIMD_Integer         = 1,    /* SIMD integer instructions not supported */\
                .A_SIMD_SinglePrecision = 1,    /* SIMD single-precision not supported */\
                .A_SIMD_HalfPrecision   = 1,    /* SIMD half-precision not supported */\
                .VFP_HalfPrecision      = 1     /* VFP half-precision supported */\
            },                                                                  \
        },                                                                      \
                                                                                \
        .mpRegDefaults = {                                                      \
                                                                                \
            .SCUConfiguration = {                                               \
                .Tag_RAM_Sizes = 0x55,  /* tag RAM sizes for each CPU */        \
            },                                                                  \
                                                                                \
            .ICDICTR = {                                                        \
                .ITLines      = 2,      /* 64 external interrupt lines */       \
                .SecurityExtn = 1,      /* always 1 */                          \
                .LSPI         = 31,     /* 31 LSPIs */                          \
            },                                                                  \
                                                                                \
            .ICDIIDR = {                                                        \
                .Implementer = 0x43b,   /* implementor */                       \
                .Revision    = 0x020,   /* revision */                          \
                .Version     = 0x01     /* version */                           \
            },                                                                  \
                                                                                \
            .PERIPH_ID = {                                                      \
                [0] = 0x04,             /* periph_id_0 */                       \
                [1] = 0x00,             /* periph_id_1 */                       \
                [2] = 0x00,             /* periph_id_2 */                       \
                [3] = 0x00,             /* periph_id_3 */                       \
                [4] = 0x90,             /* periph_id_4 */                       \
                [5] = 0xb3,             /* periph_id_5 */                       \
                [6] = 0x1b,             /* periph_id_6 */                       \
                [7] = 0x00              /* periph_id_7 */                       \
            },                                                                  \
                                                                                \
            .COMPONENT_ID = {                                                   \
                [0] = 0x0d,             /* component_id_0 */                    \
                [1] = 0xf0,             /* component_id_1 */                    \
                [2] = 0x05,             /* component_id_2 */                    \
                [3] = 0xb1              /* component_id_3 */                    \
            },                                                                  \
                                                                                \
            .ICCIDR = {                                                         \
                .Implementer  = 0x43b,  /* implementer (ARM) */                 \
                .Revision     = 2,      /* revision 2 */                        \
                .Architecture = 1,      /* architecture (GICv1) */              \
                .Part         = 0x390,  /* part number */                       \
            },                                                                  \
                                                                                \
            .PERIPH_CLK = 2,            /* peripheral clock factor (minimum 2) */ \
        },                                                                      \
    }

    // these are supported Cortex-A9MP variants
    CORTEX_A9_MPxN("Cortex-A9MP",   4),
    CORTEX_A9_MPxN("Cortex-A9MPx1", 1),
    CORTEX_A9_MPxN("Cortex-A9MPx2", 2),
    CORTEX_A9_MPxN("Cortex-A9MPx3", 3),
    CORTEX_A9_MPxN("Cortex-A9MPx4", 4),

    #undef CORTEX_A9_MPxN

    {
        .name            = "Cortex-A5UP",
        .arch            = ARM_V7,
        .fcsePresent     = True,
        .fcseRequiresMMU = True,
        .rotateUnaligned = True,
        .align64as32     = True,
        .STRoffsetPC12   = True,
        .IFARPresent     = True,
        .PCRPresent      = False,
        .CBARPresent     = True,
        .TLBLDPresent    = False,
        .TLBHRPresent    = True,
        .advSIMDPresent  = True,
        .vfpPresent      = True,
        .fpexcDexPresent = True,

        .cpRegDefaults = {
            .MIDR = {
                .postArm7 = {
                    .Revision     = 1,
                    .PartNum      = 0xc05,
                    .Architecture = 0xf,
                    .Variant      = 0x0,
                    .Implementor  = AI_ARM
                }
            },
            .CTR = {
                .postV7 = {
                    .IMinLine  = 3,         // 16 byte line
                    .L1Ipolicy = 2,         // VIPT support
                    .DMinLine  = 3,         // 16 byte line
                    .CWG       = 3,         // 8 byte writeback granule
                    .format    = CT_POST_V7 // post-armV7
                }
            },
            .ID_PFR0 = {
                .State0 = 1,        // 32-bit ARM instruction set support
                .State1 = 3,        // Thumb encoding support
                .State2 = 2,        // Jazelle support, clear JOSCR.CV on exception
                .State3 = 1         // ThumbEE support
            },
            .ID_PFR1 = {
                .ProgrammersModel      = 1, // ARM programmer's model
                .SecurityExtension     = 1, // Security Extensions support
                .MicroProgrammersModel = 0  // Microcontroller programmer's model
            },
            .ID_DFR0 = {
                .CoreDebug     = 4, // Core debug model
                .SecureDebug   = 4, // Secure debug model
                .EmbeddedDebug = 4, // Embedded debug model
                .TraceDebugCP  = 0, // Trace debug model, coprocessor-based
                .TraceDebugMM  = 1, // Trace debug model, memory mapped
                .MicroDebug    = 0, // Microcontroller debug model
                // .PerfMonitors = 1, // Performance Monitors Model - to be added for A5?
            },
            .ID_MMFR0 = {
                .VMSA               = 3,    // VMSA support
                .PMSA               = 0,    // PMSA support
                .OuterShareability  = 0,    // outer shareability
                .ShareabilityLevels = 0,    // shareability levels
                .TCMSupport         = 0,    // TCM support
                .AuxillaryRegisters = 1,    // ARMv6 Auxillary Control register
                .FCSESupport        = 0,    // FCSE support
                .InnerShareability  = 0     // inner shareability
            },
            .ID_MMFR1 = {
                .L1VAHarvard = 0,   // L1 maintainence by VA, Harvard
                .L1VAUnified = 0,   // L1 maintainence by VA, unified
                .L1SWHarvard = 0,   // L1 maintainence by Set/Way, Harvard
                .L1SWUnified = 0,   // L1 maintainence by Set/Way, unified
                .L1Harvard   = 0,   // L1 maintainence, Harvard
                .L1Unified   = 0,   // L1 maintainence, unified
                .L1TestClean = 0,   // L1 test and clean
                .BTB         = 4    // Branch target buffer
            },
            .ID_MMFR2 = {
                .L1FgndPrefetchHarvard  = 0,    // L1 F/ground cache p/fetch range, Harvard
                .L1BgndPrefetchHarvard  = 0,    // L1 B/ground cache p/fetch range, Harvard
                .L1MaintRangeHarvard    = 0,    // L1 maintanence range, Harvard
                .TLBMaintHarvard        = 0,    // TLB maintanence, Harvard
                .TLBMaintUnified        = 3,    // TLB maintanence, Unified
                .MemoryBarrierCP15      = 2,    // Memory Barrier, CP15 based
                .WaitForInterruptStall  = 1,    // Wait-for-interrupt stalling
                .HWAccessFlag           = 0     // hardware access flag support
            },
            .ID_MMFR3 = {
                .HierMaintSW   = 1,     // Hierarchical cache maintainence, set/way
                .HierMaintMVA  = 1,     // Hierarchical cache maintainence, MVA
                .BPMaint       = 1,     // Branch predictor maintainence
                .MaintBCast    = 2,     // Maitainence broadcast
                .CoherentWalk  = 1,     // Clean to unification point on TLB update
                .Supersection  = 0,     // Supersections supported
            },
            .ID_ISAR0 = {
                .Swap_instrs      = 1,  // Atomic instructions
                .BitCount_instrs  = 1,  // BitCount instructions
                .BitField_instrs  = 1,  // BitField instructions
                .CmpBranch_instrs = 1,  // CmpBranch instructions
                .Coproc_instrs    = 0,  // Coprocessor instructions
                .Debug_instrs     = 1,  // Debug instructions
                .Divide_instrs    = 0,  // Divide instructions
            },
            .ID_ISAR1 = {
                .Endian_instrs    = 1,  // Endian instructions
                .Except_instrs    = 1,  // Exception instructions
                .Except_AR_instrs = 1,  // A/R profile exception instructions
                .Extend_instrs    = 2,  // Extend instructions
                .IfThen_instrs    = 1,  // IfThen instructions
                .Immediate_instrs = 1,  // Immediate instructions
                .Interwork_instrs = 3,  // Interwork instructions
                .Jazelle_instrs   = 1,  // Jazelle instructions
                .Morac_instrs     = 1   // MORAC instructions
            },
            .ID_ISAR2 = {
                .LoadStore_instrs      = 1, // LoadStore instructions
                .MemHint_instrs        = 4, // MemoryHint instructions
                .MultiAccessInt_instrs = 0, // Multi-access interruptible instructions
                .Mult_instrs           = 2, // Multiply instructions
                .MultS_instrs          = 3, // Multiply instructions, advanced signed
                .MultU_instrs          = 2, // Multiply instructions, advanced unsigned
                .PSR_AR_instrs         = 1, // A/R profile PSR instructions
                .Reversal_instrs       = 2  // Reversal instructions
            },
            .ID_ISAR3 = {
                .Saturate_instrs     = 1,   // Saturate instructions
                .SIMD_instrs         = 3,   // SIMD instructions
                .SVC_instrs          = 1,   // SVC instructions
                .SynchPrim_instrs    = 2,   // SynchPrim instructions
                .TabBranch_instrs    = 1,   // TableBranch instructions
                .ThumbCopy_instrs    = 1,   // ThumbCopy instructions
                .TrueNOP_instrs      = 1,   // TrueNOP instructions
                .T2ExeEnvExtn_instrs = 1    // Thumb-2 Execution env extensions
            },
            .ID_ISAR4 = {
                .Unpriv_instrs         = 2, // Unprivileged instructions
                .WithShifts_instrs     = 4, // Shift instructions
                .Writeback_instrs      = 1, // Writeback instructions
                .SMI_instrs            = 1, // SMI instructions
                .Barrier_instrs        = 1, // Barrier instructions
                .SynchPrim_instrs_frac = 0, // Fractional support for sync primitive instructions
                .PSR_M_instrs          = 0, // M profile PSR instructions
                .SWP_frac              = 0  // Swap instructions
            },
            .CCSIDR = {
                // DATA CACHE ENTRIES
                [0] = {
                    [0] = {                     // L1 data cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 1,
                        .RA            = 1,
                        .WB            = 1,
                        .WT            = 0
                    },
                },
                // INSTRUCTION CACHE ENTRIES
                [1] = {
                    [0] = {                     // L1 instruction cache
                        .LineSize      = 1,
                        .Associativity = 3,
                        .NumSets       = 255,
                        .WA            = 0,
                        .RA            = 1,
                        .WB            = 0,
                        .WT            = 0
                    },
                },
            },
            .CLIDR = {
                .Ctype1 = 3, // cache type field, L1 cache
                .Ctype2 = 0, // cache type field, L2 cache
                .Ctype3 = 0, // cache type field, L3 cache
                .Ctype4 = 0, // cache type field, L4 cache
                .Ctype5 = 0, // cache type field, L5 cache
                .Ctype6 = 0, // cache type field, L6 cache
                .Ctype7 = 0, // cache type field, L7 cache
                .LoUIS  = 1, // level of unification inner shareable
                .LoC    = 1, // level of coherency
                .LoUU   = 1  // level of unification uniprocessor
            },
            .AIDR = {
                .value = 0
            },
            .SCTLR = {
                .M   = 0,           // MMU disabled
                .A   = 0,           // alignment checking disabled
                .C   = 0,           // data cache disabled
                .W   = 1,           // write buffer enabled
                .P   = 1,           // 32-bit exception handlers
                .D   = 1,           // 26-bit address exception checking disabled
                .L   = 1,           // late abort model selected
                .B   = 0,           // endianness
                .S   = 0,           // system protection bit
                .R   = 0,           // ROM protection bit
                .SW  = 0,           // SWP/SWPB enable bit
                .Z   = 0,           // branch prediction (ignored)
                .I   = 0,           // instruction cache disabled
                .V   = 0,           // high vectors
                .RR  = 0,           // replacement strategy (ignored)
                .L4  = 0,           // backwards-compatible load
                .DT  = 1,           // global data TCM enable/disable (redundant)
                .HA  = 0,           // ??
                .IT  = 1,           // global instruction TCM enable/disable (redundant)
                .DZ  = 0,           // ??
                                    // Unused bit
                .FI  = 0,           // low latency features for fast interrupts
                .U   = 1,           // unaligned data access operations
                .XP  = 1,           // subpage AP bits disabled
                .VE  = 0,           // configure vectored interrupt
                .EE  = 0,           // CPSR E bit on taking an exception
                .NMI = 0,           // non-maskable bit (set by pin FIQISNMI)
                .TRE = 0,           // TEX Remap enable
                .AFE = 0,           // access flag enable
                .TE  = 0,           // enter Thumb mode on exception
            },
        },

        .cpRegMasks = {
            .SCTLR = {
                .fields = {
                    .M   = 1,   // MMU enable
                    .A   = 1,   // alignment checking enable
                    .C   = 1,   // data cache enable
                    .SW  = 1,   // SWP/SWPB enable bit
                    .I   = 1,   // instruction cache enable
                    .V   = 1,   // high vector enable
                    .EE  = 1,   // CPSR E bit on taking an exception
                    .TRE = 1,   // TEX Remap enable
                    .AFE = 1,   // access flag enable
                    .TE  = 1,   // enter Thumb mode on exception
                }
            },
        },

        .sdfpRegDefaults = {

            .FPSID = {
                .Revision        = 0x1,     // revision
                .Variant         = 0x5,     // variant
                .PartNumber      = 0x30,    // part number
                .Subarchitecture = 0x2,     // Null VFP sub-architecture
                .SW              = 0,       // hardware implementation
                .Implementor     = AI_ARM   // implementor is ARM
            },

            .MVFR0 = {
                .A_SIMD_Registers  = 2, // 32x64-bit media register bank
                .SinglePrecision   = 2, // single precision supported
                .DoublePrecision   = 2, // double precision supported
                .VFP_ExceptionTrap = 0, // trapped exceptions not supported
                .Divide            = 1, // VFP hardware divide supported
                .SquareRoot        = 1, // VFP hardware square root supported
                .ShortVectors      = 0, // VFP short vector supported
                .VFP_RoundingModes = 1  // all VFP rounding modes supported
            },

            .MVFR1 = {
                .FlushToZeroMode        = 1,    // VFP denormal arithmetic supported
                .DefaultNaNMode         = 1,    // VFP NaN propagation supported
                .A_SIMD_LoadStore       = 1,    // SIMD load/store instructions not supported
                .A_SIMD_Integer         = 1,    // SIMD integer instructions not supported
                .A_SIMD_SinglePrecision = 1,    // SIMD single-precision not supported
                .A_SIMD_HalfPrecision   = 1,    // SIMD half-precision not supported
                .VFP_HalfPrecision      = 1,    // VFP half-precision supported
                .VFP_FusedMAC           = 1,    // FMA Instructions (implies VFPv4 and SIMDv2)
            },
        },
    },

    // null terminator
    {0}
};

