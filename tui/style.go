package main

import (
	"path/filepath"

	"charm.land/lipgloss/v2"
)

var (
	tileStyle = lipgloss.NewStyle().
			Border(lipgloss.RoundedBorder()).
			BorderForeground(lipgloss.Color("63")).
			Padding(0, 1)

	titleStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color("63"))

	placeholderStyle = lipgloss.NewStyle().
				Faint(true).
				Foreground(lipgloss.Color("242")).
				Italic(true)

	pathDirStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color("245"))

	pathFileStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color("212"))
)

const (
	tileHorizontalChrome = 4
	tileVerticalChrome   = 4
	programTileHeight    = 5
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

func truncateLeft(s string, width int) string {
	if width <= 0 {
		return ""
	}

	runes := []rune(s)
	if len(runes) <= width {
		return s
	}
	if width == 1 {
		return "…"
	}

	return "…" + string(runes[len(runes)-(width-1):])
}

func renderPath(path string, width int) string {
	if width < 1 {
		return ""
	}

	dir, file := filepath.Split(path)
	if lipgloss.Width(file) >= width {
		return pathFileStyle.Render(truncateLeft(file, width))
	}

	return pathDirStyle.Render(truncateLeft(dir, width-lipgloss.Width(file))) +
		pathFileStyle.Render(file)
}

func renderPlaceholder(text string) string {
	return placeholderStyle.Render(text)
}
