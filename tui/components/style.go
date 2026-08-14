package components

import "charm.land/lipgloss/v2"

var (
	addrStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("63")).
			Bold(true)

	columnHeaderStyle = lipgloss.NewStyle().
				Foreground(lipgloss.Color("245"))

	byteStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("252"))

	// Zero bytes are the overwhelming majority of memory, so they are dimmed
	// to let the bytes that actually hold something stand out.
	zeroByteStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("238"))

	stackAddrStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("42")).
			Bold(true)

	currentByteStyle = stackAddrStyle

	pointerStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("212")).
			Bold(true)

	placeholderStyle = lipgloss.NewStyle().
				Foreground(lipgloss.Color("240")).
				Italic(true)
)

// Styling a byte costs microseconds and a redraw touches thousands of them, so
// every possible result is built once up front and looked up by value.
var (
	byteCache        [256]string
	currentByteCache [256]string
)

func init() {
	for value := range 256 {
		text := hexByte(uint8(value))

		if value == 0 {
			byteCache[value] = zeroByteStyle.Render(text)
		} else {
			byteCache[value] = byteStyle.Render(text)
		}

		currentByteCache[value] = currentByteStyle.Render(text)
	}
}

func renderByte(value uint8) string {
	return byteCache[value]
}

// Render byte pointed to by program counter
func renderCurrentByte(value uint8) string {
	return currentByteCache[value]
}

func hexByte(value uint8) string {
	const digits = "0123456789ABCDEF"
	return string([]byte{digits[value>>4], digits[value&0x0F]})
}

func hexWord(value int) string {
	const digits = "0123456789ABCDEF"
	return string([]byte{
		digits[(value>>12)&0x0F],
		digits[(value>>8)&0x0F],
		digits[(value>>4)&0x0F],
		digits[value&0x0F],
	})
}
