package main

import (
	"fmt"
	"os"
)

var (
	strs []string
)

func main() {

	hey := "~"
	info, err := os.Stat(hey)
	if err != nil || info.Mode()&os.ModeDir > 0 {
		fmt.Println("directory!")
	}

}
