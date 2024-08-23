/* This linker script generated from xt-genldscripts.tpp for LSP sim */
/* Linker Script for default link */
MEMORY
{
  dram0_0_seg :                         org = 0x00160000, len = 0x20000
  sram0_seg : org = 0x00180000, len = 0x80000
  dram0_seg : org = 0x00200000, len = 0x100000  /* New segment */
  iram0_0_seg :                         org = 0x00200000, len = 0x2E0
  iram0_1_seg :                         org = 0x002002E0, len = 0x30
  iram0_2_seg :                         org = 0x00200310, len = 0x180
  iram0_4_seg :                         org = 0x00200600, len = 0x178
  iram0_5_seg :                         org = 0x00200778, len = 0x4
  iram0_6_seg :                         org = 0x0020077C, len = 0x1C
  iram0_7_seg :                         org = 0x00200798, len = 0x4
  iram0_8_seg :                         org = 0x0020079C, len = 0x1C
  iram0_9_seg :                         org = 0x002007B8, len = 0x4
  iram0_10_seg :                        org = 0x002007BC, len = 0x1C
  iram0_11_seg :                        org = 0x002007D8, len = 0x8
  iram0_12_seg :                        org = 0x002007E0, len = 0x38
  iram0_13_seg :                        org = 0x00200818, len = 0x4
  iram0_14_seg :                        org = 0x0020081C, len = 0x1C
  iram0_15_seg :                        org = 0x00200838, len = 0x4
  iram0_16_seg :                        org = 0x0020083C, len = 0x1C
  iram0_17_seg :                        org = 0x00200858, len = 0x4
  iram0_18_seg :                        org = 0x0020085C, len = 0x1C
  iram0_19_seg :                        org = 0x00200878, len = 0x1F788
  iram1_0_seg :                         org = 0x00220000, len = 0x20000
}

PHDRS
{
  dram0_0_phdr PT_LOAD;
  dram0_0_bss_phdr PT_LOAD;
  sram0_phdr PT_LOAD;
  sram0_bss_phdr PT_LOAD;
  iram0_0_phdr PT_LOAD;
  iram0_1_phdr PT_LOAD;
  iram0_2_phdr PT_LOAD;
  iram0_3_phdr PT_LOAD;
  iram0_4_phdr PT_LOAD;
  iram0_5_phdr PT_LOAD;
  iram0_6_phdr PT_LOAD;
  iram0_7_phdr PT_LOAD;
  iram0_8_phdr PT_LOAD;
  iram0_9_phdr PT_LOAD;
  iram0_10_phdr PT_LOAD;
  iram0_11_phdr PT_LOAD;
  iram0_12_phdr PT_LOAD;
  iram0_13_phdr PT_LOAD;
  iram0_14_phdr PT_LOAD;
  iram0_15_phdr PT_LOAD;
  iram0_16_phdr PT_LOAD;
  iram0_17_phdr PT_LOAD;
  iram0_18_phdr PT_LOAD;
  iram0_19_phdr PT_LOAD;
  iram1_0_phdr PT_LOAD;
}


/*  Default entry point:  */
ENTRY(_ResetVector)


/*  Memory boundary addresses:  */
_memmap_mem_dram0_start = 0x160000;
_memmap_mem_dram0_end   = 0x180000;
_memmap_mem_sram_start = 0x180000;
_memmap_mem_sram_end   = 0x200000;
_memmap_mem_iram0_start = 0x200000;
_memmap_mem_iram0_end   = 0x220000;
_memmap_mem_iram1_start = 0x220000;
_memmap_mem_iram1_end   = 0x240000;

