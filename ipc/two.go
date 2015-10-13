package main

import (
	"fmt"
	"net"
	"syscall"
)

func main() {
	unixAddr, err := net.ResolveUnixAddr("unixpacket", "\x00blue-ipc")
	if err != nil {
		fmt.Println("resolve fail")
		return
	}

	listener, err := net.ListenUnix("unixpacket", unixAddr)
	if err != nil {
		fmt.Println("ListenUnix fail")
		return
	}

	for {
		conn, err := listener.AcceptUnix()
		if err != nil {
			fmt.Println("AcceptUnix fail")
			return
		}

		var buffer = make([]byte, 128)
		var oobData = make([]byte, 128)

		msglen, ooblen, _, _, err := conn.ReadMsgUnix(buffer, oobData)
		if err != nil {
			fmt.Println("ReadMsgUnix fail")
			return
		}

		fmt.Println(string(buffer[:msglen]))

		if ooblen > 0 {
			scms, err := syscall.ParseSocketControlMessage(oobData[:ooblen])
			if err != nil {
				fmt.Println("ParseSocketControlMessage failed: %s", err)
				return
			}

			if len(scms) != 1 {
				fmt.Println("unexpected number of control messages: %d "+"(expected 1)", len(scms))
				return
			}

			fds, err := syscall.ParseUnixRights(&scms[0])
			if err != nil {
				fmt.Println("ParseUnixRights fail")
				return
			}

			if len(fds) != 1 {
				fmt.Println("unexpected number of fds")
				return
			}

			syscall.Write(fds[0], []byte("hooza!\n"))
		}
	}
}
