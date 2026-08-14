package main

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sync/atomic"
	"time"

	"charm.land/bubbles/v2/filepicker"
	"charm.land/bubbles/v2/help"
	"charm.land/bubbles/v2/textinput"
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
	stackView         components.StackViewModel
	help              help.Model
	programPath       string
	programPathEntry  textinput.Model
	filePicker        filepicker.Model
	selectingFile     bool
}

type tickMsg time.Time

func initialModel(initialProgram string) model {

	fp := filepicker.New()
	fp.AllowedTypes = []string{".bin"}
	styleFilePicker(&fp.Styles)
	cwd, err := os.Getwd()
	if err == nil {
		fp.CurrentDirectory = cwd
	}
	if initialProgram != "" {
		initialProgram, _ = filepath.Abs(initialProgram)
	}

	helpModel := help.New()
	styleHelp(&helpModel.Styles)

	return model{
		snapshots:         &atomic.Pointer[CPUSnapshot]{},
		emu_cmd_channel:   make(chan EmuCMD),
		emulatorIsPlaying: false,
		help:              helpModel,
		filePicker:        fp,
		programPath:       initialProgram,
		stackView:         components.New(),
	}
}

func tick() tea.Cmd {
	return tea.Tick(RefreshInterval, func(t time.Time) tea.Msg {
		return tickMsg(t)
	})
}

func (m model) Init() tea.Cmd {

	cmds := []tea.Cmd{tick(), m.filePicker.Init()}

	if m.programPath != "" {
		cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, LoadProgram{m.programPath}))
	}

	return tea.Batch(cmds...)
}

func (m model) Update(msg tea.Msg) (tea.Model, tea.Cmd) {

	var cmds []tea.Cmd

	switch msg := msg.(type) {
	case tea.KeyPressMsg:

		// While the picker is open it owns the keyboard, apart from quit.
		if m.selectingFile {
			if DefaultKeyMap.Action(msg) == ActionQuit {
				return m, tea.Quit
			}
			break
		}

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
		case ActionToggleHelp:
			// The help section grows and shrinks, so the tiles below it have
			// to be resized whenever it is toggled.
			m.help.ShowAll = !m.help.ShowAll
			resized, cmd := m.propagateTileSizes()
			m = resized
			cmds = append(cmds, cmd)
		case ActionSelectProgram:
			m.selectingFile = true
		}
		updated, cmd := m.memoryView.Update(msg)
		cmds = append(cmds, cmd)
		m.memoryView = updated.(components.MemoryViewModel)
	case tickMsg:
		if latest := m.snapshots.Load(); latest != nil {
			m.snapshot = *latest
			m.memoryView.SetData(m.snapshot.mem)
			m.stackView.SetData(m.snapshot.stack, m.snapshot.sp)
		}
		return m, tick()

	case tea.WindowSizeMsg:
		m.width = msg.Width
		m.height = msg.Height
		m.help.SetWidth(msg.Width)

		resized, cmd := m.propagateTileSizes()
		m = resized
		rangeCMD := sendEmuCmd(m.emu_cmd_channel, SetMemoryRange{uint16(m.memoryView.TotalBytes())})
		cmds = append(cmds, cmd, rangeCMD)

	case components.UpdateStartAddrMsg:
		log.Println("Start address changed", msg.NewAddr)
		cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, SetMemoryAddr{msg.NewAddr}))
	}

	_, isKeyPress := msg.(tea.KeyPressMsg)
	if m.selectingFile || !isKeyPress {
		var filePickerCmd tea.Cmd
		m.filePicker, filePickerCmd = m.filePicker.Update(msg)
		cmds = append(cmds, filePickerCmd)

		if didSelect, path := m.filePicker.DidSelectFile(msg); didSelect {
			m.programPath = path
			m.selectingFile = false
			cmds = append(cmds, sendEmuCmd(m.emu_cmd_channel, LoadProgram{path}))
		}
	}

	if _, isResize := msg.(tea.WindowSizeMsg); !isResize {
		updatedStackView, cmd := m.stackView.Update(msg)
		m.stackView = updatedStackView.(components.StackViewModel)
		cmds = append(cmds, cmd)
	}

	return m, tea.Batch(cmds...)
}

