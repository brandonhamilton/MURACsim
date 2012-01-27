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

#ifndef ARM_MORPH_FUNCTIONS_H
#define ARM_MORPH_FUNCTIONS_H

// model header files
#include "armMode.h"
#include "armMorph.h"

// unop instructions
ARM_MORPH_FN(armEmitUnopI);
ARM_MORPH_FN(armEmitUnopReg);
ARM_MORPH_FN(armEmitUnopRSI);
ARM_MORPH_FN(armEmitUnopRSR);
ARM_MORPH_FN(armEmitUnopRSRT);
ARM_MORPH_FN(armEmitUnopRX);

// binop instructions
ARM_MORPH_FN(armEmitBinopI);
ARM_MORPH_FN(armEmitBinopR);
ARM_MORPH_FN(armEmitBinopRT);
ARM_MORPH_FN(armEmitBinopIT);
ARM_MORPH_FN(armEmitBinopADR);
ARM_MORPH_FN(armEmitBinopRSI);
ARM_MORPH_FN(armEmitBinopRSR);
ARM_MORPH_FN(armEmitBinopRX);

// cmpop instructions
ARM_MORPH_FN(armEmitCmpopI);
ARM_MORPH_FN(armEmitCmpopR);
ARM_MORPH_FN(armEmitCmpopRSI);
ARM_MORPH_FN(armEmitCmpopRSR);
ARM_MORPH_FN(armEmitCmpopRX);

// multiply instructions
ARM_MORPH_FN(armEmitMUL);
ARM_MORPH_FN(armEmitMLA);
ARM_MORPH_FN(armEmitMLS);
ARM_MORPH_FN(armEmitDIV);
ARM_MORPH_FN(armEmitMULL);
ARM_MORPH_FN(armEmitMLAL);

// branch instructions
ARM_MORPH_FN(armEmitBranchC);
ARM_MORPH_FN(armEmitBranchR);
ARM_MORPH_FN(armEmitBLX);

// MORAC instructions
ARM_MORPH_FN(armEmitBAA);

// 16-bit branch instructions
ARM_MORPH_FN(armEmitBL_H10);
ARM_MORPH_FN(armEmitBL_H11);
ARM_MORPH_FN(armEmitBL_H01);

// ThumbEE instructions
ARM_MORPH_FN(armEmitUND_EE);
ARM_MORPH_FN(armEmitHB);
ARM_MORPH_FN(armEmitHBP);
ARM_MORPH_FN(armEmitCHKA);
ARM_MORPH_FN(armEmitENTERX);
ARM_MORPH_FN(armEmitLEAVEX);

// miscellaneous instructions
ARM_MORPH_FN(armEmitCLZ);
ARM_MORPH_FN(armEmitBKPT);
ARM_MORPH_FN(armEmitSWI);

// load and store instructions
ARM_MORPH_FN(armEmitLDRI);
ARM_MORPH_FN(armEmitLDRR);
ARM_MORPH_FN(armEmitLDRRSI);
ARM_MORPH_FN(armEmitLDRRX);
ARM_MORPH_FN(armEmitLDM1);
ARM_MORPH_FN(armEmitLDM2);
ARM_MORPH_FN(armEmitLDM3);
ARM_MORPH_FN(armEmitSTRI);
ARM_MORPH_FN(armEmitSTRR);
ARM_MORPH_FN(armEmitSTRRSI);
ARM_MORPH_FN(armEmitSTRRX);
ARM_MORPH_FN(armEmitSTM1);
ARM_MORPH_FN(armEmitSTM2);

// semaphore instructions
ARM_MORPH_FN(armEmitSWP);

// coprocessor instructions
ARM_MORPH_FN(armEmitCDP);
ARM_MORPH_FN(armEmitLDC);
ARM_MORPH_FN(armEmitMRC);
ARM_MORPH_FN(armEmitMCR);
ARM_MORPH_FN(armEmitMRRC);
ARM_MORPH_FN(armEmitMCRR);
ARM_MORPH_FN(armEmitSTC);

