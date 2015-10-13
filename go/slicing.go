package main

import (
	"fmt"
)

func main() {
	b := []int{2, 2, 3, 4, 5}

	for _, num := range b[:len(b)-1] {
		fmt.Println(num)
	}

	fmt.Println(b[len(b)-1])
}
