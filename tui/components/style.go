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

	pointerStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("212")).
			Bold(true)

	placeholderStyle = lipgloss.NewStyle().
				Foreground(lipgloss.Color("240")).
				Italic(true)
)

func renderByte(value uint8) string {
	if value == 0 {
		return zeroByteStyle.Render("00")
	}

	return byteStyle.Render(lipgloss.NewStyle().Render(hexByte(value)))
}

func hexByte(value uint8) string {
	const digits = "0123456789ABCDEF"
	return string([]byte{digits[value>>4], digits[value&0x0F]})
}