// status register access instructions
ARM_MORPH_FN(armEmitMRSC);
ARM_MORPH_FN(armEmitMRSS);
ARM_MORPH_FN(armEmitMSRCI);
ARM_MORPH_FN(armEmitMSRCR);
ARM_MORPH_FN(armEmitMSRSI);
ARM_MORPH_FN(armEmitMSRSR);

// hint instructions
ARM_MORPH_FN(armEmitNOP);
ARM_MORPH_FN(armEmitWFE);
ARM_MORPH_FN(armEmitWFI);
ARM_MORPH_FN(armEmitSEV);

// DSP data processing instructions
ARM_MORPH_FN(armEmitQADD);
ARM_MORPH_FN(armEmitQSUB);
ARM_MORPH_FN(armEmitQDADD);
ARM_MORPH_FN(armEmitQDSUB);

// DSP multiply instructions
ARM_MORPH_FN(armEmitSMLABB);
ARM_MORPH_FN(armEmitSMLABT);
ARM_MORPH_FN(armEmitSMLATB);
ARM_MORPH_FN(armEmitSMLATT);
ARM_MORPH_FN(armEmitSMLALBB);
ARM_MORPH_FN(armEmitSMLALBT);
ARM_MORPH_FN(armEmitSMLALTB);
ARM_MORPH_FN(armEmitSMLALTT);
ARM_MORPH_FN(armEmitSMLAWB);
ARM_MORPH_FN(armEmitSMLAWT);
ARM_MORPH_FN(armEmitSMULBB);
ARM_MORPH_FN(armEmitSMULBT);
ARM_MORPH_FN(armEmitSMULTB);
ARM_MORPH_FN(armEmitSMULTT);
ARM_MORPH_FN(armEmitSMULWB);
ARM_MORPH_FN(armEmitSMULWT);

// move instructions
ARM_MORPH_FN(armEmitMOVW);
ARM_MORPH_FN(armEmitMOVT);

// multiply instructions
ARM_MORPH_FN(armEmitMAAL);

// synchronization instructions
ARM_MORPH_FN(armEmitLDREX);
ARM_MORPH_FN(armEmitSTREX);
ARM_MORPH_FN(armEmitCLREX);

// exception instructions
ARM_MORPH_FN(armEmitSRS);
ARM_MORPH_FN(armEmitRFE);

// miscellaneous instructions
ARM_MORPH_FN(armEmitSETEND);
ARM_MORPH_FN(armEmitCPS);

// branch instructions
ARM_MORPH_FN(armEmitCBZ);
ARM_MORPH_FN(armEmitTB);

// basic media instructions
ARM_MORPH_FN(armEmitUSAD8);
ARM_MORPH_FN(armEmitUSADA8);
ARM_MORPH_FN(armEmitSBFX);
ARM_MORPH_FN(armEmitBFC);
ARM_MORPH_FN(armEmitBFI);
ARM_MORPH_FN(armEmitUBFX);

// parallel add/subtract instructions
ARM_MORPH_FN(armEmitParallelBinop8);
ARM_MORPH_FN(armEmitParallelBinop16);

// packing, unpacking, saturation and reversal instructions
ARM_MORPH_FN(armEmitPKHBT);
ARM_MORPH_FN(armEmitPKHTB);
ARM_MORPH_FN(armEmitSSAT);
ARM_MORPH_FN(armEmitSSAT16);
ARM_MORPH_FN(armEmitUSAT);
ARM_MORPH_FN(armEmitUSAT16);
ARM_MORPH_FN(armEmitSXTAB);
ARM_MORPH_FN(armEmitUXTAB);
ARM_MORPH_FN(armEmitSXTAB16);
ARM_MORPH_FN(armEmitUXTAB16);
ARM_MORPH_FN(armEmitSXTAH);
ARM_MORPH_FN(armEmitUXTAH);
ARM_MORPH_FN(armEmitSXTB);
ARM_MORPH_FN(armEmitUXTB);
ARM_MORPH_FN(armEmitSXTB16);
ARM_MORPH_FN(armEmitUXTB16);
ARM_MORPH_FN(armEmitSXTH);
ARM_MORPH_FN(armEmitUXTH);
ARM_MORPH_FN(armEmitSEL);
ARM_MORPH_FN(armEmitREV);
ARM_MORPH_FN(armEmitREV16);
ARM_MORPH_FN(armEmitRBIT);
ARM_MORPH_FN(armEmitREVSH);

