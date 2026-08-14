package components

import (
	"fmt"

	"charm.land/bubbles/v2/viewport"
	tea "charm.land/bubbletea/v2"
	"charm.land/lipgloss/v2"
)

const (
	addrGutterWidth = 5
	byteCellWidth   = 5
	maxBytesPerLine = 16
	headerLineCount = 1
	memorySize      = 1 << 16
)

type MemoryViewModel struct {
	viewport     viewport.Model
	data         []uint8
	viewReady    bool
	lineCount    int
	visibleLines int
	bytesPerLine int
	startAddr    int
	firstVisible int
}

type UpdateStartAddrMsg struct {
	NewAddr uint16
}

func bytesPerLineFor(width int) int {
	fits := (width - addrGutterWidth) / byteCellWidth
	return min(max(fits, 1), maxBytesPerLine)
}

func lineCountFor(height int) int {
	return max(height-headerLineCount, 1)
}

func (m MemoryViewModel) Init() tea.Cmd {
	return nil
}

func (m MemoryViewModel) Update(msg tea.Msg) (tea.Model, tea.Cmd) {

	var viewportCmd tea.Cmd
	var cmds []tea.Cmd

	switch msg := msg.(type) {
	case tea.WindowSizeMsg:
		m.bytesPerLine = bytesPerLineFor(msg.Width)
		m.visibleLines = lineCountFor(msg.Height)
		m.lineCount = m.visibleLines * 2 // double line count for scrolling effect

		if !m.viewReady {
			m.viewport = viewport.New(
				viewport.WithWidth(msg.Width),
				viewport.WithHeight(m.visibleLines),
			)
			RemoveShortcut(&m.viewport.KeyMap.PageDown, "space")
			m.viewReady = true
		} else {
			m.viewport.SetWidth(msg.Width)
			m.viewport.SetHeight(m.visibleLines)
		}

		m.reanchor()
		m.viewport.SetContent(m.DataToString())
	}

	yOffset := m.viewport.YOffset()
	m.viewport, viewportCmd = m.viewport.Update(msg)
	cmds = append(cmds, viewportCmd)

	if delta := m.viewport.YOffset() - yOffset; delta != 0 {
		previous := m.startAddr
		m.scrollLines(delta)

		if m.startAddr != previous {
			addr := uint16(m.startAddr)
			cmds = append(cmds, func() tea.Msg {
				return UpdateStartAddrMsg{NewAddr: addr}
			})
		}

		m.viewport.SetContent(m.DataToString())
	}

	return m, tea.Batch(cmds...)
}

// scrollLines moves the visible window by whole lines and re-derives the
// buffer that has to be resident to satisfy it.
func (m *MemoryViewModel) scrollLines(lines int) {
	if m.bytesPerLine <= 0 {
		return
	}

	maxFirstVisible := memorySize - m.VisibleBytes()
	m.firstVisible = min(max(m.firstVisible+lines*m.bytesPerLine, 0), max(maxFirstVisible, 0))
	m.reanchor()
}

func (m *MemoryViewModel) reanchor() {
	if m.bytesPerLine <= 0 {
		return
	}

	margin := (m.TotalBytes() - m.VisibleBytes()) / 2
	maxStart := max(memorySize-m.TotalBytes(), 0)

	m.startAddr = min(max(m.firstVisible-margin, 0), maxStart)
	m.startAddr -= m.startAddr % m.bytesPerLine

	m.viewport.SetYOffset((m.firstVisible - m.startAddr) / m.bytesPerLine)
}

/*
The viewport buffers twice the number of visible bytes.
This function will return the address of the first byte to be buffered
by the viewport

The nonvisible buffered data should be split above and below visible data.
If visible data is close to either end of total memory,
truncate the buffer on that side and give that space to other side
*/
func (m MemoryViewModel) GetFirstBufferedAddress() uint16 {
	return uint16(m.startAddr)
}

func (m *MemoryViewModel) SetData(data []uint8) {
	m.data = data
	if m.viewReady {
		m.viewport.SetContent(m.DataToString())
	}
}

func (m MemoryViewModel) TotalBytes() int {
	return m.bytesPerLine * m.lineCount
}

func (m MemoryViewModel) VisibleBytes() int {
	return m.bytesPerLine * m.visibleLines
}

func (m MemoryViewModel) View() tea.View {
	v := tea.NewView("")
	if !m.viewReady {
		v.SetContent("Initializing...")
	} else {
		v.SetContent(lipgloss.JoinVertical(lipgloss.Top, m.HeaderString(), m.viewport.View()))
	}
	return v
}

func (m MemoryViewModel) HeaderString() string {
	if m.bytesPerLine <= 0 {
		return ""
	}

	header := "     "
	for i := range m.bytesPerLine {
		header += fmt.Sprintf("  %02X ", i)
	}

	return columnHeaderStyle.Render(header)
}

func (m MemoryViewModel) DataToString() string {
	if m.bytesPerLine <= 0 || m.lineCount <= 0 {
		return ""
	}

	byte_lines := make([]string, 0, m.lineCount)

	for i := range m.lineCount {
		offset := i * m.bytesPerLine
		if offset >= len(m.data) {
			break
		}

		line := addrStyle.Render(fmt.Sprintf("%04X", m.startAddr+offset)) + " "
		for j := range m.bytesPerLine {
			addr := offset + j
			if addr >= len(m.data) {
				break
			}
			line += "  " + renderByte(m.data[addr]) + " "
		}

		byte_lines = append(byte_lines, line)
	}

	return lipgloss.JoinVertical(lipgloss.Top, byte_lines...)
}
