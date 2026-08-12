package main

/*
#cgo CFLAGS: -I../cpu/src
#cgo LDFLAGS: -L../cpu/bin -l6502 -Wl,-rpath,${SRCDIR}/../cpu/bin
#include <stdlib.h>
#include "cpu.h"
#include "mem.h"
*/
import "C"

import (
	"fmt"
	"unsafe"
)

const RAM_SIZE = C.RAM_SIZE

type CPUHandle struct {
	cpu *C.CPU
}

type CPUSnapshot struct {
	mem   []uint8
	pc    uint16
	sp    uint8
	accum uint8
	X     uint8
	Y     uint8
	flags uint8
}

func (snapshot *CPUSnapshot) Fill(handle CPUHandle) {

	cpu := handle.cpu
	snapshot.pc = uint16(cpu.pc)
	snapshot.sp = uint8(cpu.sp)
	snapshot.accum = uint8(cpu.accum)
	snapshot.X = uint8(cpu.X)
	snapshot.Y = uint8(cpu.Y)
	snapshot.flags = uint8(cpu.flags)

	snapshot.ReadMemory(handle)

}

func (snapshot CPUSnapshot) ReadMemory(handle CPUHandle) {

	if snapshot.mem == nil {
		return
	}

	slicePtr := unsafe.SliceData(snapshot.mem)
	memLen := C.uint16_t(len(snapshot.mem))

	C.mem_read_range(handle.cpu.mem, 0, memLen, (*C.uchar)(unsafe.Pointer(slicePtr)))
}

func NewSnapshot(memLength uint16) CPUSnapshot {
	snapshot := CPUSnapshot{nil, 0, 0, 0, 0, 0, 0}
	snapshot.mem = make([]uint8, memLength)

	return snapshot
}

func (handle CPUHandle) RunCPU(count int) {
	C.cpu_run_count(handle.cpu, C.uint64_t(count))
}

func (handle CPUHandle) PC() uint16 {
	return uint16(handle.cpu.pc)
}

func (handle CPUHandle) Reset() {
	C.cpu_reset(handle.cpu)
}

func (handle CPUHandle) LoadProgram(path string) error {
	cPath := C.CString(path)
	defer C.free(unsafe.Pointer(cPath))

	if rc := C.load_program(handle.cpu.mem, cPath); rc != 0 {
		return fmt.Errorf("load_program(%q) failed with %d", path, int(rc))
	}
	return nil
}

func NewCPU() CPUHandle {
	c_cpu := C.init_cpu()
	snapshot := CPUHandle{c_cpu}
	return snapshot
}

func (handle CPUHandle) Free() {
	C.free_cpu(handle.cpu)
}
