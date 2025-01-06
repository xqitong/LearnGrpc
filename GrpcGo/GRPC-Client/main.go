package main

import (
	"context"
	"fmt"
	"grpc-client/pb"
	"io"
	"log"
	"os"
	"time"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"google.golang.org/grpc/metadata"
	"google.golang.org/protobuf/types/known/timestamppb"
)

const port = "5001"

func main() {
	creds, err := credentials.NewClientTLSFromFile("cert.pem", "")
	if err != nil {
		log.Fatalln(err.Error())
	}
	options := []grpc.DialOption{grpc.WithTransportCredentials(creds)}
	conn, err := grpc.NewClient("localhost:"+port, options...)
	if err != nil {
		log.Fatalln(err.Error())
	}
	defer conn.Close()
	client := pb.NewEmployeeServiceClient(conn)
	//getByNo(client)
	//getAll(client)
	//addPhoto(client)
	saveAll(client)
}
func saveAll(client pb.EmployeeServiceClient) {
	var employees = []pb.Employee{
		{
			Id:           4,
			No:           104,
			FirstName:    "Tom",
			LastName:     "Johnson",
			MonthSalary:  &pb.MonthSalary{Basic: 5100, Bonus: 2000},          // 假设未定义的 MonthSalary
			Status:       pb.EmployeeStatus_ONVACATION,                       // 假设有相应的状态枚举
			LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
		},
		{
			Id:           5,
			No:           105,
			FirstName:    "Mick",
			LastName:     "Thompson",
			MonthSalary:  &pb.MonthSalary{Basic: 2000, Bonus: 300},           // 假设未定义的 MonthSalary
			Status:       pb.EmployeeStatus_RESIGNED,                         // 假设有相应的状态枚举
			LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
		},
		{
			Id:           6,
			No:           106,
			FirstName:    "Alex",
			LastName:     "Robinson",
			MonthSalary:  &pb.MonthSalary{Basic: 20000, Bonus: 300},          // 假设未定义的 MonthSalary
			Status:       pb.EmployeeStatus_RETIRED,                          // 假设有相应的状态枚举
			LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
		},
	}
	stream, err := client.SaveAll(context.Background())
	if err != nil {
		log.Fatalln(err.Error())
	}
	finishChannel := make(chan struct{})
	go func() {
		for {
			res, err := stream.Recv()
			if err == io.EOF {
				finishChannel <- struct{}{}
				break
			}
			if err != nil {
				log.Fatal(err.Error())
			}
			fmt.Println(res.Employee)
		}
	}()

	for _, emp := range employees {
		err := stream.Send(&pb.EmployeeRequest{Employee: &emp})
		if err != nil {
			log.Fatalln(err.Error())
		}
	}
	stream.CloseSend()
	<-finishChannel
}

func addPhoto(client pb.EmployeeServiceClient) {
	imgFile, err := os.Open("2035.png")
	if err != nil {
		log.Fatal(err.Error())
	}
	defer imgFile.Close()
	md := metadata.New(map[string]string{"no": "101"})
	context := context.Background()
	context = metadata.NewOutgoingContext(context, md)
	stream, err := client.AddPhoto(context)
	if err != nil {
		log.Fatalln(err.Error())
	}
	for {
		chunk := make([]byte, 128*1024)
		chunkSize, err := imgFile.Read(chunk)
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatalln(err.Error())
		}
		if chunkSize < len(chunk) {
			chunk = chunk[:chunkSize]
		}
		stream.Send(&pb.AddPhotoRequest{Id: 101, Data: chunk})

	}
	res, err := stream.CloseAndRecv()
	if err != nil {
		log.Fatalln(err.Error())
	}
	fmt.Println(res.IsOk)

}

func getByNo(client pb.EmployeeServiceClient) {
	res, err := client.GetByNo(context.Background(), &pb.GetByNoRequest{No: 101})
	if err != nil {
		log.Fatalln(err.Error())
	}
	fmt.Println(res.Employee)
}

func getAll(client pb.EmployeeServiceClient) {
	stream, err := client.GetAll(context.Background(), &pb.GetAllRequest{})
	if err != nil {
		log.Fatalln(err.Error())
	}
	for {
		res, err := stream.Recv()
		if err == io.EOF {
			break
		}
		if err != nil {
			log.Fatalln(err.Error())
		}
		fmt.Println(res.Employee)
	}
}
