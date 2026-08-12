package main

import (
	"runtime"
	"sync/atomic"
)

// Interface with dummy method acts as type guard
type EmuCMD interface{ isCommand() }

type StepEmulator struct {
	step int
}

func (StepEmulator) isCommand() {}

type PlayEmulator struct{}

func (PlayEmulator) isCommand() {}

type StopEmulator struct{}

func (StopEmulator) isCommand() {}

func RunEmulator(snapshot_pointer *atomic.Pointer[CPUSnapshot], emu_chan chan EmuCMD) {
	runtime.LockOSThread()
	cpuHandle := NewCPU()
	cpuHandle.LoadProgram("../cpu/6502_functional_test.bin")
	cpuHandle.cpu.pc = 0x0400
	defer cpuHandle.Free()

	for {

		for {
			select {
			case cmd := <-emu_chan:
				switch cmd.(type) {
				case StepEmulator:
					snapshot := NewSnapshot(80)
					cpuHandle.RunCPU(1)
					snapshot.Fill(cpuHandle)

					snapshot_pointer.Store(&snapshot)
					if snapshot.pc == 0x3469 {
						break
					}
				}
			default:
				break
			}
		}

		//time.Sleep(10 * time.Millisecond)
	}
}