func (m model) cpuStatusBody() string {
	if m.snapshot.mem == nil {
		return renderPlaceholder("waiting for first snapshot...")
	}

	s := m.snapshot
	return stackStrings(
		renderRegister("PC", fmt.Sprintf("$%04X", s.pc)),
		renderRegister("SP", fmt.Sprintf("$%02X", s.sp)),
		renderRegister("A ", fmt.Sprintf("$%02X", s.accum)),
		renderRegister("X ", fmt.Sprintf("$%02X", s.X)),
		renderRegister("Y ", fmt.Sprintf("$%02X", s.Y)),
		renderRegister("P ", fmt.Sprintf("$%02X", s.flags))+"  "+renderFlags(s.flags),
		"",
		renderRunState(m.emulatorIsPlaying),
	)
}

func (m model) helpView() string {
	return m.help.View(DefaultKeyMap)
}

// contentHeight is the space left for the tiles once the help section at the
// bottom has taken its share.
func (m model) contentHeight() int {
	return max(m.height-heightOf(m.helpView()), 1)
}

const (
	cpuTileKey     = "cpu"
	memoryTileKey  = "memory"
	programTileKey = "program"
	stackTileKey   = "stack"

	cpuWidthWeight    = 1
	memoryWidthWeight = 3
	stackWidthWeight  = 1
)

func (m model) programBody(width int, _ int) string {
	if m.programPath == "" {
		return renderPlaceholder("no program loaded — ctrl+o to browse")
	}

	return renderPath(m.programPath, width)
}

func (m model) layout() Tile {
	return joinTiles(1, 1,
		NewTile(cpuTileKey, "CPU Status", StaticBody(m.cpuStatusBody()), cpuWidthWeight, 1).
			WithAccent(accentCPU),
		stackSections(memoryWidthWeight, 1,
			NewTile(memoryTileKey, "Memory", StaticBody(m.memoryView.View().Content), 1, 1).
				WithAccent(accentMemory),
			NewTile(programTileKey, "Program", m.programBody, 1, 0).
				WithAccent(accentProgram).
				WithFixedHeight(programTileHeight),
		),
		NewTile(stackTileKey, "Stack", StaticBody(m.stackView.View().Content), stackWidthWeight, 1).
			WithAccent(accentStack),
	)
}

func (m model) tileSizes() map[string]Size {
	return MeasureTiles(m.layout(), m.width, m.contentHeight())
}

// sizedComponent ties a tile in the layout to the component that renders into
// it. Adding a component to the layout means adding one entry here.
type sizedComponent struct {
	key    string
	resize func(m *model, msg tea.Msg) tea.Cmd
}

var sizedComponents = []sizedComponent{
	{
		key: memoryTileKey,
		resize: func(m *model, msg tea.Msg) tea.Cmd {
			updated, cmd := m.memoryView.Update(msg)
			m.memoryView = updated.(components.MemoryViewModel)
			return cmd
		},
	},
	{
		key: stackTileKey,
		resize: func(m *model, msg tea.Msg) tea.Cmd {
			updated, cmd := m.stackView.Update(msg)
			m.stackView = updated.(components.StackViewModel)
			return cmd
		},
	},
}

// propagateTileSizes measures the layout once and tells every component how
// much room its tile was given, so components size themselves to what will
// actually be drawn rather than to the terminal.
func (m model) propagateTileSizes() (model, tea.Cmd) {
	sizes := m.tileSizes()

	var cmds []tea.Cmd
	for _, component := range sizedComponents {
		size, ok := sizes[component.key]
		if !ok {
			continue
		}

		cmds = append(cmds, component.resize(&m, tea.WindowSizeMsg{
			Width:  size.Width,
			Height: size.Height,
		}))
	}

	return m, tea.Batch(cmds...)
}

func (m model) View() tea.View {
	view := tea.NewView("")
	view.AltScreen = true

	if m.width == 0 || m.height == 0 {
		view.Content = "initializing..."
		return view
	}

	if m.selectingFile {
		view.Content = m.filePicker.View()
		return view
	}

	view.Content = stackStrings(
		RenderTile(m.layout(), m.width, m.contentHeight()),
		m.helpView(),
	)
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

	programPath := ""
	if len(os.Args) >= 2 {
		programPath = os.Args[1]
	}

	m := initialModel(programPath)
	program := tea.NewProgram(m)
	log_file := StartLogger()
	defer log_file.Close()

	log.Println("Starting 6502 TUI application...")

	go EmulatorHandler(m.snapshots, m.emu_cmd_channel)

	program.Run()
}
