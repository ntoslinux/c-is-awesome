#ifndef __VM_OPCODES_H
#define __VM_OPCODES_H

#define NR_OPCS				256

#define OPC_NOP				0x00
#define OPC_ACONST_NULL			0x01
#define OPC_ICONST_M1			0x02
#define OPC_ICONST_0			0x03
#define OPC_ICONST_1			0x04
#define OPC_ICONST_2			0x05
#define OPC_ICONST_3			0x06
#define OPC_ICONST_4			0x07
#define OPC_ICONST_5			0x08
#define OPC_LCONST_0			0x09
#define OPC_LCONST_1			0x0a
#define OPC_FCONST_0			0x0b
#define OPC_FCONST_1			0x0c
#define OPC_FCONST_2			0x0d
#define OPC_DCONST_0			0x0e
#define OPC_DCONST_1			0x0f
#define OPC_BIPUSH			0x10
#define OPC_SIPUSH			0x11
#define OPC_LDC				0x12
#define OPC_LDC_W			0x13
#define OPC_LDC2_W			0x14
#define OPC_ILOAD			0x15
#define OPC_LLOAD			0x16
#define OPC_FLOAD			0x17
#define OPC_DLOAD			0x18
#define OPC_ALOAD			0x19
#define OPC_ILOAD_0			0x1a
#define OPC_ILOAD_1			0x1b
#define OPC_ILOAD_2			0x1c
#define OPC_ILOAD_3			0x1d
#define OPC_LLOAD_0			0x1e
#define OPC_LLOAD_1			0x1f
#define OPC_LLOAD_2			0x20
#define OPC_LLOAD_3			0x21
#define OPC_FLOAD_0			0x22
#define OPC_FLOAD_1			0x23
#define OPC_FLOAD_2			0x24
#define OPC_FLOAD_3			0x25
#define OPC_DLOAD_0			0x26
#define OPC_DLOAD_1			0x27
#define OPC_DLOAD_2			0x28
#define OPC_DLOAD_3			0x29
#define OPC_ALOAD_0			0x2a
#define OPC_ALOAD_1			0x2b
#define OPC_ALOAD_2			0x2c
#define OPC_ALOAD_3			0x2d
#define OPC_IALOAD			0x2e
#define OPC_LALOAD			0x2f
#define OPC_FALOAD			0x30
#define OPC_DALOAD			0x31
#define OPC_AALOAD			0x32
#define OPC_BALOAD			0x33
#define OPC_CALOAD			0x34
#define OPC_SALOAD			0x35
#define OPC_ISTORE			0x36
#define OPC_LSTORE			0x37
#define OPC_FSTORE			0x38
#define OPC_DSTORE			0x39
#define OPC_ASTORE			0x3a
#define OPC_ISTORE_0			0x3b
#define OPC_ISTORE_1			0x3c
#define OPC_ISTORE_2			0x3d
#define OPC_ISTORE_3			0x3e
#define OPC_LSTORE_0			0x3f
#define OPC_LSTORE_1			0x40
#define OPC_LSTORE_2			0x41
#define OPC_LSTORE_3			0x42
#define OPC_FSTORE_0			0x43
#define OPC_FSTORE_1			0x44
#define OPC_FSTORE_2			0x45
#define OPC_FSTORE_3			0x46
#define OPC_DSTORE_0			0x47
#define OPC_DSTORE_1			0x48
#define OPC_DSTORE_2			0x49
#define OPC_DSTORE_3			0x4a
#define OPC_ASTORE_0			0x4b
#define OPC_ASTORE_1			0x4c
#define OPC_ASTORE_2			0x4d
#define OPC_ASTORE_3			0x4e
#define OPC_IASTORE			0x4f
#define OPC_LASTORE			0x50
#define OPC_FASTORE			0x51
#define OPC_DASTORE			0x52
#define OPC_AASTORE			0x53
#define OPC_BASTORE			0x54
#define OPC_CASTORE			0x55
#define OPC_SASTORE			0x56
#define OPC_POP				0x57
#define OPC_POP2			0x58
#define OPC_DUP				0x59
#define OPC_DUP_X1			0x5a
#define OPC_DUP_X2			0x5b
#define OPC_DUP2			0x5c
#define OPC_DUP2_X1			0x5d
#define OPC_DUP2_X2			0x5e
#define OPC_SWAP			0x5f
#define OPC_IADD			0x60
#define OPC_LADD			0x61
#define OPC_FADD			0x62
#define OPC_DADD			0x63
#define OPC_ISUB			0x64
#define OPC_LSUB			0x65
#define OPC_FSUB			0x66
#define OPC_DSUB			0x67
#define OPC_IMUL			0x68
#define OPC_LMUL			0x69
#define OPC_FMUL			0x6a
#define OPC_DMUL			0x6b
#define OPC_IDIV			0x6c
#define OPC_LDIV			0x6d
#define OPC_FDIV			0x6e
#define OPC_DDIV			0x6f
#define OPC_IREM			0x70
#define OPC_LREM			0x71
#define OPC_FREM			0x72
#define OPC_DREM			0x73
#define OPC_INEG			0x74
#define OPC_LNEG			0x75
#define OPC_FNEG			0x76
#define OPC_DNEG			0x77
#define OPC_ISHL			0x78
#define OPC_LSHL			0x79
#define OPC_ISHR			0x7a
#define OPC_LSHR			0x7b
#define OPC_IUSHR			0x7c
#define OPC_LUSHR			0x7d
#define OPC_IAND			0x7e
#define OPC_LAND			0x7f
#define OPC_IOR				0x80
#define OPC_LOR				0x81
#define OPC_IXOR			0x82
#define OPC_LXOR			0x83
#define OPC_IINC			0x84
#define OPC_I2L				0x85
#define OPC_I2F				0x86
#define OPC_I2D				0x87
#define OPC_L2I				0x88
#define OPC_L2F				0x89
#define OPC_L2D				0x8a
#define OPC_F2I				0x8b
#define OPC_F2L				0x8c
#define OPC_F2D				0x8d
#define OPC_D2I				0x8e
#define OPC_D2L				0x8f
#define OPC_D2F				0x90
#define OPC_I2B				0x91
#define OPC_I2C				0x92
#define OPC_I2S				0x93
#define OPC_LCMP			0x94
#define OPC_FCMPL			0x95
#define OPC_FCMPG			0x96
#define OPC_DCMPL			0x97
#define OPC_DCMPG			0x98
#define OPC_IFEQ			0x99
#define OPC_IFNE			0x9a
#define OPC_IFLT			0x9b
#define OPC_IFGE			0x9c
#define OPC_IFGT			0x9d
#define OPC_IFLE			0x9e
#define OPC_IF_ICMPEQ			0x9f
#define OPC_IF_ICMPNE			0xa0
#define OPC_IF_ICMPLT			0xa1
#define OPC_IF_ICMPGE			0xa2
#define OPC_IF_ICMPGT			0xa3
#define OPC_IF_ICMPLE			0xa4
#define OPC_IF_ACMPEQ			0xa5
#define OPC_IF_ACMPNE			0xa6
#define OPC_GOTO			0xa7
#define OPC_JSR				0xa8
#define OPC_RET				0xa9
#define OPC_TABLESWITCH			0xaa
#define OPC_LOOKUPSWITCH		0xab
#define OPC_IRETURN			0xac
#define OPC_LRETURN			0xad
#define OPC_FRETURN			0xae
#define OPC_DRETURN			0xaf
#define OPC_ARETURN			0xb0
#define OPC_RETURN			0xb1
#define OPC_GETSTATIC			0xb2
#define OPC_PUTSTATIC			0xb3
#define OPC_GETFIELD			0xb4
#define OPC_PUTFIELD			0xb5
#define OPC_INVOKEVIRTUAL		0xb6
#define OPC_INVOKESPECIAL		0xb7
#define OPC_INVOKESTATIC		0xb8
#define OPC_INVOKEINTERFACE		0xb9
#define OPC_XXXUNUSEDXXX		0xba
#define OPC_XXXUNUSEDXXX_		0xba
#define OPC_NEW				0xbb
#define OPC_NEWARRAY			0xbc
#define OPC_ANEWARRAY			0xbd
#define OPC_ARRAYLENGTH			0xbe
#define OPC_ATHROW			0xbf
#define OPC_CHECKCAST			0xc0
#define OPC_INSTANCEOF			0xc1
#define OPC_MONITORENTER		0xc2
#define OPC_MONITOREXIT			0xc3
#define OPC_WIDE			0xc4
#define OPC_MULTIANEWARRAY		0xc5
#define OPC_IFNULL			0xc6
#define OPC_IFNONNULL			0xc7
#define OPC_GOTO_W			0xc8
#define OPC_JSR_W			0xc9
#define OPC_BREAKPOINT			0xca
#define OPC_IMPDEP1			0xfe
#define OPC_IMPDEP2			0xff

#endif