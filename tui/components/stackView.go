package components

import (
	"fmt"
	"log"
	"strings"

	"charm.land/bubbles/v2/viewport"
	tea "charm.land/bubbletea/v2"
)

type StackViewModel struct {
	data      []uint8
	sp        uint8
	viewport  viewport.Model
	viewReady bool
}

func New() StackViewModel {
	return StackViewModel{
		data: make([]uint8, 256),
		sp:   0,
	}
}

func (m StackViewModel) Init() tea.Cmd {
	return nil
}

func (m StackViewModel) Update(msg tea.Msg) (tea.Model, tea.Cmd) {

	switch msg := msg.(type) {
	case tea.WindowSizeMsg:

		if !m.viewReady {
			m.viewport = viewport.New(
				viewport.WithWidth(msg.Width),
				viewport.WithHeight(msg.Height),
			)
			m.viewReady = true
		} else {
			m.viewport.SetWidth(msg.Width)
			m.viewport.SetHeight(msg.Height)
		}

		m.viewport.SetContent(m.DataToString())
	}

	cmds := []tea.Cmd{}
	var viewportCmd tea.Cmd

	_, isKeyPress := msg.(tea.KeyPressMsg)
	// do not forward key press events to stack viewport
	if !isKeyPress {
		m.viewport, viewportCmd = m.viewport.Update(msg)
		cmds = append(cmds, viewportCmd)
	}

	return m, tea.Batch(cmds...)

}

func (m StackViewModel) View() tea.View {
	v := tea.NewView("")

	v.SetContent(m.viewport.View())

	return v

}

func (m *StackViewModel) SetData(data []uint8, pointer uint8) {
	m.data = data
	m.sp = pointer
	if m.viewReady {
		m.viewport.SetContent(m.DataToString())
		m.viewport.GotoBottom()
	}
}

func (m StackViewModel) DataToString() string {
	data := m.data
	builder := strings.Builder{}
	for i := range len(data) {
		log.Println(i)
		line := fmt.Sprintf("$%02x\t%02x", i, data[i])
		builder.WriteString(line)
		if m.sp == uint8(i) {
			builder.WriteString("  <-----")
		}
		builder.WriteString("\n")
	}

	return builder.String()
}