// signed multiply instructions
ARM_MORPH_FN(armEmitSMLXD);
ARM_MORPH_FN(armEmitSMUXD);
ARM_MORPH_FN(armEmitSMLXLD);
ARM_MORPH_FN(armEmitSMMLX);

// SIMD/VFP data processing instructions - Misc
ARM_MORPH_FN(armEmitVEXT);
ARM_MORPH_FN(armEmitVTBL);
ARM_MORPH_FN(armEmitVTBX);
ARM_MORPH_FN(armEmitVDUPZ);

// SIMD data processing instructions - 3 regs same length
ARM_MORPH_FN(armEmitVBinop);
ARM_MORPH_FN(armEmitVPairOp);
ARM_MORPH_FN(armEmitVQDMul);
ARM_MORPH_FN(armEmitVPADD);
ARM_MORPH_FN(armEmitVBIF);
ARM_MORPH_FN(armEmitVBIT);
ARM_MORPH_FN(armEmitVBSL);
ARM_MORPH_FN(armEmitVTst);
ARM_MORPH_FN(armEmitVCmpBool);
ARM_MORPH_FN(armEmitVCmp0);
ARM_MORPH_FN(armEmitVCmpReg);
ARM_MORPH_FN(armEmitVPCmpReg);
ARM_MORPH_FN(armEmitVAbsDiff);
ARM_MORPH_FN(armEmitVMUL_P);
ARM_MORPH_FN(armEmitVMulAcc);
ARM_MORPH_FN(armEmitVBinop_F);
ARM_MORPH_FN(armEmitVTernop_F);
ARM_MORPH_FN(armEmitVABD_F);
ARM_MORPH_FN(armEmitVPairOp_F);
ARM_MORPH_FN(armEmitVCmpBool_F);
ARM_MORPH_FN(armEmitVAbsCmp_F);
ARM_MORPH_FN(armEmitVCmpReg_F);
ARM_MORPH_FN(armEmitVPCmpReg_F);

// SIMD data processing instructions - 3 regs different lengths
ARM_MORPH_FN(armEmitVABALU);
ARM_MORPH_FN(armEmitVABALS);
ARM_MORPH_FN(armEmitVABDLU);
ARM_MORPH_FN(armEmitVABDLS);
ARM_MORPH_FN(armEmitVMULL_P);
ARM_MORPH_FN(armEmitVQDMultiply);
ARM_MORPH_FN(armEmitVShiftReg);

// SIMD data processing instructions - Two Registers and a scalar
ARM_MORPH_FN(armEmitVBinopZ);
ARM_MORPH_FN(armEmitVMulAccZ);
ARM_MORPH_FN(armEmitVQDMulZ);
ARM_MORPH_FN(armEmitVBinopZ_F);
ARM_MORPH_FN(armEmitVTernopZ_F);

// SIMD data processing instructions - Two Registers and a shift amount
ARM_MORPH_FN(armEmitVShiftImm);
ARM_MORPH_FN(armEmitVShiftIns);

