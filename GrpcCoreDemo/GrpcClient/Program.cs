using Google.Protobuf;
using Google.Protobuf.WellKnownTypes;
using Grpc.Core;
using GrpcServer.Web.Protos;

internal class Program
{
    private static async Task Main(string[] args)
    {
        const int port = 5001;
        var pem = File.ReadAllText(@"cert.pem");
        var sslCredentials = new SslCredentials(pem);
        var channel = new Channel("localhost", port, sslCredentials);
        var client = new EmployeeService.EmployeeServiceClient(channel);
        // 取消注释以调用其他函数
        // await GetByNo(client);
         //await GetAll(client);
         //await AddPhoto(client);
        await SaveAll(client);
        
        Console.ReadKey();

        await channel.ShutdownAsync();

    }
    private static async Task GetByNo(EmployeeService.EmployeeServiceClient client)
    {
        var response = await client.GetByNoAsync(new GetByNoRequest { No = 101 });
        Console.WriteLine(response.Employee);
    }
    private static async Task SaveAll(EmployeeService.EmployeeServiceClient client)
    {
        var employees = new List<Employee>
        {
            new Employee
            {
                Id = 4,
                No = 104,
                FirstName = "Tom",
                LastName = "Johnson",
                MonthSalary = new MonthSalary { Basic = 5100, Bonus = 2000 },
                Status = EmployeeStatus.Onvacation,
                LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
            },
            new Employee
            {
                Id = 5,
                No = 105,
                FirstName = "Mick",
                LastName = "Thompson",
                MonthSalary = new MonthSalary { Basic = 2000, Bonus = 300 },
                Status = EmployeeStatus.Resigned,
                LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
            },
            new Employee
            {
                Id = 6,
                No = 106,
                FirstName = "Alex",
                LastName = "Robinson",
                MonthSalary = new MonthSalary { Basic = 20000, Bonus = 300 },
                Status = EmployeeStatus.Retired,
                LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
            }
        };

        using (var call = client.SaveAll())
        {
            var responseTask = Task.Run(async () =>
            {
                while (await call.ResponseStream.MoveNext())
                {
                    var response = call.ResponseStream.Current;
                    Console.WriteLine(response.Employee);
                }
            });

            foreach (var emp in employees)
            {
                await call.RequestStream.WriteAsync(new EmployeeRequest { Employee = emp });
            }

            await call.RequestStream.CompleteAsync();
            await responseTask;
        }
    }

    private static async Task AddPhoto(EmployeeService.EmployeeServiceClient client)
    {
  
        var md = new Metadata
            {
                {"no","101" }
            };
        FileStream fs = File.OpenRead("2035.png");
        var call = client.AddPhoto(md);
        var stream = call.RequestStream;
        while (true)
        {
            byte[] buffer = new byte[128*1024];
            int numRead = await fs.ReadAsync(buffer, 0, buffer.Length);
            if (numRead == 0)
            {
                break;
            }
            if (numRead < buffer.Length)
            {
                Array.Resize(ref buffer, numRead);
            }
            await stream.WriteAsync(new AddPhotoRequest
            {
                Data = ByteString.CopyFrom(buffer)
            });
        }
        await stream.CompleteAsync();
        var response3 = await call.ResponseAsync;
        Console.WriteLine(response3.IsOk);
    }



    private static async Task GetAll(EmployeeService.EmployeeServiceClient client)
    {
        using (var call = client.GetAll(new GetAllRequest()))
        {
            while (await call.ResponseStream.MoveNext())
            {
                var response = call.ResponseStream.Current;
                Console.WriteLine(response.Employee);
            }
        }
    }
}