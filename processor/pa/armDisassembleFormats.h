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

#ifndef ARM_DISASSEMBLE_FORMATS_H
#define ARM_DISASSEMBLE_FORMATS_H

//
// These are placeholders in disassembly decoder
//
#define EMIT_R1         '\001'
#define EMIT_R2         '\002'
#define EMIT_R3         '\003'
#define EMIT_R4         '\004'
#define EMIT_CU         '\005'
#define EMIT_CS         '\006'
#define EMIT_CX         '\007'
#define EMIT_T          '\010'
#define EMIT_SHIFT      '\011'
#define EMIT_SHIFT_C    '\012'
#define EMIT_CPNUM      '\013'
#define EMIT_COP1       '\014'
#define EMIT_COP2       '\015'
#define EMIT_CR1        '\016'
#define EMIT_CR2        '\017'
#define EMIT_CR3        '\020'
#define EMIT_OPT        '\021'
#define EMIT_WB         '\022'
#define EMIT_RLIST      '\023'
#define EMIT_U          '\024'
#define EMIT_FIELDSR1   '\025'
#define EMIT_FLAGS      '\026'
#define EMIT_OPT_MODE   '\027'
#define EMIT_ENDIAN     '\030'
#define EMIT_LIM        '\031'
#define EMIT_WIDTH      '\032'
#define EMIT_ITC        '\033'
#define EMIT_SZSHIFT    '\034'
#define EMIT_R1F        '\035'
#define EMIT_H          '\036'
#define EMIT_VESR1      '\300'
#define EMIT_VESR2      '\301'
#define EMIT_S1         '\302'
#define EMIT_S2         '\303'
#define EMIT_S3         '\304'
#define EMIT_D1         '\305'
#define EMIT_D2         '\306'
#define EMIT_D3         '\307'
#define EMIT_Q1         '\310'
#define EMIT_Q2         '\311'
#define EMIT_Q3         '\312'
#define EMIT_Z1         '\313'
#define EMIT_Z2         '\314'
#define EMIT_Z3         '\315'
#define EMIT_SS1        '\316'
#define EMIT_SS3        '\317'
#define EMIT_C0         '\320'
#define EMIT_C8         '\321'
#define EMIT_C16        '\322'
#define EMIT_C32        '\323'
#define EMIT_C0F        '\324'
#define EMIT_SDFP_MI    '\325'
#define EMIT_SIMD_RL    '\326'
#define EMIT_VFP_RL     '\327'
#define EMIT_ES_RL      '\330'
#define EMIT_ES_RL2     '\331'
#define EMIT_ES_RLZ     '\332'
#define EMIT_ES_RLZA    '\333'
#define EMIT_ES_ALIGN   '\334'
#define EMIT_ES_UPDT    '\335'

