package main

import (
	"fmt"
	"log"
	"os"

	"charm.land/bubbles/v2/key"
	tea "charm.land/bubbletea/v2"
	"charm.land/lipgloss/v2"
)

type model struct {
	snapshotChan chan CPUSnapshot
	snapshot     CPUSnapshot
	width        int
	height       int
}

type KeyMap struct {
	CtrlC key.Binding
}

var DefaultKeyMap = KeyMap{
	CtrlC: key.NewBinding(
		key.WithKeys("ctrl+c"),
	),
}

func initialModel() model {
	return model{
		snapshotChan: make(chan CPUSnapshot),
	}
}

func (m model) Init() tea.Cmd {
	return nil
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {

	switch msg := msg.(type) {
	case tea.KeyPressMsg:
		switch {
		case key.Matches(msg, DefaultKeyMap.CtrlC):
			return m, tea.Quit
		}
	case CPUSnapshot:
		m.snapshot = msg

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
			byte_lines[i] += fmt.Sprintf("  %02X ", s.mem[j])
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

	go RunEmulator(m.snapshotChan)

	go func() {
		for snapshot := range m.snapshotChan {
			program.Send(snapshot)
		}
	}()

	program.Run()
}
