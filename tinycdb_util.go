package tinycdb

import "unsafe"

func clone(ptr uintptr, size uint) string {
	if size == 0 {
		return ""
	}

	buf := make([]byte, size)
	for i := 0; uint(i) < size; i++ {
		b := (*byte)(unsafe.Pointer(ptr + uintptr(i)))
		buf[i] = *b
	}
	return string(buf)
}
