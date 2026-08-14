package main

import (
	"charm.land/lipgloss/v2"
)

type Direction int

const (
	Leaf Direction = iota
	Row
	Column
)

type Size struct {
	Width  int
	Height int
}

type Tile struct {
	key          string
	title        string
	body         func(width, height int) string
	direction    Direction
	children     []Tile
	widthWeight  int
	heightWeight int
	fixedWidth   int
	fixedHeight  int
}

// NewTile builds a bordered tile. The body is a function of the space the tile
// ends up with, so content that has to fit a width can be built once the
// layout has decided what that width is.
func NewTile(key string, title string, body func(width, height int) string, widthWeight int, heightWeight int) Tile {
	return Tile{
		key:          key,
		title:        title,
		body:         body,
		direction:    Leaf,
		widthWeight:  widthWeight,
		heightWeight: heightWeight,
	}
}

func StaticBody(content string) func(width, height int) string {
	return func(int, int) string { return content }
}

func joinTiles(widthWeight int, heightWeight int, tiles ...Tile) Tile {
	return Tile{
		direction:    Row,
		children:     tiles,
		widthWeight:  widthWeight,
		heightWeight: heightWeight,
	}
}

func stackSections(widthWeight int, heightWeight int, tiles ...Tile) Tile {
	return Tile{
		direction:    Column,
		children:     tiles,
		widthWeight:  widthWeight,
		heightWeight: heightWeight,
	}
}

func (t Tile) WithFixedWidth(width int) Tile {
	t.fixedWidth = width
	return t
}

func (t Tile) WithFixedHeight(height int) Tile {
	t.fixedHeight = height
	return t
}

// outerSize clamps to the smallest box that can still draw a border.
func outerSize(width, height int) (int, int) {
	return max(width, 4), max(height, 3)
}

// innerSize is the space left for a tile's body once its border, padding,
// title and the blank line under it are accounted for.
func innerSize(width, height int) (int, int) {
	outerWidth, outerHeight := outerSize(width, height)

	return max(outerWidth-tileHorizontalChrome, 1),
		max(outerHeight-tileVerticalChrome, 1)
}

func RenderTile(tile Tile, width int, height int) string {
	switch tile.direction {
	case Row:
		widths := distribute(width, tile.children, widthOf)
		parts := make([]string, len(tile.children))
		for i, child := range tile.children {
			parts[i] = RenderTile(child, widths[i], height)
		}

		return lipgloss.JoinHorizontal(lipgloss.Top, parts...)

	case Column:
		heights := distribute(height, tile.children, heightOfTile)
		parts := make([]string, len(tile.children))
		for i, child := range tile.children {
			parts[i] = RenderTile(child, width, heights[i])
		}

		return lipgloss.JoinVertical(lipgloss.Left, parts...)
	}

	outerWidth, outerHeight := outerSize(width, height)
	innerWidth, innerHeight := innerSize(width, height)

	content := titleStyle.Render(tile.title)
	if tile.body != nil {
		if body := tile.body(innerWidth, innerHeight); body != "" {
			content += "\n\n" + body
		}
	}

	return tileStyle.Width(outerWidth).Height(outerHeight).Render(content)
}

// MeasureTiles walks the same tree RenderTile does and reports the body size
// every keyed tile will be given, without building any content. Components
// that need to be sized before they can render use this.
func MeasureTiles(tile Tile, width int, height int) map[string]Size {
	sizes := map[string]Size{}
	measureInto(sizes, tile, width, height)

	return sizes
}

func measureInto(sizes map[string]Size, tile Tile, width int, height int) {
	switch tile.direction {
	case Row:
		widths := distribute(width, tile.children, widthOf)
		for i, child := range tile.children {
			measureInto(sizes, child, widths[i], height)
		}
		return

	case Column:
		heights := distribute(height, tile.children, heightOfTile)
		for i, child := range tile.children {
			measureInto(sizes, child, width, heights[i])
		}
		return
	}

	if tile.key != "" {
		innerWidth, innerHeight := innerSize(width, height)
		sizes[tile.key] = Size{Width: innerWidth, Height: innerHeight}
	}
}

func widthOf(t Tile) (int, int) {
	return t.widthWeight, t.fixedWidth
}

func heightOfTile(t Tile) (int, int) {
	return t.heightWeight, t.fixedHeight
}

func distribute(total int, tiles []Tile, sizeOf func(Tile) (int, int)) []int {
	out := make([]int, len(tiles))
	if total <= 0 {
		return out
	}

	remaining := total
	weightSum := 0

	for i, tile := range tiles {
		weight, fixed := sizeOf(tile)
		if fixed > 0 {
			out[i] = min(fixed, max(remaining, 0))
			remaining -= out[i]
			continue
		}
		weightSum += weight
	}

	if remaining <= 0 || weightSum <= 0 {
		return out
	}

	unit := remaining / weightSum

	for i, tile := range tiles {
		weight, fixed := sizeOf(tile)
		if fixed > 0 {
			continue
		}
		out[i] = weight * unit
	}

	return out
}

func stackStrings(parts ...string) string {
	return lipgloss.JoinVertical(lipgloss.Left, parts...)
}

func heightOf(s string) int {
	return lipgloss.Height(s)
}
