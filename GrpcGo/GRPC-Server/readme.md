### go生成pb和代码

C:\Users\xqit\source\repos\TestGrpc\grpc\bin\protoc --proto_path=./protos ./protos/*.proto --go_out=. 

C:\Users\xqit\source\repos\TestGrpc\grpc\bin\protoc --proto_path=./protos ./protos/*.proto --go-grpc_out=.  


### 参考
https://www.cnblogs.com/beatle-go/p/17988004

### ssh
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj '/CN=localhost'
### ssh with sans
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes -subj '/CN=localhost' -addext 'subjectAltName=DNS:localhost'

go build

./grpc-server.exe

go run main.go