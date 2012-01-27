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


// VMI header files
#include "vmi/vmiAttrs.h"
#include "vmi/vmiModelInfo.h"
#include "vmi/vmiDoc.h"

#include "armDoc.h"
#include "armParameters.h"
#include "armCP.h"

void armDoc(vmiProcessorP processor, armParamValuesP parameters) {
    vmiDocNodeP root = vmidocAddSection(0, "Root");
    vmiDocNodeP desc = vmidocAddSection(root, "Description");
    vmidocAddText(desc, "ARM Processor Model");

    vmiDocNodeP lic  = vmidocAddSection(root, "Licensing");
    vmidocAddText(lic,  "Imperas Modified Apache 2.0 Open Source License");

    vmiDocNodeP lim  = vmidocAddSection(root, "Limitations");
    vmidocAddText(lim, "Security Extensions are not yet implemented.");
    vmidocAddText(lim, "Performance Monitors are not implemented.");
#if 0
    vmiDocNodeP reg14  = vmidocAddSection(root, "CP14 Register Names and Indexes");
    armCPRegDoc(processor, reg14, 14);

    vmiDocNodeP reg15  = vmidocAddSection(root, "CP15 Register Names and Indexes");
    armCPRegDoc(processor, reg15, 15);
#endif
    vmidocProcessor(processor, root);
}
