package main

import (
	"runtime"
	"sync/atomic"
)

func RunEmulator(snapshot_pointer *atomic.Pointer[CPUSnapshot]) {
	runtime.LockOSThread()
	cpuHandle := NewCPU()
	cpuHandle.LoadProgram("../cpu/6502_functional_test.bin")
	cpuHandle.cpu.pc = 0x0400
	defer cpuHandle.Free()

	for {
		snapshot := NewSnapshot(80)
		cpuHandle.RunCPU(1)
		snapshot.Fill(cpuHandle)

		snapshot_pointer.Store(&snapshot)
		if snapshot.pc == 0x3469 {
			break
		}
		//time.Sleep(10 * time.Millisecond)
	}
}
