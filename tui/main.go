package main

import (
	"fmt"
	"log"
	"os"
	"sync/atomic"
	"time"

	"charm.land/bubbles/v2/key"
	tea "charm.land/bubbletea/v2"
	"charm.land/lipgloss/v2"
)

const RefreshInterval = 16 * time.Millisecond

type model struct {
	snapshots       *atomic.Pointer[CPUSnapshot]
	snapshot        CPUSnapshot
	emu_cmd_channel chan EmuCMD
	width           int
	height          int
}

type tickMsg time.Time

type KeyMap struct {
	CtrlC key.Binding
	Plus  key.Binding
}

var DefaultKeyMap = KeyMap{
	CtrlC: key.NewBinding(
		key.WithKeys("ctrl+c"),
	),
	Plus: key.NewBinding(
		key.WithKeys("enter"),
	),
}

func initialModel() model {
	return model{
		snapshots:       &atomic.Pointer[CPUSnapshot]{},
		emu_cmd_channel: make(chan EmuCMD),
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

	switch msg := msg.(type) {
	case tea.KeyPressMsg:
		switch {
		case key.Matches(msg, DefaultKeyMap.CtrlC):
			return m, tea.Quit
		case key.Matches(msg, DefaultKeyMap.Plus):
			m.emu_cmd_channel <- StepEmulator{}
			return m, nil
		}
	case tickMsg:
		if latest := m.snapshots.Load(); latest != nil {
			m.snapshot = *latest
		}
		return m, tick()

	case tea.WindowSizeMsg:
		m.width = msg.Width
		m.height = msg.Height
	}

	return m, nil
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

func (m model) memoryViewBody(lines int) string {
	if m.snapshot.mem == nil {
		return "waiting for first snapshot..."
	}

	s := m.snapshot
	byte_lines := make([]string, lines)
	header_line := "     "
	for i := range 16 {
		header_line += fmt.Sprintf("  %02X ", i)
	}

	for i := range lines {
		byte_lines[i] += fmt.Sprintf("%04X ", i*16)
		for j := range 16 {
			byte_lines[i] += fmt.Sprintf("  %02X ", s.mem[(i*16)+j])
		}

	}

	return lipgloss.JoinVertical(lipgloss.Top, append([]string{header_line}, byte_lines...)...)
}

func (m model) View() tea.View {
	view := tea.NewView("")
	view.AltScreen = true

	if m.width == 0 || m.height == 0 {
		view.Content = "initializing..."
		return view
	}

	leftWidth := m.width / 4
	rightWidth := m.width - leftWidth

	cpuTile := tile(leftWidth, m.height/2, "CPU Status", m.cpuStatusBody())
	memTile := tile(rightWidth, m.height, "Memory", m.memoryViewBody(5))

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

	go RunEmulator(m.snapshots, m.emu_cmd_channel)

	program.Run()
}
