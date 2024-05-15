# tinycdb-go
利用purego库绑定tinycdb 只读数据库的 库 提供快速情报和共享功能

## 注意
提前编译tinycdb的so文件 , 安装步骤如下:
```bash
    cd tinycdb-c
    make
```
## interface

```golang
package main

import (
	"fmt"
	"github.com/vela-security/tinycdb-go"
	"github.com/vela-security/tinycdb-go/ipconv"
	"time"
)

func NewTinyCDB() *tinycdb.CDb {
	cdb, err := tinycdb.New("/vdb/cdb/tinycdb-0.81/libcdb.so.1")
	if err != nil {
		panic(err)
	}

	return cdb
}

func Maker(cdb *tinycdb.CDb) {
	var err error
	err = cdb.Maker("shm-2.db")
	if err != nil {
		fmt.Println(err)
		return
	}

	ip, _, _ := ipconv.ParseIP("10.0.0.1")
	start, _ := ipconv.IPv4ToInt(ip)
	for i := 0; i < 100050; i++ {
		key := ipconv.IntToIPv4(start + uint32(i))
		val := time.Now().Format(time.RFC3339)
		err = cdb.Add(key.String(), val)
		if err != nil {
			fmt.Println(err)
			continue
		}
	}
	err = cdb.Finish()
	if err != nil {
		fmt.Println(err)
		return
	}

	err = cdb.Free()
	if err != nil {
		fmt.Println(err)
	}

}

func checkErr(err error) {
	if err != nil {
		panic(err)
	}
}

func main() {
	cdb := NewTinyCDB()
	//Maker(cdb)

	err := cdb.Finder("shm-2.db")
	checkErr(err)

	elem, err := cdb.Get("10.0.0.2")
	checkErr(err)
	println(elem)

	//_ = cdb.Foreach(func(key string, val string) (stop bool) {
	//	fmt.Println(key, val)
	//	return false
	//})
}

```