package main

import (
	"fmt"
)

type Range int

type Test struct {
	Range Range
}

func main() {
	t := &Test{}
	t.Range = 5
	fmt.Println(t.Range)
}
