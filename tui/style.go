package main

import (
	"path/filepath"

	"charm.land/bubbles/v2/filepicker"
	"charm.land/bubbles/v2/help"
	"charm.land/lipgloss/v2"
)

type helpStyles = help.Styles

type filePickerStyles = filepicker.Styles

// One accent per tile, plus shared colors for the values inside them.
const (
	accentCPU     = "212"
	accentMemory  = "63"
	accentProgram = "214"
	accentStack   = "42"
	accentDefault = "63"

	colorLabel = "245"
	colorValue = "252"
	colorDim   = "240"
	colorOn    = "84"
	colorOff   = "238"
)

func tileStyleFor(accent string) lipgloss.Style {
	return lipgloss.NewStyle().
		Border(lipgloss.RoundedBorder()).
		BorderForeground(lipgloss.Color(accent)).
		Padding(0, 1)
}

func titleStyleFor(accent string) lipgloss.Style {
	return lipgloss.NewStyle().
		Bold(true).
		Foreground(lipgloss.Color(accent))
}

var (
	tileStyle = tileStyleFor(accentDefault)

	titleStyle = titleStyleFor(accentDefault)

	labelStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color(colorLabel))

	valueStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color(colorValue))

	flagSetStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color(colorOn))

	flagClearStyle = lipgloss.NewStyle().
			Foreground(lipgloss.Color(colorOff))

	runningStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color(colorOn))

	pausedStyle = lipgloss.NewStyle().
			Bold(true).
			Foreground(lipgloss.Color(accentProgram))

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

func renderRegister(name string, value string) string {
	return labelStyle.Render(name) + "  " + valueStyle.Render(value)
}

func renderFlags(flags uint8) string {
	const names = "NV-BDIZC"

	out := ""
	for i := range names {
		char := string(names[i])
		if flags&(uint8(1)<<(7-i)) != 0 {
			out += flagSetStyle.Render(char)
			continue
		}
		out += flagClearStyle.Render(".")
	}

	return out
}

func renderRunState(playing bool) string {
	if playing {
		return runningStyle.Render("running")
	}

	return pausedStyle.Render("paused")
}

func styleHelp(styles *helpStyles) {
	styles.ShortKey = styles.ShortKey.Foreground(lipgloss.Color(accentCPU))
	styles.ShortDesc = styles.ShortDesc.Foreground(lipgloss.Color(colorLabel))
	styles.ShortSeparator = styles.ShortSeparator.Foreground(lipgloss.Color(colorDim))
	styles.FullKey = styles.FullKey.Foreground(lipgloss.Color(accentCPU))
	styles.FullDesc = styles.FullDesc.Foreground(lipgloss.Color(colorLabel))
	styles.FullSeparator = styles.FullSeparator.Foreground(lipgloss.Color(colorDim))
	styles.Ellipsis = styles.Ellipsis.Foreground(lipgloss.Color(colorDim))
}

func styleFilePicker(styles *filePickerStyles) {
	styles.Cursor = styles.Cursor.Foreground(lipgloss.Color(accentCPU))
	styles.Directory = styles.Directory.Foreground(lipgloss.Color(accentMemory)).Bold(true)
	styles.File = styles.File.Foreground(lipgloss.Color(colorValue))
	styles.Selected = styles.Selected.Foreground(lipgloss.Color(accentCPU)).Bold(true)
	styles.FileSize = styles.FileSize.Foreground(lipgloss.Color(colorDim))
	styles.Permission = styles.Permission.Foreground(lipgloss.Color(colorDim))
	styles.EmptyDirectory = styles.EmptyDirectory.Foreground(lipgloss.Color(colorDim))
}
