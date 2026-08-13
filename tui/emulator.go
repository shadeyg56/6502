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

type SetMemoryRange struct {
	memoryRange uint16
}

func (SetMemoryRange) isCommand() {}

type SetMemoryAddr struct {
	address uint16
}

func (SetMemoryAddr) isCommand() {}

type LoadProgram struct {
	path string
}

func (LoadProgram) isCommand() {}

type ResetCPU struct{}

func (ResetCPU) isCommand() {}

/*
Dunno if its an anti pattern for bubble tea for goroutine to maintain this state or not
I think its ok since its seperate from model state (we aren't directly touching any model state in emulator goroutines)
*/
type EmuState struct {
	isPlaying     bool
	ctx           context.Context
	cancel        context.CancelFunc
	memoryRange   uint16
	program       string
	memoryAddress uint16
}

/*
Goroutine responsible for handling messages sent over provided channel
Anything related to controlling the emulator from TUI passes through this function

Caller provides an atomic pointer that CPU data is passed through
*/
func EmulatorHandler(snapshot_pointer *atomic.Pointer[CPUSnapshot], emu_chan chan EmuCMD) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	emuState := EmuState{
		isPlaying:     false,
		memoryRange:   (1 << 16) - 1,
		memoryAddress: 0x0,
	}

	cpuHandle := NewCPU()
	//cpuHandle.LoadProgram("../cpu/6502_functional_test.bin")
	//cpuHandle.cpu.pc = 0x0400
	defer cpuHandle.Free()
	emuState.ctx, emuState.cancel = context.WithCancel(context.Background())
	defer emuState.cancel()

	for cmd := range emu_chan {
		log.Printf("Got emu cmd: %T\n", cmd)
		switch c := cmd.(type) {
		case StepEmulator:
			go runEmulator(emuState.ctx, cpuHandle, snapshot_pointer, c.step, &emuState.memoryRange, &emuState.memoryAddress)
		case PlayEmulator:
			go runEmulator(emuState.ctx, cpuHandle, snapshot_pointer, 0, &emuState.memoryRange, &emuState.memoryAddress)
			emuState.isPlaying = true
		case StopEmulator:
			stopEmulator(&emuState)
			defer emuState.cancel()
		case SetMemoryRange:
			emuState.memoryRange = c.memoryRange
			if !emuState.isPlaying {
				snapshot := snapshot_pointer.Load()
				if snapshot == nil {
					continue
				}
				clonedSnapshot := snapshot.Clone(emuState.memoryRange)
				clonedSnapshot.ReadMemory(cpuHandle, emuState.memoryAddress)
				snapshot_pointer.Store(&clonedSnapshot)
			}
		case SetMemoryAddr:
			emuState.memoryAddress = c.address
		case LoadProgram:
			stopEmulator(&emuState)
			cpuHandle.LoadProgram(c.path)
			cpuHandle.Reset()
			snapshot := NewSnapshot(emuState.memoryRange)
			snapshot.Fill(cpuHandle, cpuHandle.PC())
			snapshot_pointer.Store(&snapshot)
		}

	}

	//time.Sleep(10 * time.Millisecond)
}

func stopEmulator(emuState *EmuState) {
	emuState.cancel()
	emuState.ctx, emuState.cancel = context.WithCancel(context.Background())
	emuState.isPlaying = false
}

/*
Command function to run emulator either continuously if steps <= 0 or to step a discrete number of instructions.
Copies CPU status into snapshot pointer after the steps have finished.
If running continously, the snapshot is copied after every single step
*/
func runEmulator(ctx context.Context, cpuHandle CPUHandle, snapshop_pointer *atomic.Pointer[CPUSnapshot], steps int, memoryRange *uint16, memoryAddress *uint16) {
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

		snapshot := NewSnapshot(uint16(*memoryRange))
		cpuHandle.RunCPU(runSteps)
		snapshot.Fill(cpuHandle, *memoryAddress)
		snapshop_pointer.Store(&snapshot)

		if steps > 0 {
			return
		}
	}
}
