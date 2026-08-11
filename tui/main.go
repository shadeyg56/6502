package main

import (
	"fmt"
	"log"
	"os"

	"charm.land/bubbles/v2/key"
	tea "charm.land/bubbletea/v2"
)

type model struct {
	snapshotChan chan CPUSnapshot
	snapshot     CPUSnapshot
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
	}

	return m, nil
}

// flagString renders the P register as NV-BDIZC, with a dot for each clear bit.
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

func (m model) View() tea.View {

	if m.snapshot.mem == nil {
		return tea.NewView("waiting for first snapshot...")
	}

	s := m.snapshot
	body := fmt.Sprintf(
		"6502\n\n"+
			"PC  $%04X\n"+
			"SP  $%02X\n"+
			"A   $%02X\n"+
			"X   $%02X\n"+
			"Y   $%02X\n"+
			"P   $%02X  %s\n\n"+
			"ctrl+c quit\n",
		s.pc, s.sp, s.accum, s.X, s.Y, s.flags, flagString(s.flags),
	)

	return tea.NewView(body)
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
