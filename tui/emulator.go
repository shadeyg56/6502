package main

func RunEmulator(ch chan<- CPUSnapshot) {

	cpuHandle := NewCPU()
	cpuHandle.LoadProgram("../cpu/6502_functional_test.bin")
	cpuHandle.cpu.pc = 0x0400
	defer cpuHandle.Free()

	for {
		snapshot := NewSnapshot((1 << 16) - 1)
		cpuHandle.RunCPU(1)
		snapshot.Fill(cpuHandle)
		ch <- snapshot
		//time.Sleep(10 * time.Millisecond)
	}
}
