package main

import (
	"context"
	"log"
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

/*
Goroutine responsible for handling messages sent over provided channel
Anything related to controlling the emulator from TUI passes through this function

Caller provides an atomic pointer that CPU data is passed through
*/
func EmulatorHandler(snapshot_pointer *atomic.Pointer[CPUSnapshot], emu_chan chan EmuCMD) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	cpuHandle := NewCPU()
	cpuHandle.LoadProgram("../cpu/6502_functional_test.bin")
	cpuHandle.cpu.pc = 0x0400
	defer cpuHandle.Free()
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	for cmd := range emu_chan {
		log.Printf("Got emu cmd: %T\n", cmd)
		switch c := cmd.(type) {
		case StepEmulator:
			go runEmulator(ctx, cpuHandle, snapshot_pointer, c.step)
		case PlayEmulator:
			go runEmulator(ctx, cpuHandle, snapshot_pointer, 0)
		case StopEmulator:
			cancel()
			ctx, cancel = context.WithCancel(context.Background())
			defer cancel()
		}
	}

	//time.Sleep(10 * time.Millisecond)
}

/*
Command function to run emulator either continuously if steps <= 0 or to step a discrete number of instructions.
Copies CPU status into snapshot pointer after the steps have finished.
If running continously, the snapshot is copied after every single step
*/
func runEmulator(ctx context.Context, cpuHandle CPUHandle, snapshop_pointer *atomic.Pointer[CPUSnapshot], steps int) {
	for {
		// Handle task cancellation
		select {
		case <-ctx.Done():
			return
		default:
		}

		runSteps := steps
		if runSteps <= 0 {
			runSteps = 1
		}

		snapshot := NewSnapshot((1 << 16) - 1)
		cpuHandle.RunCPU(runSteps)
		snapshot.Fill(cpuHandle)
		snapshop_pointer.Store(&snapshot)

		if steps > 0 {
			return
		}
	}
}
