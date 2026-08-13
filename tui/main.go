package main

import (
	"fmt"
	"log"
	"os"
	"sync/atomic"
	"time"

	tea "charm.land/bubbletea/v2"

	"650tea/components"
)

const RefreshInterval = 16 * time.Millisecond

type model struct {
	snapshots         *atomic.Pointer[CPUSnapshot]
	snapshot          CPUSnapshot
	emu_cmd_channel   chan EmuCMD
	width             int
	height            int
	emulatorIsPlaying bool
	memoryView        components.MemoryViewModel
}

type tickMsg time.Time

func initialModel() model {
	return model{
		snapshots:         &atomic.Pointer[CPUSnapshot]{},
		emu_cmd_channel:   make(chan EmuCMD),
		emulatorIsPlaying: false,
	}
}

func tick() tea.Cmd {
	return tea.Tick(RefreshInterval, func(t time.Time) tea.Msg {
		return tickMsg(t)
	})
}

func (m model) Init() tea.Cmd {
	return tick()
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {

	var cmds []tea.Cmd

	switch msg := msg.(type) {
	case tea.KeyPressMsg:
		switch DefaultKeyMap.Action(msg) {
		case ActionQuit:
			return m, tea.Quit
		case ActionStep:
			cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, StepEmulator{1}))
		case ActionTogglePlay:
			if m.emulatorIsPlaying {
				m.emulatorIsPlaying = false
				cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, StopEmulator{}))
				break
			}
			m.emulatorIsPlaying = true
			cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, PlayEmulator{}))
		}
		updated, cmd := m.memoryView.Update(msg)
		cmds = append(cmds, cmd)
		m.memoryView = updated.(components.MemoryViewModel)
	case tickMsg:
		if latest := m.snapshots.Load(); latest != nil {
			m.snapshot = *latest
			m.memoryView.SetData(m.snapshot.mem)
		}
		return m, tick()

	case tea.WindowSizeMsg:
		m.width = msg.Width
		m.height = msg.Height

		memWidth, memHeight := m.memoryTileInner()
		updated, cmd := m.memoryView.Update(tea.WindowSizeMsg{
			Width:  memWidth,
			Height: memHeight,
		})
		m.memoryView = updated.(components.MemoryViewModel)
		rangeCMD := sendEmuCmd(m.emu_cmd_channel, SetMemoryRange{uint16(m.memoryView.TotalBytes())})
		cmds = append(cmds, cmd, rangeCMD)

	case components.UpdateStartAddrMsg:
		log.Println("Start address changed", msg.NewAddr)
		cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, SetMemoryAddr{msg.NewAddr}))
	}

	return m, tea.Batch(cmds...)
}

func flagString(flags uint8) string {
	const names = "NV-BDIZC"

	out := make([]byte, len(names))
	for i := range names {
		if flags&(uint8(1)<<(7-i)) != 0 {
			out[i] = names[i]
		} else {
			out[i] = '.'
		}
	}

	return string(out)
}

func (m model) cpuStatusBody() string {
	if m.snapshot.mem == nil {
		return "waiting for first snapshot..."
	}

	s := m.snapshot
	return fmt.Sprintf(
		"PC  $%04X\n"+
			"SP  $%02X\n"+
			"A   $%02X\n"+
			"X   $%02X\n"+
			"Y   $%02X\n"+
			"P   $%02X  %s",
		s.pc, s.sp, s.accum, s.X, s.Y, s.flags, flagString(s.flags),
	)
}

func (m model) memoryTileSize() (int, int) {
	return m.width - (m.width / 4), m.height
}

func (m model) memoryTileInner() (int, int) {
	width, height := m.memoryTileSize()
	return max(width-tileHorizontalChrome, 1), max(height-tileVerticalChrome, 1)
}

func (m model) View() tea.View {
	view := tea.NewView("")
	view.AltScreen = true

	if m.width == 0 || m.height == 0 {
		view.Content = "initializing..."
		return view
	}

	leftWidth := m.width / 4
	rightWidth, rightHeight := m.memoryTileSize()

	cpuTile := tile(leftWidth, m.height/2, "CPU Status", m.cpuStatusBody())
	memTile := tile(rightWidth, rightHeight, "Memory", m.memoryView.View().Content)

	view.Content = joinTiles(cpuTile, memTile)
	return view
}

func StartLogger() *os.File {
	f, err := tea.LogToFile("debug.log", "debug")
	if err != nil {
		fmt.Println("Error occured when starting logger:", err)
	}

	return f
}

func main() {
	m := initialModel()
	program := tea.NewProgram(m)
	log_file := StartLogger()
	defer log_file.Close()

	log.Println("Starting 6502 TUI application...")

	go EmulatorHandler(m.snapshots, m.emu_cmd_channel)

	program.Run()
}
