/* SPDX-License-Identifier: Apache-2.0 */
/* GRUB: slot stores must use movl $imm; register stores to BSS fail. */

#define GRUB_SLOT_STORE(pos, imm)					\
	do {								\
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"	\
			     "movl %c[imm], " #pos "(%%rdi)\n\t"	\
			     :						\
			     : [imm] "i"(imm)				\
			     : "rdi", "memory");			\
	} while (0)

static void grub_store_slot(int pos, char ch)
{
	switch (pos) {
	case 0:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(, 0x68); break;
		case 'e': GRUB_SLOT_STORE(, 0x65); break;
		case 'l': GRUB_SLOT_STORE(, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(, 0x70); break;
		case 'v': GRUB_SLOT_STORE(, 0x76); break;
		case 'r': GRUB_SLOT_STORE(, 0x72); break;
		case 's': GRUB_SLOT_STORE(, 0x73); break;
		case 'i': GRUB_SLOT_STORE(, 0x69); break;
		case 'o': GRUB_SLOT_STORE(, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(, 0x75); break;
		case 'a': GRUB_SLOT_STORE(, 0x61); break;
		case 'm': GRUB_SLOT_STORE(, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(, 0x63); break;
		case 't': GRUB_SLOT_STORE(, 0x74); break;
		case 'x': GRUB_SLOT_STORE(, 0x78); break;
		case 'g': GRUB_SLOT_STORE(, 0x67); break;
		default: break;
		}
		break;
	case 1:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(4, 0x68); break;
		case 'e': GRUB_SLOT_STORE(4, 0x65); break;
		case 'l': GRUB_SLOT_STORE(4, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(4, 0x70); break;
		case 'v': GRUB_SLOT_STORE(4, 0x76); break;
		case 'r': GRUB_SLOT_STORE(4, 0x72); break;
		case 's': GRUB_SLOT_STORE(4, 0x73); break;
		case 'i': GRUB_SLOT_STORE(4, 0x69); break;
		case 'o': GRUB_SLOT_STORE(4, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(4, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(4, 0x75); break;
		case 'a': GRUB_SLOT_STORE(4, 0x61); break;
		case 'm': GRUB_SLOT_STORE(4, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(4, 0x63); break;
		case 't': GRUB_SLOT_STORE(4, 0x74); break;
		case 'x': GRUB_SLOT_STORE(4, 0x78); break;
		case 'g': GRUB_SLOT_STORE(4, 0x67); break;
		default: break;
		}
		break;
	case 2:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(8, 0x68); break;
		case 'e': GRUB_SLOT_STORE(8, 0x65); break;
		case 'l': GRUB_SLOT_STORE(8, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(8, 0x70); break;
		case 'v': GRUB_SLOT_STORE(8, 0x76); break;
		case 'r': GRUB_SLOT_STORE(8, 0x72); break;
		case 's': GRUB_SLOT_STORE(8, 0x73); break;
		case 'i': GRUB_SLOT_STORE(8, 0x69); break;
		case 'o': GRUB_SLOT_STORE(8, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(8, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(8, 0x75); break;
		case 'a': GRUB_SLOT_STORE(8, 0x61); break;
		case 'm': GRUB_SLOT_STORE(8, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(8, 0x63); break;
		case 't': GRUB_SLOT_STORE(8, 0x74); break;
		case 'x': GRUB_SLOT_STORE(8, 0x78); break;
		case 'g': GRUB_SLOT_STORE(8, 0x67); break;
		default: break;
		}
		break;
	case 3:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(12, 0x68); break;
		case 'e': GRUB_SLOT_STORE(12, 0x65); break;
		case 'l': GRUB_SLOT_STORE(12, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(12, 0x70); break;
		case 'v': GRUB_SLOT_STORE(12, 0x76); break;
		case 'r': GRUB_SLOT_STORE(12, 0x72); break;
		case 's': GRUB_SLOT_STORE(12, 0x73); break;
		case 'i': GRUB_SLOT_STORE(12, 0x69); break;
		case 'o': GRUB_SLOT_STORE(12, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(12, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(12, 0x75); break;
		case 'a': GRUB_SLOT_STORE(12, 0x61); break;
		case 'm': GRUB_SLOT_STORE(12, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(12, 0x63); break;
		case 't': GRUB_SLOT_STORE(12, 0x74); break;
		case 'x': GRUB_SLOT_STORE(12, 0x78); break;
		case 'g': GRUB_SLOT_STORE(12, 0x67); break;
		default: break;
		}
		break;
	case 4:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(16, 0x68); break;
		case 'e': GRUB_SLOT_STORE(16, 0x65); break;
		case 'l': GRUB_SLOT_STORE(16, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(16, 0x70); break;
		case 'v': GRUB_SLOT_STORE(16, 0x76); break;
		case 'r': GRUB_SLOT_STORE(16, 0x72); break;
		case 's': GRUB_SLOT_STORE(16, 0x73); break;
		case 'i': GRUB_SLOT_STORE(16, 0x69); break;
		case 'o': GRUB_SLOT_STORE(16, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(16, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(16, 0x75); break;
		case 'a': GRUB_SLOT_STORE(16, 0x61); break;
		case 'm': GRUB_SLOT_STORE(16, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(16, 0x63); break;
		case 't': GRUB_SLOT_STORE(16, 0x74); break;
		case 'x': GRUB_SLOT_STORE(16, 0x78); break;
		case 'g': GRUB_SLOT_STORE(16, 0x67); break;
		default: break;
		}
		break;
	case 5:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(20, 0x68); break;
		case 'e': GRUB_SLOT_STORE(20, 0x65); break;
		case 'l': GRUB_SLOT_STORE(20, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(20, 0x70); break;
		case 'v': GRUB_SLOT_STORE(20, 0x76); break;
		case 'r': GRUB_SLOT_STORE(20, 0x72); break;
		case 's': GRUB_SLOT_STORE(20, 0x73); break;
		case 'i': GRUB_SLOT_STORE(20, 0x69); break;
		case 'o': GRUB_SLOT_STORE(20, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(20, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(20, 0x75); break;
		case 'a': GRUB_SLOT_STORE(20, 0x61); break;
		case 'm': GRUB_SLOT_STORE(20, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(20, 0x63); break;
		case 't': GRUB_SLOT_STORE(20, 0x74); break;
		case 'x': GRUB_SLOT_STORE(20, 0x78); break;
		case 'g': GRUB_SLOT_STORE(20, 0x67); break;
		default: break;
		}
		break;
	case 6:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(24, 0x68); break;
		case 'e': GRUB_SLOT_STORE(24, 0x65); break;
		case 'l': GRUB_SLOT_STORE(24, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(24, 0x70); break;
		case 'v': GRUB_SLOT_STORE(24, 0x76); break;
		case 'r': GRUB_SLOT_STORE(24, 0x72); break;
		case 's': GRUB_SLOT_STORE(24, 0x73); break;
		case 'i': GRUB_SLOT_STORE(24, 0x69); break;
		case 'o': GRUB_SLOT_STORE(24, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(24, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(24, 0x75); break;
		case 'a': GRUB_SLOT_STORE(24, 0x61); break;
		case 'm': GRUB_SLOT_STORE(24, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(24, 0x63); break;
		case 't': GRUB_SLOT_STORE(24, 0x74); break;
		case 'x': GRUB_SLOT_STORE(24, 0x78); break;
		case 'g': GRUB_SLOT_STORE(24, 0x67); break;
		default: break;
		}
		break;
	case 7:
		switch ((unsigned char)ch) {
		case 'h': GRUB_SLOT_STORE(28, 0x68); break;
		case 'e': GRUB_SLOT_STORE(28, 0x65); break;
		case 'l': GRUB_SLOT_STORE(28, 0x6c); break;
		case 'p': GRUB_SLOT_STORE(28, 0x70); break;
		case 'v': GRUB_SLOT_STORE(28, 0x76); break;
		case 'r': GRUB_SLOT_STORE(28, 0x72); break;
		case 's': GRUB_SLOT_STORE(28, 0x73); break;
		case 'i': GRUB_SLOT_STORE(28, 0x69); break;
		case 'o': GRUB_SLOT_STORE(28, 0x6f); break;
		case 'n': GRUB_SLOT_STORE(28, 0x6e); break;
		case 'u': GRUB_SLOT_STORE(28, 0x75); break;
		case 'a': GRUB_SLOT_STORE(28, 0x61); break;
		case 'm': GRUB_SLOT_STORE(28, 0x6d); break;
		case 'c': GRUB_SLOT_STORE(28, 0x63); break;
		case 't': GRUB_SLOT_STORE(28, 0x74); break;
		case 'x': GRUB_SLOT_STORE(28, 0x78); break;
		case 'g': GRUB_SLOT_STORE(28, 0x67); break;
		default: break;
		}
		break;
	default:
		break;
	}
}
