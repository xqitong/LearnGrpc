package main

import (
	"errors"
	"fmt"
	"grpc-server/pb"
	"io"
	"log"
	"net"

	"golang.org/x/net/context"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
)

const port = ":5001"

func main() {
	listen, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalln(err.Error())
	}
	creds, err := credentials.NewServerTLSFromFile("cert.pem", "key.pem")
	if err != nil {
		log.Fatalln(err.Error())
	}
	options := []grpc.ServerOption{grpc.Creds(creds)}
	server := grpc.NewServer(options...)
	pb.RegisterEmployeeServiceServer(server, new(employeeService))
	log.Println("GRPC-Server is running on port " + port)
	server.Serve(listen)
}

type employeeService struct {
	pb.UnimplementedEmployeeServiceServer // 嵌入未导出的结构体
}

func (s *employeeService) GetByNo(ctx context.Context, req *pb.GetByNoRequest) (*pb.EmployeeResponse, error) {
	for _, emp := range employees {
		if emp.No == req.No {
			fmt.Println("GetByNo called and Employee found:", emp)
			return &pb.EmployeeResponse{Employee: &emp}, nil
		}
	}
	return nil, errors.New("Employee not found")
}
func (s *employeeService) GetAll(req *pb.GetAllRequest, stream grpc.ServerStreamingServer[pb.EmployeeResponse]) error {
	for _, emp := range employees {
		stream.Send(&pb.EmployeeResponse{Employee: &emp})
	}
	return nil
}
func (s *employeeService) AddPhoto(stream grpc.ClientStreamingServer[pb.AddPhotoRequest, pb.AddPhotoResponse]) error {
	md, ok := metadata.FromIncomingContext(stream.Context())
	if ok {
		fmt.Printf("Employee: %s\n", md["no"][0])
	}
	img := []byte{}
	for {
		data, err := stream.Recv()
		if err == io.EOF {
			fmt.Println("File Size:", len(img))
			return stream.SendAndClose(&pb.AddPhotoResponse{IsOk: true})
		}
		if err != nil {
			return err
		}
		fmt.Println("Received", len(data.Data), "bytes")
		img = append(img, data.Data...)
	}

}
func (s *employeeService) Save(context.Context, *pb.EmployeeRequest) (*pb.EmployeeResponse, error) {
	return nil, nil
}
func (s *employeeService) SaveAll(stream grpc.BidiStreamingServer[pb.EmployeeRequest, pb.EmployeeResponse]) error {

	for {
		empReq, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}
		employees = append(employees, *empReq.Employee)
		stream.Send((&pb.EmployeeResponse{Employee: empReq.Employee}))

	}
	for _, emp := range employees {
		fmt.Println(emp)
	}
	return nil
}
func (s *employeeService) CreateToken(context.Context, *pb.TokenRequest) (*pb.TokenResponse, error) {
	return nil, nil
}
func (s *employeeService) mustEmbedUnimplementedEmployeeServiceServer() {}
