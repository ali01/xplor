package main

import "fmt"

func closureTest() (func() int, func() int) {
	i := 0

	f1 := func() int {
		i += 1
		return i
	}

	f2 := func() int {
		i += 2
		return i
	}

	return f1, f2
}

func main() {
	f1, f2 := closureTest()

	fmt.Println(f1())
	fmt.Println(f2())
}
