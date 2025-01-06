using Grpc.Core;
using GrpcServer.Web.Protos;
using GrpcServer.Web.Services;

internal class Program
{
    private static async Task Main(string[] args)
    {
        const int port = 5001;
        var cert = File.ReadAllText(@"cert.pem");
        var privateKey = File.ReadAllText(@"key.pem");
        var sslCredentials = new SslServerCredentials(new List<KeyCertificatePair>{
            new KeyCertificatePair(cert,privateKey)
        });
        var server = new Server
        {
          Ports = {new ServerPort("localhost",port, sslCredentials)},
          Services = { EmployeeService.BindService(new EmployeeGrpcService())}
        };
        server.Start();
        Console.WriteLine($"Starting server on port:{port}");
        Console.WriteLine("Press any key to continue");
        Console.ReadKey();
        await server.ShutdownAsync();
    }
}