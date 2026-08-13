package main

import (
	"charm.land/bubbles/v2/key"
	tea "charm.land/bubbletea/v2"
)

type Action int

const (
	ActionNone Action = iota
	ActionQuit
	ActionStep
	ActionTogglePlay
	ActionToggleHelp
)

type KeyMap struct {
	CtrlC  key.Binding
	Enter  key.Binding
	Space  key.Binding
	Help   key.Binding
	Scroll key.Binding
}

var DefaultKeyMap = KeyMap{
	CtrlC: key.NewBinding(
		key.WithKeys("ctrl+c"),
		key.WithHelp("ctrl+c", "quit"),
	),
	Enter: key.NewBinding(
		key.WithKeys("enter"),
		key.WithHelp("enter", "step"),
	),
	Space: key.NewBinding(
		key.WithKeys("space"),
		key.WithHelp("space", "play/pause"),
	),
	Help: key.NewBinding(
		key.WithKeys("?"),
		key.WithHelp("?", "toggle help"),
	),
	Scroll: key.NewBinding(
		key.WithKeys("up", "down", "pgup", "pgdown"),
		key.WithHelp("↑/↓/pgup/pgdn", "scroll memory"),
	),
}

func (kMap KeyMap) ShortHelp() []key.Binding {
	return []key.Binding{kMap.Space, kMap.Enter, kMap.Help, kMap.CtrlC}
}

func (kMap KeyMap) FullHelp() [][]key.Binding {
	return [][]key.Binding{
		{kMap.Space, kMap.Enter},
		{kMap.Scroll},
		{kMap.Help, kMap.CtrlC},
	}
}

func (kMap KeyMap) Action(msg tea.KeyPressMsg) Action {
	switch {
	case key.Matches(msg, kMap.CtrlC):
		return ActionQuit
	case key.Matches(msg, kMap.Enter):
		return ActionStep
	case key.Matches(msg, kMap.Space):
		return ActionTogglePlay
	case key.Matches(msg, kMap.Help):
		return ActionToggleHelp
	}

	return ActionNone
}

func sendEmuCmd(ch chan EmuCMD, cmd EmuCMD) tea.Cmd {
	return func() tea.Msg {
		ch <- cmd
		return nil
	}
}