/*  Memory segment boundary addresses:  */
_memmap_seg_dram0_0_start = 0x160000;
_memmap_seg_dram0_0_max   = 0x180000;
_memmap_seg_sram0_start = 0x180000;
_memmap_seg_sram0_max   = 0x200000;
_memmap_seg_iram0_0_start = 0x200000;
_memmap_seg_iram0_0_max   = 0x2002e0;
_memmap_seg_iram0_1_start = 0x2002e0;
_memmap_seg_iram0_1_max   = 0x200310;
_memmap_seg_iram0_2_start = 0x200310;
_memmap_seg_iram0_2_max   = 0x200490;
_memmap_seg_iram0_4_start = 0x200600;
_memmap_seg_iram0_4_max   = 0x200778;
_memmap_seg_iram0_5_start = 0x200778;
_memmap_seg_iram0_5_max   = 0x20077c;
_memmap_seg_iram0_6_start = 0x20077c;
_memmap_seg_iram0_6_max   = 0x200798;
_memmap_seg_iram0_7_start = 0x200798;
_memmap_seg_iram0_7_max   = 0x20079c;
_memmap_seg_iram0_8_start = 0x20079c;
_memmap_seg_iram0_8_max   = 0x2007b8;
_memmap_seg_iram0_9_start = 0x2007b8;
_memmap_seg_iram0_9_max   = 0x2007bc;
_memmap_seg_iram0_10_start = 0x2007bc;
_memmap_seg_iram0_10_max   = 0x2007d8;
_memmap_seg_iram0_11_start = 0x2007d8;
_memmap_seg_iram0_11_max   = 0x2007e0;
_memmap_seg_iram0_12_start = 0x2007e0;
_memmap_seg_iram0_12_max   = 0x200818;
_memmap_seg_iram0_13_start = 0x200818;
_memmap_seg_iram0_13_max   = 0x20081c;
_memmap_seg_iram0_14_start = 0x20081c;
_memmap_seg_iram0_14_max   = 0x200838;
_memmap_seg_iram0_15_start = 0x200838;
_memmap_seg_iram0_15_max   = 0x20083c;
_memmap_seg_iram0_16_start = 0x20083c;
_memmap_seg_iram0_16_max   = 0x200858;
_memmap_seg_iram0_17_start = 0x200858;
_memmap_seg_iram0_17_max   = 0x20085c;
_memmap_seg_iram0_18_start = 0x20085c;
_memmap_seg_iram0_18_max   = 0x200878;
_memmap_seg_iram0_19_start = 0x200878;
_memmap_seg_iram0_19_max   = 0x220000;
_memmap_seg_iram1_0_start = 0x220000;
_memmap_seg_iram1_0_max   = 0x240000;

_rom_store_table = 0;
PROVIDE(_memmap_reset_vector = 0x200000);
/* Various memory-map dependent cache attribute settings: */
_memmap_cacheattr_wb_base = 0x00000001;
_memmap_cacheattr_wt_base = 0x00000001;
_memmap_cacheattr_bp_base = 0x00000002;
_memmap_cacheattr_unused_mask = 0xFFFFFFF0;
_memmap_cacheattr_wb_trapnull = 0x22222221;
_memmap_cacheattr_wba_trapnull = 0x22222221;
_memmap_cacheattr_wbna_trapnull = 0x22222221;
_memmap_cacheattr_wt_trapnull = 0x22222221;
_memmap_cacheattr_bp_trapnull = 0x22222222;
_memmap_cacheattr_wb_strict = 0xFFFFFFF1;
_memmap_cacheattr_wt_strict = 0xFFFFFFF1;
_memmap_cacheattr_bp_strict = 0xFFFFFFF2;
_memmap_cacheattr_wb_allvalid = 0x22222221;
_memmap_cacheattr_wt_allvalid = 0x22222221;
_memmap_cacheattr_bp_allvalid = 0x22222222;
_memmap_region_map = 0x00000001;
PROVIDE(_memmap_cacheattr_reset = _memmap_cacheattr_wb_trapnull);

