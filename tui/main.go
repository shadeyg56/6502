package main

import (
	"charm.land/bubbles/v2/key"
	tea "charm.land/bubbletea/v2"
)

type model struct {
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

	return model{}
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
	}

	return m, nil
}

func (m model) View() tea.View {

	return tea.NewView("Hello, World")
}

func main() {
	m := initialModel()
	program := tea.NewProgram(m)
	program.Run()
}
