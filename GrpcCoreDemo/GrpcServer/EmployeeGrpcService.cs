﻿
using Grpc.Core;
using GrpcServer.Web.Data;
using GrpcServer.Web.Protos;
using Google.Protobuf.WellKnownTypes;

namespace GrpcServer.Web.Services
{
    public class EmployeeGrpcService:EmployeeService.EmployeeServiceBase
    {


        public EmployeeGrpcService()
        {

        }
        public override Task<EmployeeResponse> GetByNo(GetByNoRequest request, ServerCallContext context)
        {
            var employee = InMemoryData.Employees.SingleOrDefault(x => x.No == request.No);
            if (employee != null)
            {
                var response = new EmployeeResponse { Employee = employee };
                return Task.FromResult(response);
            }

            throw new Exception($"Employee not found with no.");
        }
        public override async  Task GetAll(GetAllRequest request, IServerStreamWriter<EmployeeResponse> responseStream, ServerCallContext context)
        {
            foreach (var employee in InMemoryData.Employees)
            {
                await responseStream.WriteAsync(new EmployeeResponse { Employee = employee });
            }
             
        }
        public override async Task<AddPhotoResponse> AddPhoto(IAsyncStreamReader<AddPhotoRequest> requestStream, ServerCallContext context)
        {
            Metadata md = context.RequestHeaders;
            foreach (var pair in md)
            {
                Console.WriteLine($"{pair.Key}:{pair.Value}");   
            }
            var data = new List<byte>();
            while (await requestStream.MoveNext())
            {
                Console.WriteLine($"Received:{requestStream.Current.Data.Length} bytes");
                data.AddRange(requestStream.Current.Data);
            }
            Console.WriteLine($"Received file with :{data.Count}bytes");
            return new AddPhotoResponse { IsOk = true };
        }
        public override async Task SaveAll(IAsyncStreamReader<EmployeeRequest> requestStream, IServerStreamWriter<EmployeeResponse> responseStream, ServerCallContext context)
        {
            while(await requestStream.MoveNext())
            {
                var employee =requestStream.Current.Employee;
                lock (this)
                {
                    InMemoryData.Employees.Add(employee);
                }
               
                await responseStream.WriteAsync(new EmployeeResponse 
                {
                    Employee = employee 
                });
            }
            Console.WriteLine($"Empolyees:{InMemoryData.Employees.Count}");
            foreach (var employee in InMemoryData.Employees)
            {
                Console.WriteLine(employee);
            }


        }
    }
}