SECTIONS
{

  .dram0.rodata : ALIGN(4)
  {
    _dram0_rodata_start = ABSOLUTE(.);
    *(.dram0.rodata)
    *(.dram.rodata)
    . = ALIGN (4);
    _dram0_rodata_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .dram0.data : ALIGN(4)
  {
    _dram0_data_start = ABSOLUTE(.);
    *(.dram0.data)
    *(.dram.data)
    . = ALIGN (4);
    _dram0_data_end = ABSOLUTE(.);
  } >dram0_0_seg :dram0_0_phdr

  .dram0.bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _dram0_bss_start = ABSOLUTE(.);
    *(.dram0.bss)
    . = ALIGN (8);
    _dram0_bss_end = ABSOLUTE(.);
    _memmap_seg_dram0_0_end = ALIGN(0x8);
  } >dram0_0_seg :dram0_0_bss_phdr

  _memmap_mem_dram0_max = ABSOLUTE(.);

  .ResetVector.text : ALIGN(4)
  {
    _ResetVector_text_start = ABSOLUTE(.);
    KEEP (*(.ResetVector.text))
    . = ALIGN (4);
    _ResetVector_text_end = ABSOLUTE(.);
  } >iram0_0_seg :iram0_0_phdr

  .ResetHandler.text : ALIGN(4)
  {
    _ResetHandler_text_start = ABSOLUTE(.);
    *(.ResetHandler.literal .ResetHandler.text)
    . = ALIGN (4);
    _ResetHandler_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_0_end = ALIGN(0x8);
  } >iram0_0_seg :iram0_0_phdr


  .MemoryExceptionVector.literal : ALIGN(4)
  {
    _MemoryExceptionVector_literal_start = ABSOLUTE(.);
    *(.MemoryExceptionVector.literal)
    . = ALIGN (4);
    _MemoryExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_1_end = ALIGN(0x8);
  } >iram0_1_seg :iram0_1_phdr


  .MemoryExceptionVector.text : ALIGN(4)
  {
    _MemoryExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.MemoryExceptionVector.text))
    . = ALIGN (4);
    _MemoryExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_2_end = ALIGN(0x8);
  } >iram0_2_seg :iram0_2_phdr



  .WindowVectors.text : ALIGN(4)
  {
    _WindowVectors_text_start = ABSOLUTE(.);
    KEEP (*(.WindowVectors.text))
    . = ALIGN (4);
    _WindowVectors_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_4_end = ALIGN(0x8);
  } >iram0_4_seg :iram0_4_phdr


  .Level2InterruptVector.literal : ALIGN(4)
  {
    _Level2InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level2InterruptVector.literal)
    . = ALIGN (4);
    _Level2InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_5_end = ALIGN(0x8);
  } >iram0_5_seg :iram0_5_phdr


  .Level2InterruptVector.text : ALIGN(4)
  {
    _Level2InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level2InterruptVector.text))
    . = ALIGN (4);
    _Level2InterruptVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_6_end = ALIGN(0x8);
  } >iram0_6_seg :iram0_6_phdr


  .Level3InterruptVector.literal : ALIGN(4)
  {
    _Level3InterruptVector_literal_start = ABSOLUTE(.);
    *(.Level3InterruptVector.literal)
    . = ALIGN (4);
    _Level3InterruptVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_7_end = ALIGN(0x8);
  } >iram0_7_seg :iram0_7_phdr


  .Level3InterruptVector.text : ALIGN(4)
  {
    _Level3InterruptVector_text_start = ABSOLUTE(.);
    KEEP (*(.Level3InterruptVector.text))
    . = ALIGN (4);
    _Level3InterruptVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_8_end = ALIGN(0x8);
  } >iram0_8_seg :iram0_8_phdr


  .DebugExceptionVector.literal : ALIGN(4)
  {
    _DebugExceptionVector_literal_start = ABSOLUTE(.);
    *(.DebugExceptionVector.literal)
    . = ALIGN (4);
    _DebugExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_9_end = ALIGN(0x8);
  } >iram0_9_seg :iram0_9_phdr


  .DebugExceptionVector.text : ALIGN(4)
  {
    _DebugExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DebugExceptionVector.text))
    . = ALIGN (4);
    _DebugExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_10_end = ALIGN(0x8);
  } >iram0_10_seg :iram0_10_phdr


  .NMIExceptionVector.literal : ALIGN(4)
  {
    _NMIExceptionVector_literal_start = ABSOLUTE(.);
    *(.NMIExceptionVector.literal)
    . = ALIGN (4);
    _NMIExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_11_end = ALIGN(0x8);
  } >iram0_11_seg :iram0_11_phdr


  .NMIExceptionVector.text : ALIGN(4)
  {
    _NMIExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.NMIExceptionVector.text))
    . = ALIGN (4);
    _NMIExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_12_end = ALIGN(0x8);
  } >iram0_12_seg :iram0_12_phdr


  .KernelExceptionVector.literal : ALIGN(4)
  {
    _KernelExceptionVector_literal_start = ABSOLUTE(.);
    *(.KernelExceptionVector.literal)
    . = ALIGN (4);
    _KernelExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_13_end = ALIGN(0x8);
  } >iram0_13_seg :iram0_13_phdr


  .KernelExceptionVector.text : ALIGN(4)
  {
    _KernelExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.KernelExceptionVector.text))
    . = ALIGN (4);
    _KernelExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_14_end = ALIGN(0x8);
  } >iram0_14_seg :iram0_14_phdr


  .UserExceptionVector.literal : ALIGN(4)
  {
    _UserExceptionVector_literal_start = ABSOLUTE(.);
    *(.UserExceptionVector.literal)
    . = ALIGN (4);
    _UserExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_15_end = ALIGN(0x8);
  } >iram0_15_seg :iram0_15_phdr


  .UserExceptionVector.text : ALIGN(4)
  {
    _UserExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.UserExceptionVector.text))
    . = ALIGN (4);
    _UserExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_16_end = ALIGN(0x8);
  } >iram0_16_seg :iram0_16_phdr


  .DoubleExceptionVector.literal : ALIGN(4)
  {
    _DoubleExceptionVector_literal_start = ABSOLUTE(.);
    *(.DoubleExceptionVector.literal)
    . = ALIGN (4);
    _DoubleExceptionVector_literal_end = ABSOLUTE(.);
    _memmap_seg_iram0_17_end = ALIGN(0x8);
  } >iram0_17_seg :iram0_17_phdr


  .DoubleExceptionVector.text : ALIGN(4)
  {
    _DoubleExceptionVector_text_start = ABSOLUTE(.);
    KEEP (*(.DoubleExceptionVector.text))
    . = ALIGN (4);
    _DoubleExceptionVector_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_18_end = ALIGN(0x8);
  } >iram0_18_seg :iram0_18_phdr


  .iram0.text : ALIGN(4)
  {
    _iram0_text_start = ABSOLUTE(.);
    *(.iram0.literal .iram0.text)
    . = ALIGN (4);
    _iram0_text_end = ABSOLUTE(.);
    _memmap_seg_iram0_19_end = ALIGN(0x8);
  } >iram0_19_seg :iram0_19_phdr

  _memmap_mem_iram0_max = ABSOLUTE(.);

  .iram1.text : ALIGN(4)
  {
    _iram1_text_start = ABSOLUTE(.);
    *(.iram1.literal .iram1.text)
    . = ALIGN (4);
    _iram1_text_end = ABSOLUTE(.);
    _memmap_seg_iram1_0_end = ALIGN(0x8);
  } >iram1_0_seg :iram1_0_phdr

  _memmap_mem_iram1_max = ABSOLUTE(.);

  .sram.rodata : ALIGN(4)
  {
    _sram_rodata_start = ABSOLUTE(.);
    *(.sram.rodata)
    . = ALIGN (4);
    _sram_rodata_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .clib.rodata : ALIGN(4)
  {
    _clib_rodata_start = ABSOLUTE(.);
    *(.clib.rodata)
    . = ALIGN (4);
    _clib_rodata_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .rtos.rodata : ALIGN(4)
  {
    _rtos_rodata_start = ABSOLUTE(.);
    *(.rtos.rodata)
    . = ALIGN (4);
    _rtos_rodata_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .rodata : ALIGN(4)
  {
    _rodata_start = ABSOLUTE(.);
    *(.rodata)
    *(SORT(.rodata.sort.*))
    KEEP (*(SORT(.rodata.keepsort.*) .rodata.keep.*))
    *(.rodata.*)
    *(.gnu.linkonce.r.*)
    *(.rodata1)
    __XT_EXCEPTION_TABLE__ = ABSOLUTE(.);
    KEEP (*(.xt_except_table))
    KEEP (*(.gcc_except_table))
    *(.gnu.linkonce.e.*)
    *(.gnu.version_r)
    PROVIDE (__eh_frame_start = .);
    KEEP (*(.eh_frame))
    PROVIDE (__eh_frame_end = .);
    /*  C++ constructor and destructor tables, properly ordered:  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
    KEEP (*crtbegin.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
    /*  C++ exception handlers table:  */
    __XT_EXCEPTION_DESCS__ = ABSOLUTE(.);
    *(.xt_except_desc)
    *(.gnu.linkonce.h.*)
    __XT_EXCEPTION_DESCS_END__ = ABSOLUTE(.);
    *(.xt_except_desc_end)
    *(.dynamic)
    *(.gnu.version_d)
    . = ALIGN(4);   /* this table MUST be 4-byte aligned */
    _bss_table_start = ABSOLUTE(.);
    LONG(_dram0_bss_start)
    LONG(_dram0_bss_end)
    LONG(_bss_start)
    LONG(_bss_end)
    _bss_table_end = ABSOLUTE(.);
    . = ALIGN (4);
    _rodata_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .sram.text : ALIGN(4)
  {
    _sram_text_start = ABSOLUTE(.);
    *(.sram.literal .sram.text)
    . = ALIGN (4);
    _sram_text_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .text : ALIGN(4)
  {
    _stext = .;
    _text_start = ABSOLUTE(.);
    *(.entry.text)
    *(.init.literal)
    KEEP(*(.init))
    *(.literal.sort.* SORT(.text.sort.*))
    KEEP (*(.literal.keepsort.* SORT(.text.keepsort.*) .literal.keep.* .text.keep.* .literal.*personality* .text.*personality*))
    *(.literal .text .literal.* .text.* .stub .gnu.warning .gnu.linkonce.literal.* .gnu.linkonce.t.*.literal .gnu.linkonce.t.*)
    *(.fini.literal)
    KEEP(*(.fini))
    *(.gnu.version)
    . = ALIGN (4);
    _text_end = ABSOLUTE(.);
    _etext = .;
  } >sram0_seg :sram0_phdr

  .clib.text : ALIGN(4)
  {
    _clib_text_start = ABSOLUTE(.);
    *(.clib.literal .clib.text)
    . = ALIGN (4);
    _clib_text_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .rtos.text : ALIGN(4)
  {
    _rtos_text_start = ABSOLUTE(.);
    *(.rtos.literal .rtos.text)
    . = ALIGN (4);
    _rtos_text_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .clib.data : ALIGN(4)
  {
    _clib_data_start = ABSOLUTE(.);
    *(.clib.data)
    . = ALIGN (4);
    _clib_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .clib.percpu.data : ALIGN(4)
  {
    _clib_percpu_data_start = ABSOLUTE(.);
    *(.clib.percpu.data)
    . = ALIGN (4);
    _clib_percpu_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .rtos.percpu.data : ALIGN(4)
  {
    _rtos_percpu_data_start = ABSOLUTE(.);
    *(.rtos.percpu.data)
    . = ALIGN (4);
    _rtos_percpu_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .rtos.data : ALIGN(4)
  {
    _rtos_data_start = ABSOLUTE(.);
    *(.rtos.data)
    . = ALIGN (4);
    _rtos_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .sram.data : ALIGN(4)
  {
    _sram_data_start = ABSOLUTE(.);
    *(.sram.data)
    . = ALIGN (4);
    _sram_data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .data : ALIGN(4)
  {
    _data_start = ABSOLUTE(.);
    *(.data)
    *(SORT(.data.sort.*))
    KEEP (*(SORT(.data.keepsort.*) .data.keep.*))
    *(.data.*)
    *(.gnu.linkonce.d.*)
    KEEP(*(.gnu.linkonce.d.*personality*))
    *(.data1)
    *(.sdata)
    *(.sdata.*)
    *(.gnu.linkonce.s.*)
    *(.sdata2)
    *(.sdata2.*)
    *(.gnu.linkonce.s2.*)
    KEEP(*(.jcr))
    *(__llvm_prf_cnts)
    *(__llvm_prf_data)
    *(__llvm_prf_vnds)
    . = ALIGN (4);
    _data_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  __llvm_prf_names : ALIGN(4)
  {
    __llvm_prf_names_start = ABSOLUTE(.);
    *(__llvm_prf_names)
    . = ALIGN (4);
    __llvm_prf_names_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .note.gnu.build-id : ALIGN(4)
  {
    _note_gnu_build-id_start = ABSOLUTE(.);
    *(.note.gnu.build-id)
    . = ALIGN (4);
    _note_gnu_build-id_end = ABSOLUTE(.);
  } >sram0_seg :sram0_phdr

  .bss (NOLOAD) : ALIGN(8)
  {
    . = ALIGN (8);
    _bss_start = ABSOLUTE(.);
    *(.dynsbss)
    *(.sbss)
    *(.sbss.*)
    *(.gnu.linkonce.sb.*)
    *(.scommon)
    *(.sbss2)
    *(.sbss2.*)
    *(.gnu.linkonce.sb2.*)
    *(.dynbss)
    *(.bss)
    *(SORT(.bss.sort.*))
    KEEP (*(SORT(.bss.keepsort.*) .bss.keep.*))
    *(.bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    *(.clib.bss)
    *(.clib.percpu.bss)
    *(.rtos.percpu.bss)
    *(.rtos.bss)
    *(.sram.bss)
    . = ALIGN (8);
    _bss_end = ABSOLUTE(.);
    _end = ALIGN(0x8);
    PROVIDE(end = ALIGN(0x8));
    _stack_sentry = ALIGN(0x8);
    _memmap_seg_sram0_end = ALIGN(0x8);
  }  >dram0_seg :sram0_bss_phdr

  PROVIDE(__stack = 0x200000);
  _heap_sentry = 0x200000;

  _memmap_mem_sram_max = ABSOLUTE(.);

  .debug  0 :  { *(.debug) }
  .line  0 :  { *(.line) }
  .debug_srcinfo  0 :  { *(.debug_srcinfo) }
  .debug_sfnames  0 :  { *(.debug_sfnames) }
  .debug_aranges  0 :  { *(.debug_aranges) }
  .debug_ranges   0 :  { *(.debug_ranges) }
  .debug_pubnames  0 :  { *(.debug_pubnames) }
  .debug_info  0 :  { *(.debug_info) }
  .debug_abbrev  0 :  { *(.debug_abbrev) }
  .debug_line  0 :  { *(.debug_line) }
  .debug_frame  0 :  { *(.debug_frame) }
  .debug_str  0 :  { *(.debug_str) }
  .debug_loc  0 :  { *(.debug_loc) }
  .debug_macinfo  0 :  { *(.debug_macinfo) }
  .debug_weaknames  0 :  { *(.debug_weaknames) }
  .debug_funcnames  0 :  { *(.debug_funcnames) }
  .debug_typenames  0 :  { *(.debug_typenames) }
  .debug_varnames  0 :  { *(.debug_varnames) }
  .xt.insn 0 :
  {
    KEEP (*(.xt.insn))
    KEEP (*(.gnu.linkonce.x.*))
  }
  .xt.prop 0 :
  {
    *(.xt.prop)
    *(.xt.prop.*)
    *(.gnu.linkonce.prop.*)
  }
  .xt.lit 0 :
  {
    *(.xt.lit)
    *(.xt.lit.*)
    *(.gnu.linkonce.p.*)
  }
  .xtensa.info 0 :
  {
    *(.xtensa.info)
  }
  .debug.xt.callgraph 0 :
  {
    KEEP (*(.debug.xt.callgraph .debug.xt.callgraph.* .gnu.linkonce.xt.callgraph.*))
  }
  .comment 0 :
  {
    KEEP(*(.comment))
  }
  .note.GNU-stack 0 :
  {
    *(.note.GNU-stack)
  }
}
