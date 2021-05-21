
#pragma once

typedef struct {
	uint8_t  e_ident[0x10];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} Elf32_Shdr;

typedef struct {
	uint32_t index;
	uint32_t sh_offset;
} Elf32_Shdr_Sort;

typedef struct {
	uint32_t st_name;
	uint32_t st_value;
	uint32_t st_size;
	uint8_t  st_info;
	uint8_t  st_other;
	uint16_t st_shndx;
} Elf32_Sym;

typedef struct {
	uint32_t r_offset;
	uint32_t r_info;
	int32_t r_addend;
} Elf32_Rela;

typedef struct {
	uint32_t magic_version;
	uint32_t mRegBytes_Text;
	uint32_t mRegBytes_TextAlign;
	uint32_t mRegBytes_Data;
	uint32_t mRegBytes_DataAlign;
	uint32_t mRegBytes_LoaderInfo;
	uint32_t mRegBytes_LoaderInfoAlign;
	uint32_t mRegBytes_Temp;
	uint32_t mTrampAdj;
	uint32_t mSDABase;
	uint32_t mSDA2Base;
	uint32_t mSizeCoreStacks;
	uint32_t mSrcFileNameOffset;
	uint32_t mFlags;
	uint32_t mSysHeapBytes;
	uint32_t mTagsOffset;
} Rpl_Fileinfo;