// SIMD data processing instructions - Two registers, miscellaneous
ARM_MORPH_FN(armEmitVREV);
ARM_MORPH_FN(armEmitVUnop);
ARM_MORPH_FN(armEmitVUnopReg);
ARM_MORPH_FN(armEmitVCLS);
ARM_MORPH_FN(armEmitVCNT);
ARM_MORPH_FN(armEmitVSWP);
ARM_MORPH_FN(armEmitVZIP);
ARM_MORPH_FN(armEmitVUZP);
ARM_MORPH_FN(armEmitVTRN);
ARM_MORPH_FN(armEmitVShiftMax);
ARM_MORPH_FN(armEmitVCmp0_F);
ARM_MORPH_FN(armEmitVABS_F);
ARM_MORPH_FN(armEmitVNEG_F);
ARM_MORPH_FN(armEmitVRECPE_F);
ARM_MORPH_FN(armEmitVRECPE_U);
ARM_MORPH_FN(armEmitVRSQRTE_F);
ARM_MORPH_FN(armEmitVRSQRTE_U);
ARM_MORPH_FN(armEmitVRECPS_F);
ARM_MORPH_FN(armEmitVRSQRTS_F);
ARM_MORPH_FN(armEmitVCVT_XF_SIMD);
ARM_MORPH_FN(armEmitVCVT_FX_SIMD);
ARM_MORPH_FN(armEmitVCVT_FH_SIMD);
ARM_MORPH_FN(armEmitVCVT_HF_SIMD);

// SIMD data processing instructions - One Register and a modified immediate
ARM_MORPH_FN(armEmitVOpRI);

// VFP data processing instructions
ARM_MORPH_FN(armEmitVFPUnop);
ARM_MORPH_FN(armEmitVFPBinop);
ARM_MORPH_FN(armEmitVMulAcc_VFP);
ARM_MORPH_FN(armEmitVMOVI_VFP);
ARM_MORPH_FN(armEmitVMOVR_VFP);
ARM_MORPH_FN(armEmitVABS_VFP);
ARM_MORPH_FN(armEmitVNEG_VFP);
ARM_MORPH_FN(armEmitVCMP_VFP);
ARM_MORPH_FN(armEmitVCMP0_VFP);
ARM_MORPH_FN(armEmitVCVT_SD_VFP);
ARM_MORPH_FN(armEmitVCVT_SH_VFP);
ARM_MORPH_FN(armEmitVCVT_HS_VFP);
ARM_MORPH_FN(armEmitVCVT_XF_VFP);
ARM_MORPH_FN(armEmitVCVT_FX_VFP);
ARM_MORPH_FN(armEmitVCVT_IF_VFP);
ARM_MORPH_FN(armEmitVCVT_FI_VFP);

// SIMD/VFP Extension register load/store instructions
ARM_MORPH_FN(armEmitVLDR);
ARM_MORPH_FN(armEmitVSTR);
ARM_MORPH_FN(armEmitVLDM);
ARM_MORPH_FN(armEmitVSTM);

// SIMD element or structure load/store instructions
ARM_MORPH_FN(armEmitVLDN);
ARM_MORPH_FN(armEmitVSTN);
ARM_MORPH_FN(armEmitVLDNZ1);
ARM_MORPH_FN(armEmitVSTNZ1);
ARM_MORPH_FN(armEmitVLDNZA);

// SIMD/VFP 8, 16 and 32-bit transfer instructions
ARM_MORPH_FN(armEmitVMRS);
ARM_MORPH_FN(armEmitVMSR);
ARM_MORPH_FN(armEmitVMOVRS);
ARM_MORPH_FN(armEmitVMOVSR);
ARM_MORPH_FN(armEmitVMOVRZ);
ARM_MORPH_FN(armEmitVMOVZR);
ARM_MORPH_FN(armEmitVDUPR);

// SIMD/VFP 64-bit transfer instructions
ARM_MORPH_FN(armEmitVMOVRRD);
ARM_MORPH_FN(armEmitVMOVDRR);
ARM_MORPH_FN(armEmitVMOVRRSS);
ARM_MORPH_FN(armEmitVMOVSSRR);



//
// If the register index specifies a banked register, return a vmiReg structure
// for the banked register; otherwise, return VMI_NOREG. Also, validate the
// current block mode for registers r8 to r14.
//
vmiReg armGetBankedRegMode(armCPSRMode mode, Uns32 r);

#endif
