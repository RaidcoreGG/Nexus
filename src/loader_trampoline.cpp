/*
*  MinHook - The Minimalistic API Hooking Library for x64/x86
*  Copyright (C) 2009-2015 Tsuda Kageyu.
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   1. Redistributions of source code must retain the above copyright
*      notice, this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright
*      notice, this list of conditions and the following disclaimer in the
*      documentation and/or other materials provided with the distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
*  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
*  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
*  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
*  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
*  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
*  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
*  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <windows.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(A) (sizeof(A) / sizeof((A)[0]))
#endif

#include "loader_disasm.h"
typedef hde64s HDE;
#define HDE_DISASM(code, hs) hde64_disasm(code, hs)

#include "loader_trampoline.h"
#include "loader_buffer.h"

// Maximum size of a trampoline function.
#define TRAMPOLINE_MAX_SIZE (MEMORY_SLOT_SIZE - sizeof(JMP_ABS))

//-------------------------------------------------------------------------
static BOOL IsCodePadding(LPBYTE pInst, UINT size) {
	UINT i;

	if (pInst[0] != 0x00 && pInst[0] != 0x90 && pInst[0] != 0xCC)
		return FALSE;

	for (i = 1; i < size; ++i) {
		if (pInst[i] != pInst[0])
			return FALSE;
	}
	return TRUE;
}

//-------------------------------------------------------------------------
BOOL CreateTrampolineFunction(PTRAMPOLINE ct) {
	CALL_ABS call = {
		0xFF,
		0x15,
		0x00000002, // FF15 00000002: CALL [RIP+8]
		0xEB,
		0x08, // EB 08:         JMP +10
		0x0000000000000000ULL // Absolute destination address
	};
	JMP_ABS jmp = {
		0xFF,
		0x25,
		0x00000000, // FF25 00000000: JMP [RIP+6]
		0x0000000000000000ULL // Absolute destination address
	};
	JCC_ABS jcc = {
		0x70,
		0x0E, // 7* 0E:         J** +16
		0xFF,
		0x25,
		0x00000000, // FF25 00000000: JMP [RIP+6]
		0x0000000000000000ULL // Absolute destination address
	};

	UINT8 oldPos = 0;
	UINT8 newPos = 0;
	ULONG_PTR jmpDest = 0; // Destination address of an internal jump.
	BOOL finished = FALSE; // Is the function completed?
	UINT8 instBuf[16];

	ct->patchAbove = FALSE;
	ct->nIP = 0;

	do {
		HDE hs;
		UINT copySize;
		LPVOID pCopySrc;
		ULONG_PTR pOldInst = (ULONG_PTR)ct->pTarget + oldPos;
		ULONG_PTR pNewInst = (ULONG_PTR)ct->pTrampoline + newPos;

		copySize = HDE_DISASM((LPVOID)pOldInst, &hs);
		if (hs.flags & F_ERROR)
			return FALSE;

		pCopySrc = (LPVOID)pOldInst;
		if (oldPos >= sizeof(JMP_REL)) {
			// The trampoline function is long enough.
			// Complete the function with the jump to the target function.
			jmp.address = pOldInst;
			pCopySrc = &jmp;
			copySize = sizeof(jmp);

			finished = TRUE;
		} else if ((hs.modrm & 0xC7) == 0x05) {
			// Instructions using RIP relative addressing. (ModR/M = 00???101B)

			// Modify the RIP relative address.
			PUINT32 pRelAddr;

			// Avoid using memcpy to reduce the footprint.
			__movsb(instBuf, (LPBYTE)pOldInst, copySize);
			pCopySrc = instBuf;

			// Relative address is stored at (instruction length - immediate value length - 4).
			pRelAddr = (PUINT32)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
			*pRelAddr = (UINT32)((pOldInst + hs.len + (INT32)hs.disp.disp32) - (pNewInst + hs.len));

			// Complete the function if JMP (FF /4).
			if (hs.opcode == 0xFF && hs.modrm_reg == 4)
				finished = TRUE;
		} else if (hs.opcode == 0xE8) {
			// Direct relative CALL
			ULONG_PTR dest = pOldInst + hs.len + (INT32)hs.imm.imm32;
			call.address = dest;
			pCopySrc = &call;
			copySize = sizeof(call);
		} else if ((hs.opcode & 0xFD) == 0xE9) {
			// Direct relative JMP (EB or E9)
			ULONG_PTR dest = pOldInst + hs.len;

			if (hs.opcode == 0xEB) // isShort jmp
				dest += (INT8)hs.imm.imm8;
			else
				dest += (INT32)hs.imm.imm32;

			// Simply copy an internal jump.
			if ((ULONG_PTR)ct->pTarget <= dest && dest < ((ULONG_PTR)ct->pTarget + sizeof(JMP_REL))) {
				if (jmpDest < dest)
					jmpDest = dest;
			} else {
				jmp.address = dest;
				pCopySrc = &jmp;
				copySize = sizeof(jmp);

				// Exit the function If it is not in the branch
				finished = (pOldInst >= jmpDest);
			}
		} else if ((hs.opcode & 0xF0) == 0x70 || (hs.opcode & 0xFC) == 0xE0 || (hs.opcode2 & 0xF0) == 0x80) {
			// Direct relative Jcc
			ULONG_PTR dest = pOldInst + hs.len;

			if ((hs.opcode & 0xF0) == 0x70 // Jcc
				||
				(hs.opcode & 0xFC) == 0xE0) // LOOPNZ/LOOPZ/LOOP/JECXZ
				dest += (INT8)hs.imm.imm8;
			else
				dest += (INT32)hs.imm.imm32;

			// Simply copy an internal jump.
			if ((ULONG_PTR)ct->pTarget <= dest && dest < ((ULONG_PTR)ct->pTarget + sizeof(JMP_REL))) {
				if (jmpDest < dest)
					jmpDest = dest;
			} else if ((hs.opcode & 0xFC) == 0xE0) {
				// LOOPNZ/LOOPZ/LOOP/JCXZ/JECXZ to the outside are not supported.
				return FALSE;
			} else {
				UINT8 cond = ((hs.opcode != 0x0F ? hs.opcode : hs.opcode2) & 0x0F);
				// Invert the condition in x64 mode to simplify the conditional jump logic.
				jcc.opcode = 0x71 ^ cond;
				jcc.address = dest;
				pCopySrc = &jcc;
				copySize = sizeof(jcc);
			}
		} else if ((hs.opcode & 0xFE) == 0xC2) {
			// RET (C2 or C3)

			// Complete the function if not in a branch.
			finished = (pOldInst >= jmpDest);
		}

		// Can't alter the instruction length in a branch.
		if (pOldInst < jmpDest && copySize != hs.len)
			return FALSE;

		// Trampoline function is too large.
		if ((newPos + copySize) > TRAMPOLINE_MAX_SIZE)
			return FALSE;

		// Trampoline function has too many instructions.
		if (ct->nIP >= ARRAYSIZE(ct->oldIPs))
			return FALSE;

		ct->oldIPs[ct->nIP] = oldPos;
		ct->newIPs[ct->nIP] = newPos;
		ct->nIP++;

		memcpy((LPBYTE)ct->pTrampoline + newPos, pCopySrc, copySize);
		newPos += (UINT8)copySize;
		oldPos += hs.len;
	} while (!finished);

	// Is there enough place for a long jump?
	if (oldPos < sizeof(JMP_REL) && !IsCodePadding((LPBYTE)ct->pTarget + oldPos, sizeof(JMP_REL) - oldPos)) {
		// Is there enough place for a short jump?
		if (oldPos < sizeof(JMP_REL_SHORT) && !IsCodePadding((LPBYTE)ct->pTarget + oldPos, sizeof(JMP_REL_SHORT) - oldPos)) {
			return FALSE;
		}

		// Can we place the long jump above the function?
		if (!IsExecutableAddress((LPBYTE)ct->pTarget - sizeof(JMP_REL)))
			return FALSE;

		if (!IsCodePadding((LPBYTE)ct->pTarget - sizeof(JMP_REL), sizeof(JMP_REL)))
			return FALSE;

		ct->patchAbove = TRUE;
	}

	// Create a relay function.
	jmp.address = (ULONG_PTR)ct->pDetour;

	ct->pRelay = (LPBYTE)ct->pTrampoline + newPos;
	memcpy(ct->pRelay, &jmp, sizeof(jmp));

	return TRUE;
}