//
// These are placeholders in disassembly format strings
//
#define EMIT_R1_S       "\001"
#define EMIT_R2_S       "\002"
#define EMIT_R3_S       "\003"
#define EMIT_R4_S       "\004"
#define EMIT_CU_S       "\005"
#define EMIT_CS_S       "\006"
#define EMIT_CX_S       "\007"
#define EMIT_T_S        "\010"
#define EMIT_SHIFT_S    "\011"
#define EMIT_SHIFT_C_S  "\012"
#define EMIT_CPNUM_S    "\013"
#define EMIT_COP1_S     "\014"
#define EMIT_COP2_S     "\015"
#define EMIT_CR1_S      "\016"
#define EMIT_CR2_S      "\017"
#define EMIT_CR3_S      "\020"
#define EMIT_OPT_S      "\021"
#define EMIT_WB_S       "\022"
#define EMIT_RLIST_S    "\023"
#define EMIT_U_S        "\024"
#define EMIT_FIELDSR1_S "\025"
#define EMIT_FLAGS_S    "\026"
#define EMIT_OPT_MODE_S "\027"
#define EMIT_ENDIAN_S   "\030"
#define EMIT_LIM_S      "\031"
#define EMIT_WIDTH_S    "\032"
#define EMIT_ITC_S      "\033"
#define EMIT_SZSHIFT_S  "\034"
#define EMIT_R1F_S      "\035"
#define EMIT_H_S        "\036"
#define EMIT_VESR1_S    "\300"
#define EMIT_VESR2_S    "\301"
#define EMIT_S1_S       "\302"
#define EMIT_S2_S       "\303"
#define EMIT_S3_S       "\304"
#define EMIT_D1_S       "\305"
#define EMIT_D2_S       "\306"
#define EMIT_D3_S       "\307"
#define EMIT_Q1_S       "\310"
#define EMIT_Q2_S       "\311"
#define EMIT_Q3_S       "\312"
#define EMIT_Z1_S       "\313"
#define EMIT_Z2_S       "\314"
#define EMIT_Z3_S       "\315"
#define EMIT_SS1_S      "\316"
#define EMIT_SS3_S      "\317"
#define EMIT_C0_S       "\320"
#define EMIT_C8_S       "\321"
#define EMIT_C16_S      "\322"
#define EMIT_C32_S      "\323"
#define EMIT_C0F_S      "\324"
#define EMIT_SDFP_MI_S  "\325"
#define EMIT_SIMD_RL_S  "\326"
#define EMIT_VFP_RL_S   "\327"
#define EMIT_ES_RL_S    "\330"
#define EMIT_ES_RL2_S   "\331"
#define EMIT_ES_RLZ_S   "\332"
#define EMIT_ES_RLZA_S  "\333"
#define EMIT_ES_ALIGN_S "\334"
#define EMIT_ES_UPDT_S  "\335"
//
// These are disassembly format strings
//
#define FMT_NONE                            ""
#define FMT_R1                              EMIT_R1_S
#define FMT_R1_R2                           EMIT_R1_S "," EMIT_R2_S
#define FMT_R1_R2_R3                        EMIT_R1_S "," EMIT_R2_S "," EMIT_R3_S
#define FMT_R1_R2_R3_R4                     EMIT_R1_S "," EMIT_R2_S "," EMIT_R3_S "," EMIT_R4_S
#define FMT_XIMM                            EMIT_CX_S
#define FMT_SIMM                            EMIT_CS_S
#define FMT_UIMM                            EMIT_CU_S
#define FMT_R1_SIMM                         EMIT_R1_S "," EMIT_CS_S
#define FMT_R1_UIMM                         EMIT_R1_S "," EMIT_CU_S
#define FMT_R1_R2_SIMM                      EMIT_R1_S "," EMIT_R2_S "," EMIT_CS_S
#define FMT_R1_R2_UIMM                      EMIT_R1_S "," EMIT_R2_S "," EMIT_CU_S
#define FMT_R1_R2_SHIFT_SIMM                EMIT_R1_S "," EMIT_R2_S "," EMIT_SHIFT_C_S
#define FMT_R1_R2_SHIFT_R3                  EMIT_R1_S "," EMIT_R2_S "," EMIT_SHIFT_S " " EMIT_R3_S
#define FMT_R1_R2_SHIFT                     EMIT_R1_S "," EMIT_R2_S "," EMIT_SHIFT_S
#define FMT_R1_R2_R3_SHIFT_SIMM             EMIT_R1_S "," EMIT_R2_S "," EMIT_R3_S "," EMIT_SHIFT_C_S
#define FMT_R1_R2_R3_SHIFT_R4               EMIT_R1_S "," EMIT_R2_S "," EMIT_R3_S "," EMIT_SHIFT_S " " EMIT_R4_S
#define FMT_R1_R2_R3_SHIFT                  EMIT_R1_S "," EMIT_R2_S "," EMIT_R3_S "," EMIT_SHIFT_S
#define FMT_R1_WIDTH_R2                     EMIT_R1_S "," EMIT_WIDTH_S "," EMIT_R2_S
#define FMT_R1_WIDTH_R2_SHIFT_SIMM          EMIT_R1_S "," EMIT_WIDTH_S "," EMIT_R2_S "," EMIT_SHIFT_C_S
#define FMT_R1_ADDR_R2_SIMM                 EMIT_R1_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]" EMIT_WB_S
#define FMT_R1_ADDR_R2_R3                   EMIT_R1_S ",[" EMIT_R2_S "1]," EMIT_U_S EMIT_R3_S "2]" EMIT_WB_S
#define FMT_R1_ADDR_R2_R3_SHIFT_SIMM        EMIT_R1_S ",[" EMIT_R2_S "1]," EMIT_U_S EMIT_R3_S "," EMIT_SHIFT_C_S "2]" EMIT_WB_S
#define FMT_R1_ADDR_R2_R3_SHIFT             EMIT_R1_S ",[" EMIT_R2_S "1]," EMIT_U_S EMIT_R3_S "," EMIT_SHIFT_S "2]" EMIT_WB_S
#define FMT_R1_R4_ADDR_R2_SIMM              EMIT_R1_S "," EMIT_R4_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]" EMIT_WB_S
#define FMT_ADDR_R1_SIMM                    "[" EMIT_R1_S "1],*" EMIT_CS_S "2]" EMIT_WB_S
#define FMT_ADDR_R1_R2                      "[" EMIT_R1_S "1]," EMIT_U_S EMIT_R2_S "2]" EMIT_WB_S
#define FMT_ADDR_R1_R2_SHIFT_SIMM           "[" EMIT_R1_S "1]," EMIT_U_S EMIT_R2_S "," EMIT_SHIFT_C_S "2]" EMIT_WB_S
#define FMT_ADDR_R1_R2_SHIFT                "[" EMIT_R1_S "1]," EMIT_U_S EMIT_R2_S "," EMIT_SHIFT_S "2]" EMIT_WB_S
#define FMT_R1_R2_ADDR_R3_SIMM              EMIT_R1_S "," EMIT_R2_S ",[" EMIT_R3_S "1],*" EMIT_CS_S "2]"
#define FMT_R1_R2_R4_ADDR_R3_SIMM           EMIT_R1_S "," EMIT_R2_S "," EMIT_R4_S ",[" EMIT_R3_S "1],*" EMIT_CS_S "2]"
#define FMT_R1_R4_ADDR_R2_SIMM              EMIT_R1_S "," EMIT_R4_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]" EMIT_WB_S
#define ADDR_R1_R2_SZSHIFT                  ",[" EMIT_R1_S "," EMIT_R2_S ",*" EMIT_SZSHIFT_S "]"
#define FMT_CPNUM_CR1_SIMM                  EMIT_CPNUM_S "," EMIT_CR1_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]" EMIT_WB_S
#define FMT_CPNUM_CR1_UNINDEXED             EMIT_CPNUM_S "," EMIT_CR1_S ",[" EMIT_R2_S "],{" EMIT_OPT_S "}"
#define FMT_R1_COMMA                        EMIT_R1_S ","
#define FMT_FIELDSR1_SIMM                   "_" EMIT_FIELDSR1_S "," EMIT_CS_S
#define FMT_FIELDSR1_R2                     "_" EMIT_FIELDSR1_S "," EMIT_R2_S
#define FMT_R1_WB                           EMIT_R1_S EMIT_WB_S
#define FMT_R1_WB_UIMM                      EMIT_R1_S EMIT_WB_S "," EMIT_CU_S
#define FMT_T                               EMIT_T_S
#define FMT_R1_T                            EMIT_R1_S "," EMIT_T_S
#define FMT_CPNUM_COP1_CR1_CR2_CR3_COP2     EMIT_CPNUM_S "," EMIT_COP1_S "," EMIT_CR1_S "," EMIT_CR2_S "," EMIT_CR3_S ",{" EMIT_COP2_S "}"
#define FMT_CPNUM_COP1_R1_R2_CR3            EMIT_CPNUM_S "," EMIT_COP1_S "," EMIT_R1_S "," EMIT_R2_S "," EMIT_CR3_S
#define FMT_CPNUM_COP1_R1_CR2_CR3_COP2      EMIT_CPNUM_S "," EMIT_COP1_S "," EMIT_R1_S "," EMIT_CR2_S "," EMIT_CR3_S ",{" EMIT_COP2_S "}"
#define FMT_CPNUM_COP1_R1F_CR2_CR3_COP2     EMIT_CPNUM_S "," EMIT_COP1_S "," EMIT_R1F_S "," EMIT_CR2_S "," EMIT_CR3_S ",{" EMIT_COP2_S "}"
#define FMT_RLIST                           EMIT_RLIST_S
#define FMT_SIMD_RL                         EMIT_SIMD_RL_S
#define FMT_VFP_RL                          EMIT_VFP_RL_S
#define FMT_R1_RLIST                        EMIT_R1_S EMIT_WB_S "," EMIT_RLIST_S
#define FMT_R1_RLIST_T                      EMIT_R1_S "!," EMIT_RLIST_S
#define FMT_R1_RLIST_UM                     EMIT_R1_S EMIT_WB_S "," EMIT_RLIST_S "^"
#define FMT_R1_VFP_RL                       EMIT_R1_S EMIT_WB_S "," EMIT_VFP_RL_S
#define FMT_R1_SIMD_RL                      EMIT_R1_S EMIT_WB_S "," EMIT_SIMD_RL_S
#define FMT_FLAGS_OPT_MODE                  EMIT_FLAGS_S ",*" EMIT_OPT_MODE_S
#define FMT_ENDIAN                          EMIT_ENDIAN_S
#define FMT_LIM                             EMIT_LIM_S
#define FMT_R1_R2_LSB_WIDTH                 EMIT_R1_S "," EMIT_R2_S "," EMIT_CU_S "," EMIT_WIDTH_S
#define FMT_R1_LSB_WIDTH                    EMIT_R1_S "," EMIT_CU_S "," EMIT_WIDTH_S
#define FMT_ITC                             EMIT_ITC_S
#define FMT_R1F_VESR2                       EMIT_R1F_S "," EMIT_VESR2_S
#define FMT_VESR1_R2                        EMIT_VESR1_S "," EMIT_R2_S
#define FMT_R1_D2                           EMIT_R1_S "," EMIT_D2_S
#define FMT_R1_Q2                           EMIT_R1_S "," EMIT_Q2_S
#define FMT_R1_S2                           EMIT_R1_S "," EMIT_S2_S
#define FMT_R1_Z2                           EMIT_R1_S "," EMIT_Z2_S
#define FMT_D1_D2                           EMIT_D1_S "," EMIT_D2_S
#define FMT_D1_F0                           EMIT_D1_S "," EMIT_C0F_S
#define FMT_D1_Q2                           EMIT_D1_S "," EMIT_Q2_S
#define FMT_D1_R2                           EMIT_D1_S "," EMIT_R2_S
#define FMT_D1_ADDR_R2_SIMM                 EMIT_D1_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]"
#define FMT_D1_S2                           EMIT_D1_S "," EMIT_S2_S
#define FMT_D1_SDFP_MI                      EMIT_D1_S "," EMIT_SDFP_MI_S
#define FMT_D1_Z2                           EMIT_D1_S "," EMIT_Z2_S
#define FMT_Q1_Q2                           EMIT_Q1_S "," EMIT_Q2_S
#define FMT_Q1_D2                           EMIT_Q1_S "," EMIT_D2_S
#define FMT_Q1_R2                           EMIT_Q1_S "," EMIT_R2_S
#define FMT_Q1_SDFP_MI                      EMIT_Q1_S "," EMIT_SDFP_MI_S
#define FMT_Q1_Z2                           EMIT_Q1_S "," EMIT_Z2_S
#define FMT_S1_S2                           EMIT_S1_S "," EMIT_S2_S
#define FMT_S1_D2                           EMIT_S1_S "," EMIT_D2_S
#define FMT_S1_F0                           EMIT_S1_S "," EMIT_C0F_S
#define FMT_S1_R2                           EMIT_S1_S "," EMIT_R2_S
#define FMT_S1_ADDR_R2_SIMM                 EMIT_S1_S ",[" EMIT_R2_S "1],*" EMIT_CS_S "2]"
#define FMT_S1_SDFP_MI                      EMIT_S1_S "," EMIT_SDFP_MI_S
#define FMT_Z1_R2                           EMIT_Z1_S "," EMIT_R2_S
#define FMT_D1_D2_D3                        EMIT_D1_S "," EMIT_D2_S "," EMIT_D3_S
#define FMT_D1_D2_Z3                        EMIT_D1_S "," EMIT_D2_S "," EMIT_Z3_S
#define FMT_D1_D2_UIMM                      EMIT_D1_S "," EMIT_D2_S "," EMIT_CU_S
#define FMT_D1_D2_UIMM                      EMIT_D1_S "," EMIT_D2_S "," EMIT_CU_S
#define FMT_D1_D2_U0                        EMIT_D1_S "," EMIT_D2_S "," EMIT_C0_S
#define FMT_D1_R2_R3                        EMIT_D1_S "," EMIT_R2_S "," EMIT_R3_S
#define FMT_D1_RL_D3                        EMIT_D1_S "," EMIT_ES_RL2_S "," EMIT_D3_S
#define FMT_D1_Q2_Q3                        EMIT_D1_S "," EMIT_Q2_S "," EMIT_Q3_S
#define FMT_D1_Q2_UIMM                      EMIT_D1_S "," EMIT_Q2_S "," EMIT_CU_S
#define FMT_D1_D2_D3_UIMM                   EMIT_D1_S "," EMIT_D2_S "," EMIT_D3_S "," EMIT_CU_S
#define FMT_Q1_Q2_Q3                        EMIT_Q1_S "," EMIT_Q2_S "," EMIT_Q3_S
#define FMT_Q1_Q2_D3                        EMIT_Q1_S "," EMIT_Q2_S "," EMIT_D3_S
#define FMT_Q1_Q2_Z3                        EMIT_Q1_S "," EMIT_Q2_S "," EMIT_Z3_S
#define FMT_Q1_Q2_UIMM                      EMIT_Q1_S "," EMIT_Q2_S "," EMIT_CU_S
#define FMT_Q1_Q2_U0                        EMIT_Q1_S "," EMIT_Q2_S "," EMIT_C0_S
#define FMT_Q1_D2_D3                        EMIT_Q1_S "," EMIT_D2_S "," EMIT_D3_S
#define FMT_Q1_D2_Z3                        EMIT_Q1_S "," EMIT_D2_S "," EMIT_Z3_S
#define FMT_Q1_D2_UIMM                      EMIT_Q1_S "," EMIT_D2_S "," EMIT_CU_S
#define FMT_Q1_D2_U8                        EMIT_Q1_S "," EMIT_D2_S "," EMIT_C8_S
#define FMT_Q1_D2_U16                       EMIT_Q1_S "," EMIT_D2_S "," EMIT_C16_S
#define FMT_Q1_D2_U32                       EMIT_Q1_S "," EMIT_D2_S "," EMIT_C32_S
#define FMT_Q1_Q2_Q3_UIMM                   EMIT_Q1_S "," EMIT_Q2_S "," EMIT_Q3_S "," EMIT_CU_S
#define FMT_R1_R2_D3                        EMIT_R1_S "," EMIT_R2_S "," EMIT_D3_S
#define FMT_R1_R2_SS3                       EMIT_R1_S "," EMIT_R2_S "," EMIT_SS3_S
#define FMT_S1_S2_S3                        EMIT_S1_S "," EMIT_S2_S "," EMIT_S3_S
#define FMT_S1_S2_UIMM                      EMIT_S1_S "," EMIT_S2_S "," EMIT_CU_S
#define FMT_SS1_R2_R3                       EMIT_SS1_S "," EMIT_R2_S "," EMIT_R3_S
#define FMT_H                               EMIT_H_S
#define FMT_UIMM_H                          EMIT_CU_S "," EMIT_H_S
#define FMT_SIMD_ES							EMIT_ES_RL_S ",[" EMIT_R2_S EMIT_ES_ALIGN_S "]" EMIT_ES_UPDT_S
#define FMT_SIMD_ESZ						EMIT_ES_RLZ_S ",[" EMIT_R2_S EMIT_ES_ALIGN_S "]" EMIT_ES_UPDT_S
#define FMT_SIMD_ESZA						EMIT_ES_RLZA_S ",[" EMIT_R2_S EMIT_ES_ALIGN_S "]" EMIT_ES_UPDT_S

#endif
