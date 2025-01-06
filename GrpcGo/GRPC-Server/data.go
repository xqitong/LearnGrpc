package main

import (
	"grpc-server/pb"
	"time"

	"google.golang.org/protobuf/types/known/timestamppb"
)

var employees = []pb.Employee{
	{
		Id:           1,
		No:           101,
		FirstName:    "John",
		LastName:     "Doe",
		MonthSalary:  &pb.MonthSalary{Basic: 5000, Bonus: 1000},          // 假设未定义的 MonthSalary
		Status:       pb.EmployeeStatus_NORMAL,                           // 假设有相应的状态枚举
		LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
	},
	{
		Id:           2,
		No:           102,
		FirstName:    "Jane",
		LastName:     "Smith",
		MonthSalary:  &pb.MonthSalary{Basic: 1000, Bonus: 3000},          // 假设未定义的 MonthSalary
		Status:       pb.EmployeeStatus_RESIGNED,                         // 假设有相应的状态枚举
		LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
	},
	{
		Id:           3,
		No:           103,
		FirstName:    "Alice",
		LastName:     "Johnson",
		MonthSalary:  &pb.MonthSalary{Basic: 10000, Bonus: 300},          // 假设未定义的 MonthSalary
		Status:       pb.EmployeeStatus_RETIRED,                          // 假设有相应的状态枚举
		LastModified: &timestamppb.Timestamp{Seconds: time.Now().Unix()}, // 当前时间
	},
}
