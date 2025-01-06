// GrpcClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <format>
#include "protos/Messages.grpc.pb.h"


constexpr char kServerCertPath[] = "D:\\Repos\\LearnGrpc\\GrpcCppDemo\\pems\\cert.pem";
constexpr char pngPath[] = "D:\\Repos\\LearnGrpc\\GrpcCppDemo\\x64\\Release\\2035.png";

std::string LoadStringFromFile(std::string path) {
    if (!std::filesystem::exists(path))
    {
        std::cout << "not exists " << path << std::endl ;
    }
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "Failed to open " << path << std::endl;
        abort();
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    return sstr.str();
}

class ClientImpl final {
public:
    ClientImpl(std::shared_ptr<grpc::Channel> channel)
        : stub_(EmployeeService::NewStub(channel)) {
    }

    void GetByNo()
    {
        grpc::ClientContext context;
        GetByNoRequest request;
        EmployeeResponse response;
        request.set_no(101);
        grpc::Status status = stub_->GetByNo(&context,request,&response);
        if (!status.ok())
        {
            std::cout << "getbyno failed\n";
            return;
        }
        std::cout << response.employee().ShortDebugString() << "\n";
        std::cout << "getbyno ok\n";

    }
    void GetAll()
    {
        grpc::ClientContext context;
        GetAllRequest request;
        auto reader = stub_->GetAll(&context,request);
        EmployeeResponse response;
        while (reader->Read(&response))
        {
            std::cout << response.employee().ShortDebugString() << "\n";
        }              
        std::cout << "GetAll ok\n";
    }
    void AddPhoto()
    {
        // 打开图片文件
        std::ifstream imgFile(pngPath, std::ios::binary);
        if (!imgFile.is_open()) {
            std::cerr << "Failed to open image file" << std::endl;
            return;
        }
        // 创建元数据
        grpc::ClientContext context;
        context.AddMetadata("no", "101");

        AddPhotoResponse response;
        // 创建流
        auto stream = stub_->AddPhoto(&context, &response);
        if (!stream) {
            std::cerr << "Failed to create stream" << std::endl;
            return;
        }

        // 读取图片文件并发送
           // 使用智能指针动态分配缓冲区
        constexpr int size = 128*1024;
        auto buffer = std::make_unique<char[]>(size);
        while (imgFile.read(buffer.get(), size)) {
            AddPhotoRequest request;
            request.set_data(buffer.get(), imgFile.gcount());
            if (!stream->Write(request)) {
                std::cerr << "Failed to write to stream" << std::endl;
                return;
            }
        }

        // 发送最后一块数据
        if (imgFile.gcount() > 0) {
            AddPhotoRequest request;
            request.set_data(buffer.get(), imgFile.gcount());
            if (!stream->Write(request)) {
                std::cerr << "Failed to write to stream" << std::endl;
                return;
            }
        }
        // 关闭流
        stream->WritesDone();
        grpc::Status status = stream->Finish();
        if (status.ok()) {
            std::cout << "AddPhoto rpc ok" << std::endl;
        }
        else {
            std::cout << "AddPhoto rpc failed." << std::endl;
        }
    }
    void SaveAll()
    {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto now_seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

        std::vector<Employee> employees = {
            // 第一个 Employee
            [&now_seconds] {
                Employee emp;
                emp.set_id(4);
                emp.set_no(104);
                emp.set_firstname("Tom");
                emp.set_lastname("Johnson");
                emp.mutable_monthsalary()->set_basic(5100);
                emp.mutable_monthsalary()->set_bonus(2000);
                emp.set_status(EmployeeStatus::ONVACATION);
                emp.mutable_lastmodified()->set_seconds(now_seconds);
                return emp;
            }(),
                // 第二个 Employee
                [&now_seconds] {
                    Employee emp;
                    emp.set_id(5);
                    emp.set_no(105);
                    emp.set_firstname("Mick");
                    emp.set_lastname("Thompson");
                    emp.mutable_monthsalary()->set_basic(2000);
                    emp.mutable_monthsalary()->set_bonus(300);
                    emp.set_status(EmployeeStatus::RESIGNED);
                    emp.mutable_lastmodified()->set_seconds(now_seconds);
                    return emp;
                }(),
                // 第三个 Employee
               [&now_seconds] {
                   Employee emp;
                   emp.set_id(6);
                   emp.set_no(106);
                   emp.set_firstname("Alex");
                   emp.set_lastname("Robinson");
                   emp.mutable_monthsalary()->set_basic(20000);
                   emp.mutable_monthsalary()->set_bonus(300);
                   emp.set_status(EmployeeStatus::RETIRED);
                   emp.mutable_lastmodified()->set_seconds(now_seconds);
               return emp;
           }()
        };
        grpc::ClientContext context;
        std::shared_ptr<::grpc::ClientReaderWriter< ::EmployeeRequest, ::EmployeeResponse>> 
        stream(stub_->SaveAll(&context));

        std::thread writer([stream,&employees](){
            for (auto& emp : employees)
            {
                EmployeeRequest request;
                //request.set_allocated_employee(&emp);
                request.mutable_employee()->CopyFrom(emp);
                stream->Write(request);
            }
            stream->WritesDone();
        });
        ::EmployeeResponse response;
        while (stream->Read(&response))
        {
           std::cout << response.employee().ShortDebugString() << "\n";
        }
        writer.join();
        grpc::Status status = stream->Finish();
        if (!status.ok())
        {
            std::cout << "SaveAll rpc failed." << std::endl;
        }
    }
private:
    std::unique_ptr<EmployeeService::Stub> stub_;
};

const int port = 5001;

int main()
{
    auto creds = grpc::SslCredentials(grpc::SslCredentialsOptions{
        .pem_root_certs = LoadStringFromFile(kServerCertPath)
    });
    if (!creds)
    {
        std::cout << "Cert error!";
    }
    auto channel = grpc::CreateChannel(std::format( "localhost:{}",port),creds);
    if (!channel) {
        std::cerr << "Failed to create channel" << std::endl;
        return -1;
    }
    // 创建客户端
    auto client = ClientImpl(channel);
    //client.GetByNo();
    //client.GetAll();
    //client.AddPhoto();
    client.SaveAll();
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
