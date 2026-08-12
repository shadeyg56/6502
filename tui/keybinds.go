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
)

type KeyMap struct {
	CtrlC key.Binding
	Enter key.Binding
	Space key.Binding
}

var DefaultKeyMap = KeyMap{
	CtrlC: key.NewBinding(
		key.WithKeys("ctrl+c"),
	),
	Enter: key.NewBinding(
		key.WithKeys("enter"),
	),
	Space: key.NewBinding(
		key.WithKeys("space"),
	),
}

func (kMap KeyMap) Action(msg tea.KeyPressMsg) Action {
	switch {
	case key.Matches(msg, kMap.CtrlC):
		return ActionQuit
	case key.Matches(msg, kMap.Enter):
		return ActionStep
	case key.Matches(msg, kMap.Space):
		return ActionTogglePlay
	}

	return ActionNone
}

func sendEmuCmd(ch chan EmuCMD, cmd EmuCMD) tea.Cmd {
	return func() tea.Msg {
		ch <- cmd
		return nil
	}
}
