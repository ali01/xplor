package main

import (
	"fmt"
	"sort"
)

type SortableInt64Array []int64

func (a SortableInt64Array) Len() int           { return len(a) }
func (a SortableInt64Array) Less(i, j int) bool { return a[i] < a[j] }
func (a SortableInt64Array) Swap(i, j int)      { a[i], a[j] = a[j], a[i] }

func main() {
	l := SortableInt64Array{3, 2, 1, 4}
	sort.Sort(sort.Reverse(l))
	fmt.Print(l)
}
