go build -gcflags "all=-N -l" -o cdb-go cmd/main.go &&
dlv --listen=0.0.0.0:2345 --headless=true --api-version=2  exec ./cdb-go