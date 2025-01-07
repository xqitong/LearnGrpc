// See https://aka.ms/new-console-template for more information
using Google.Protobuf;
using Google.Protobuf.WellKnownTypes;
using Grpc.Core;
using Grpc.Net.Client;
using GrpcServer.Web.Protos;
using Microsoft.Extensions.Logging;
using Serilog;

internal class Program
{
    private static string _token;
    private static DateTime _expiration = DateTime.MinValue;
    private static bool NeedToken() => string.IsNullOrEmpty(_token) || _expiration > DateTime.UtcNow;
    private static async Task Main(string[] args)
    {
 
        Console.WriteLine("https://localhost:7149");
        Log.Logger = new LoggerConfiguration().MinimumLevel.Debug().WriteTo.Console().CreateLogger();
        Log.Information("Client starting...");
        using var channel = GrpcChannel.ForAddress("https://localhost:7149",
        new GrpcChannelOptions
        {
            LoggerFactory = new GrpcClient.SerilogLoggerFactory()
        });
        var client = new EmployeeService.EmployeeServiceClient(channel);
        var option = int.Parse(args[0]);
        switch (option)
        {
            case 1:
                await GetByNoAsync(client);
                break;
            case 2:
                await GetAllAsync(client);
                break;
            case 3:
                await AddPhotoAsync(client);

                break;
            case 5:
                   await SaveAllAsync(client);
                break;
            default:
                break;
        }



        Console.WriteLine("Press any key to continue");
        Console.ReadKey();
        Log.CloseAndFlush();


    }
    static async Task SaveAllAsync(EmployeeService.EmployeeServiceClient client)
    {
        var employees = new List<Employee>
        {
            new Employee { Id = 111,
                            No = 111,
                            FirstName = "Alice",
                            LastName = "Smith",
                            MonthSalary = new MonthSalary{ Basic = 10000f, Bonus = 10f },
                            Status = EmployeeStatus.Normal,
                            LastModified = Timestamp.FromDateTime(DateTime.UtcNow)/*, Salary = 60000*/ },
            new Employee { Id = 222,
                            No = 222,
                            FirstName = "Bob",
                            LastName = "Johnson",
                            MonthSalary = new MonthSalary{ Basic = 1000f, Bonus = 100f },
                            Status = EmployeeStatus.Resigned,
                            LastModified = Timestamp.FromDateTime(DateTime.UtcNow)
                            /*, Salary = 75000*/ }
        };
        var call = client.SaveAll();
        var requestStream = call.RequestStream;
        var responseStream = call.ResponseStream;
        var responseTask = Task.Run(async () =>
        {
            while (await responseStream.MoveNext())
            {
                Console.WriteLine($"Saved:{responseStream.Current.Employee}");
            }
        });
        foreach (var employee in employees)
        {
            await requestStream.WriteAsync(new EmployeeRequest
            {
                Employee = employee
            });
        }
        await requestStream.CompleteAsync();
        await responseTask;
    }
    static async Task AddPhotoAsync(EmployeeService.EmployeeServiceClient client)
    {
        var md = new Metadata
            {
                {"username","dave" },
                {"role","administrator" }
            };
        FileStream fs = File.OpenRead("logo.png");
        var call = client.AddPhoto();
        var stream = call.RequestStream;
        while (true)
        {
            byte[] buffer = new byte[1024];
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
    static async Task GetAllAsync(EmployeeService.EmployeeServiceClient client)
    {
        var responseAll = client.GetAll(new GetAllRequest());
        await foreach (var employee in responseAll.ResponseStream.ReadAllAsync())
        {
            Console.WriteLine($"Employee: {employee}");
        }

    }
    static async Task GetByNoAsync(EmployeeService.EmployeeServiceClient client)
    {
        
        if (!NeedToken() || await GetTokenAsync(client))
        {
            var headers = new Metadata
            {
                {"Authorization",$"Bearer {_token}"}
            };
            var reponse = await client.GetByNoAsync(new GetByNoRequest { No = 101 }, headers);
            Console.WriteLine($"Response messages:{reponse}");
        }

    }

    private static async Task<bool> GetTokenAsync(EmployeeService.EmployeeServiceClient client)
    {
        var request = new TokenRequest {Username = "admin",Password = "1234" };
        var response = await client.CreateTokenAsync(request);
        if (response.Success)
        {
            _token = response.Token;
            _expiration = response.Expiration.ToDateTime();
            return true;
        }
        return false;
    }
}