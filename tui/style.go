package main

import "charm.land/lipgloss/v2"

var (
	tileStyle = lipgloss.NewStyle().
			Border(lipgloss.RoundedBorder()).
			BorderForeground(lipgloss.Color("63")).
			Padding(0, 1)

	titleStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color("63"))
)

const (
	tileHorizontalChrome = 4
	tileVerticalChrome   = 4
)

func tile(outerWidth, outerHeight int, title, body string) string {
	width := max(outerWidth, 4)
	height := max(outerHeight, 3)

	content := titleStyle.Render(title)
	if body != "" {
		content += "\n\n" + body
	}

	return tileStyle.Width(width).Height(height).Render(content)
}

func joinTiles(tiles ...string) string {
	return lipgloss.JoinHorizontal(lipgloss.Top, tiles...)
}

func stackSections(sections ...string) string {
	return lipgloss.JoinVertical(lipgloss.Left, sections...)
}

func heightOf(s string) int {
	return lipgloss.Height(s)
}
