package main

import (
	"fmt"
)

func Test(func(int)) {
	fmt.Println("hello world")
}

func inc(int) {

}

func main() {
	Test(inc)
}
