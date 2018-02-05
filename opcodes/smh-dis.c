/* SMH dissassemble instructions.  */

#include "sysdep.h"
#include <stdio.h>
#define STATIC_TABLE
#define DEFINE_TABLE

#include "opcode/smh.h"
#include "dis-asm.h"

extern const smh_opc_info_t smh_opc_info[128];

static fprintf_ftype fpr;
static void *stream;

#define OP_A(i) ((i >> 6) & 0x7)
#define OP_B(i) ((i >> 3) & 0x7)
#define OP_C(i) ((i >> 0) & 0x7)

int
print_insn_smh (bfd_vma addr, struct disassemble_info *info)
{
  int length = 2;
  int status;
  const smh_opc_info_t *opcode;
  bfd_byte buffer[4];
  unsigned short iword;

  stream = info->stream;
  fpr = info->fprintf_func;

  if ((status = info->read_memory_func (addr, buffer, 2, info)))
    goto fail;
  iword = bfd_getb16 (buffer);

  if ((iword & (1<<15)) == 0)
    {
      opcode = &smh_form1_opc_info[iword >> 9];
      switch (opcode->itype)
	{
	case SMH_F1_NARG:
	  fpr (stream, "%s", opcode->name);
	  break;
	case SMH_F1_AB:
	  fpr (stream, "%s\t$r%d, $r%d", opcode->name,
	       OP_A (iword), OP_B (iword));
	  break;
	case SMH_F1_A4:
	  {
	    unsigned imm;

	    if ((status = info->read_memory_func (addr+2, buffer, 4, info)))
	      goto fail;
	    imm = bfd_getb32 (buffer);
	    fpr (stream, "%s\t$r%d, 0x%x", opcode->name, OP_A(iword), imm);
	    length = 6;
	  }
	  break;
	default:
	  abort();
	}
    }
  else
    {
      opcode = &smh_form2_opc_info[(iword >> 12) & 7];
      switch (opcode->itype)
	{
	case SMH_F2_NARG:
	  fpr (stream, "%s", opcode->name);
	  break;
	default:
	  abort();
	}
    }
  return length;

  fail:
    info->memory_error_func (status, addr, info);
    return -1;
}
