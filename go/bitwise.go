package main

import (
	"fmt"
)

const messageSlotAlign = 8

func alignMessageSize(size int64) int64 {
	return (messageSlotAlign - 1) &^ (messageSlotAlign - 1)
}

func main() {
	fmt.Println(4 &^ 8)
}
